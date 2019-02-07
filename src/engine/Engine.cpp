#include "engine/Engine.hpp"
#include "settings.hpp"
#include "system.hpp"
#include "random.hpp"

#include <algorithm>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <xmmintrin.h>
#include <pmmintrin.h>


namespace rack {
namespace engine {


/** Threads which obtain a VIPLock will cause wait() to block for other less important threads.
This does not provide the VIPs with an exclusive lock. That should be left up to another mutex shared between the less important thread.
*/
struct VIPMutex {
	int count = 0;
	std::condition_variable cv;
	std::mutex countMutex;

	/** Blocks until there are no remaining VIPLocks */
	void wait() {
		std::unique_lock<std::mutex> lock(countMutex);
		while (count > 0)
			cv.wait(lock);
	}
};


struct VIPLock {
	VIPMutex &m;
	VIPLock(VIPMutex &m) : m(m) {
		std::unique_lock<std::mutex> lock(m.countMutex);
		m.count++;
	}
	~VIPLock() {
		std::unique_lock<std::mutex> lock(m.countMutex);
		m.count--;
		lock.unlock();
		m.cv.notify_all();
	}
};


struct Barrier {
	std::mutex mutex;
	std::condition_variable cv;
	int count = 0;
	int total = 0;

	void wait() {
		// Waiting on one thread is trivial.
		if (total <= 1)
			return;
		std::unique_lock<std::mutex> lock(mutex);
		count++;
		if (count < total) {
			cv.wait(lock);
		}
		else {
			count = 0;
			cv.notify_all();
		}
	}
};


struct SpinBarrier {
	std::atomic<int> count;
	int total = 0;

	SpinBarrier() {
		count = 0;
	}

	void wait() {
		count++;
		if (count < total) {
			while (count > 0) {}
		}
		else {
			count = 0;
		}
	}
};


struct EngineWorker {
	Engine *engine;
	int id;
	std::thread thread;
	bool running = true;

	void start() {
		thread = std::thread([&] {
			run();
		});
	}

	void stop() {
		running = false;
	}

	void join() {
		thread.join();
	}

	void run() {
		system::setThreadName("Engine worker");
		system::setThreadRealTime();
		while (running) {
			step();
		}
	}

	void step();
};


struct Engine::Internal {
	std::vector<Module*> modules;
	std::vector<Cable*> cables;
	bool paused = false;

	bool running = false;
	float sampleRate;
	float sampleTime;
	float sampleRateRequested;

	int nextModuleId = 0;
	int nextCableId = 0;

	// Parameter smoothing
	Module *smoothModule = NULL;
	int smoothParamId;
	float smoothValue;

	std::mutex mutex;
	std::thread thread;
	VIPMutex vipMutex;

	int threadCount = 1;
	std::vector<EngineWorker> workers;
	SpinBarrier engineBarrier;
	SpinBarrier workerBarrier;
};


Engine::Engine() {
	internal = new Internal;

	float sampleRate = settings.sampleRate;
	internal->sampleRate = sampleRate;
	internal->sampleTime = 1 / sampleRate;
	internal->sampleRateRequested = sampleRate;

	internal->threadCount = settings.threadCount;
	internal->engineBarrier.total = 1;
	internal->workerBarrier.total = 1;
}

Engine::~Engine() {
	settings.sampleRate = internal->sampleRate;
	settings.threadCount = internal->threadCount;

	// Make sure there are no cables or modules in the rack on destruction. This suggests that a module failed to remove itself before the RackWidget was destroyed.
	assert(internal->cables.empty());
	assert(internal->modules.empty());

	delete internal;
}

static void Engine_setWorkerCount(Engine *engine, int workerCount) {
	assert(0 <= workerCount && workerCount <= 32);
	Engine::Internal *internal = engine->internal;

	// Stop all workers
	for (EngineWorker &worker : internal->workers) {
		worker.stop();
	}
	internal->engineBarrier.wait();

	// Destroy all workers
	for (EngineWorker &worker : internal->workers) {
		worker.join();
	}
	internal->workers.resize(0);

	// Set barrier counts
	internal->engineBarrier.total = workerCount + 1;
	internal->workerBarrier.total = workerCount + 1;

	if (workerCount >= 1) {
		// Create workers
		internal->workers.resize(workerCount);
		for (int i = 0; i < workerCount; i++) {
			EngineWorker &worker = internal->workers[i];
			worker.id = i + 1;
			worker.engine = engine;
			worker.start();
		}
	}
}

static void Engine_stepModules(Engine *engine, int threadId) {
	Engine::Internal *internal = engine->internal;

	int threadCount = internal->threadCount;
	int modulesLen = internal->modules.size();

	for (int i = threadId; i < modulesLen; i += threadCount) {
		Module *module = internal->modules[i];
		if (!module->bypass) {
			// Step module
			if (settings.cpuMeter) {
				auto startTime = std::chrono::high_resolution_clock::now();

				module->step();

				auto stopTime = std::chrono::high_resolution_clock::now();
				float cpuTime = std::chrono::duration<float>(stopTime - startTime).count();
				// Smooth CPU time
				const float cpuTau = 2.f /* seconds */;
				module->cpuTime += (cpuTime - module->cpuTime) * internal->sampleTime / cpuTau;
			}
			else {
				module->step();
			}
		}

		// Iterate ports and step plug lights
		for (Input &input : module->inputs) {
			input.step();
		}
		for (Output &output : module->outputs) {
			output.step();
		}
	}
}

static void Engine_step(Engine *engine) {
	Engine::Internal *internal = engine->internal;

	// Sample rate
	if (internal->sampleRateRequested != internal->sampleRate) {
		internal->sampleRate = internal->sampleRateRequested;
		internal->sampleTime = 1 / internal->sampleRate;
		for (Module *module : internal->modules) {
			module->onSampleRateChange();
		}
	}

	// Param smoothing
	{
		Module *smoothModule = internal->smoothModule;
		int smoothParamId = internal->smoothParamId;
		float smoothValue = internal->smoothValue;
		if (smoothModule) {
			Param *param = &smoothModule->params[smoothParamId];
			float value = param->value;
			// decay rate is 1 graphics frame
			const float smoothLambda = 60.f;
			float newValue = value + (smoothValue - value) * smoothLambda * internal->sampleTime;
			if (value == newValue || !(param->minValue <= newValue && newValue <= param->maxValue)) {
				// Snap to actual smooth value if the value doesn't change enough (due to the granularity of floats), or if newValue is out of bounds
				param->setValue(smoothValue);
				internal->smoothModule = NULL;
			}
			else {
				param->value = newValue;
			}
		}
	}

	// Lazily create/destroy workers
	int workerCount = internal->threadCount - 1;
	if ((int) internal->workers.size() != workerCount) {
		Engine_setWorkerCount(engine, workerCount);
	}
	else {
		internal->engineBarrier.wait();
	}

	// Step modules along with workers
	Engine_stepModules(engine, 0);
	internal->workerBarrier.wait();

	// Step cables
	for (Cable *cable : engine->internal->cables) {
		cable->step();
	}
}

static void Engine_run(Engine *engine) {
	// Set up thread
	system::setThreadName("Engine");
	system::setThreadRealTime();

	// Set CPU to flush-to-zero (FTZ) and denormals-are-zero (DAZ) mode
	// https://software.intel.com/en-us/node/682949
	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
	_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

	// Every time the engine waits and locks a mutex, it steps this many frames
	const int mutexSteps = 64;
	// Time in seconds that the engine is rushing ahead of the estimated clock time
	double ahead = 0.0;
	auto lastTime = std::chrono::high_resolution_clock::now();

	while (engine->internal->running) {
		engine->internal->vipMutex.wait();

		if (!engine->internal->paused) {
			std::lock_guard<std::mutex> lock(engine->internal->mutex);
			// auto startTime = std::chrono::high_resolution_clock::now();

			for (int i = 0; i < mutexSteps; i++) {
				Engine_step(engine);
			}

			// auto stopTime = std::chrono::high_resolution_clock::now();
			// float cpuTime = std::chrono::duration<float>(stopTime - startTime).count();
			// DEBUG("%g", cpuTime / mutexSteps * 44100);
		}

		double stepTime = mutexSteps * engine->internal->sampleTime;
		ahead += stepTime;
		auto currTime = std::chrono::high_resolution_clock::now();
		const double aheadFactor = 2.0;
		ahead -= aheadFactor * std::chrono::duration<double>(currTime - lastTime).count();
		lastTime = currTime;
		ahead = std::fmax(ahead, 0.0);

		// Avoid pegging the CPU at 100% when there are no "blocking" modules like AudioInterface, but still step audio at a reasonable rate
		// The number of steps to wait before possibly sleeping
		const double aheadMax = 1.0; // seconds
		if (ahead > aheadMax) {
			std::this_thread::sleep_for(std::chrono::duration<double>(stepTime));
		}
	}

	Engine_setWorkerCount(engine, 0);
}

void Engine::start() {
	internal->running = true;
	internal->thread = std::thread(Engine_run, this);
}

void Engine::stop() {
	internal->running = false;
	internal->thread.join();
}

void Engine::setThreadCount(int threadCount) {
	assert(threadCount >= 1);
	VIPLock vipLock(internal->vipMutex);
	std::lock_guard<std::mutex> lock(internal->mutex);
	internal->threadCount = threadCount;
}

int Engine::getThreadCount() {
	// No lock
	return internal->threadCount;
}

void Engine::setPaused(bool paused) {
	// No lock
	internal->paused = paused;
}

bool Engine::isPaused() {
	// No lock
	return internal->paused;
}

void Engine::addModule(Module *module) {
	assert(module);
	VIPLock vipLock(internal->vipMutex);
	std::lock_guard<std::mutex> lock(internal->mutex);
	// Check that the module is not already added
	auto it = std::find(internal->modules.begin(), internal->modules.end(), module);
	assert(it == internal->modules.end());
	// Set ID
	if (module->id < 0) {
		// Automatically assign ID
		module->id = internal->nextModuleId++;
	}
	else {
		// Manual ID
		// Check that the ID is not already taken
		for (Module *m : internal->modules) {
			assert(module->id != m->id);
		}
		if (module->id >= internal->nextModuleId) {
			internal->nextModuleId = module->id + 1;
		}
	}
	// Add module
	internal->modules.push_back(module);
}

void Engine::removeModule(Module *module) {
	assert(module);
	VIPLock vipLock(internal->vipMutex);
	std::lock_guard<std::mutex> lock(internal->mutex);
	// If a param is being smoothed on this module, stop smoothing it immediately
	if (module == internal->smoothModule) {
		internal->smoothModule = NULL;
	}
	// Check that all cables are disconnected
	for (Cable *cable : internal->cables) {
		assert(cable->outputModule != module);
		assert(cable->inputModule != module);
	}
	// Check that the module actually exists
	auto it = std::find(internal->modules.begin(), internal->modules.end(), module);
	assert(it != internal->modules.end());
	// Remove the module
	internal->modules.erase(it);
	// Remove id
	module->id = -1;
}

void Engine::resetModule(Module *module) {
	assert(module);
	VIPLock vipLock(internal->vipMutex);
	std::lock_guard<std::mutex> lock(internal->mutex);

	module->reset();
}

void Engine::randomizeModule(Module *module) {
	assert(module);
	VIPLock vipLock(internal->vipMutex);
	std::lock_guard<std::mutex> lock(internal->mutex);

	module->randomize();
}

void Engine::bypassModule(Module *module, bool bypass) {
	assert(module);
	VIPLock vipLock(internal->vipMutex);
	std::lock_guard<std::mutex> lock(internal->mutex);
	if (bypass) {
		for (Output &output : module->outputs) {
			// This also zeros all voltages
			output.setChannels(0);
		}
		module->cpuTime = 0.f;
	}
	else {
		// Set all outputs to 1 channel
		for (Output &output : module->outputs) {
			output.setChannels(1);
		}
	}
	module->bypass = bypass;
}

static void Engine_updateConnected(Engine *engine) {
	// Set everything to unconnected
	for (Module *module : engine->internal->modules) {
		for (Input &input : module->inputs) {
			input.active = false;
		}
		for (Output &output : module->outputs) {
			output.active = false;
		}
	}
	// Set inputs/outputs to active
	for (Cable *cable : engine->internal->cables) {
		cable->outputModule->outputs[cable->outputId].active = true;
		cable->inputModule->inputs[cable->inputId].active = true;
	}
}

void Engine::addCable(Cable *cable) {
	assert(cable);
	VIPLock vipLock(internal->vipMutex);
	std::lock_guard<std::mutex> lock(internal->mutex);
	// Check cable properties
	assert(cable->outputModule);
	assert(cable->inputModule);
	// Check that the cable is not already added, and that the input is not already used by another cable
	for (Cable *cable2 : internal->cables) {
		assert(cable2 != cable);
		assert(!(cable2->inputModule == cable->inputModule && cable2->inputId == cable->inputId));
	}
	// Set ID
	if (cable->id < 0) {
		// Automatically assign ID
		cable->id = internal->nextCableId++;
	}
	else {
		// Manual ID
		// Check that the ID is not already taken
		for (Cable *w : internal->cables) {
			assert(cable->id != w->id);
		}
		if (cable->id >= internal->nextCableId) {
			internal->nextCableId = cable->id + 1;
		}
	}
	// Add the cable
	internal->cables.push_back(cable);
	Engine_updateConnected(this);
}

void Engine::removeCable(Cable *cable) {
	assert(cable);
	VIPLock vipLock(internal->vipMutex);
	std::lock_guard<std::mutex> lock(internal->mutex);
	// Check that the cable is already added
	auto it = std::find(internal->cables.begin(), internal->cables.end(), cable);
	assert(it != internal->cables.end());
	// Set input to inactive
	Input &input = cable->inputModule->inputs[cable->inputId];
	input.setChannels(0);
	// Remove the cable
	internal->cables.erase(it);
	Engine_updateConnected(this);
	// Remove ID
	cable->id = -1;
}

void Engine::setParam(Module *module, int paramId, float value) {
	// TODO Does this need to be thread-safe?
	module->params[paramId].value = value;
}

float Engine::getParam(Module *module, int paramId) {
	return module->params[paramId].value;
}

void Engine::setSmoothParam(Module *module, int paramId, float value) {
	// If another param is being smoothed, jump value
	if (internal->smoothModule && !(internal->smoothModule == module && internal->smoothParamId == paramId)) {
		internal->smoothModule->params[internal->smoothParamId].value = internal->smoothValue;
	}
	internal->smoothParamId = paramId;
	internal->smoothValue = value;
	internal->smoothModule = module;
}

float Engine::getSmoothParam(Module *module, int paramId) {
	if (internal->smoothModule == module && internal->smoothParamId == paramId)
		return internal->smoothValue;
	return getParam(module, paramId);
}

int Engine::getNextModuleId() {
	return internal->nextModuleId++;
}

void Engine::setSampleRate(float newSampleRate) {
	internal->sampleRateRequested = newSampleRate;
}

float Engine::getSampleRate() {
	return internal->sampleRate;
}

float Engine::getSampleTime() {
	return internal->sampleTime;
}


void EngineWorker::step() {
	Engine_stepModules(engine, id);
	engine->internal->workerBarrier.wait();
	engine->internal->engineBarrier.wait();
}


} // namespace engine
} // namespace rack

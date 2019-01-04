#include "engine/Engine.hpp"
#include "settings.hpp"

#include <algorithm>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <xmmintrin.h>
#include <pmmintrin.h>


namespace rack {


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


struct Engine::Internal {
	bool running = false;
	float sampleRate;
	float sampleTime;
	float sampleRateRequested;

	Module *resetModule = NULL;
	Module *randomizeModule = NULL;
	int nextModuleId = 1;
	int nextWireId = 1;

	// Parameter smoothing
	Module *smoothModule = NULL;
	int smoothParamId;
	float smoothValue;

	std::mutex mutex;
	std::thread thread;
	VIPMutex vipMutex;
};


Engine::Engine() {
	internal = new Internal;

	float sampleRate = 44100.f;
	internal->sampleRate = sampleRate;
	internal->sampleTime = 1 / sampleRate;
	internal->sampleRateRequested = sampleRate;
}

Engine::~Engine() {
	// Make sure there are no wires or modules in the rack on destruction. This suggests that a module failed to remove itself before the RackWidget was destroyed.
	assert(wires.empty());
	assert(modules.empty());

	delete internal;
}

static void Engine_step(Engine *engine) {
	// Sample rate
	if (engine->internal->sampleRateRequested != engine->internal->sampleRate) {
		engine->internal->sampleRate = engine->internal->sampleRateRequested;
		engine->internal->sampleTime = 1 / engine->internal->sampleRate;
		for (Module *module : engine->modules) {
			module->onSampleRateChange();
		}
	}

	// Events
	if (engine->internal->resetModule) {
		engine->internal->resetModule->reset();
		engine->internal->resetModule = NULL;
	}
	if (engine->internal->randomizeModule) {
		engine->internal->randomizeModule->randomize();
		engine->internal->randomizeModule = NULL;
	}

	// Param smoothing
	{
		Module *smoothModule = engine->internal->smoothModule;
		int smoothParamId = engine->internal->smoothParamId;
		float smoothValue = engine->internal->smoothValue;
		if (smoothModule) {
			Param *param = &smoothModule->params[smoothParamId];
			float value = param->value;
			// decay rate is 1 graphics frame
			const float lambda = 60.f;
			float delta = smoothValue - value;
			float newValue = value + delta * lambda * engine->internal->sampleTime;
			if (value == newValue) {
				// Snap to actual smooth value if the value doesn't change enough (due to the granularity of floats)
				param->value = smoothValue;
				engine->internal->smoothModule = NULL;
			}
			else {
				param->value = newValue;
			}
		}
	}

	// Iterate modules
	for (Module *module : engine->modules) {
		if (module->bypass) {
			// Bypass module
			for (Output &output : module->outputs) {
				output.numChannels = 1;
				output.setValue(0.f);
			}
			module->cpuTime = 0.f;
		}
		else {
			// Step module
			if (settings::powerMeter) {
				auto startTime = std::chrono::high_resolution_clock::now();

				module->step();

				auto stopTime = std::chrono::high_resolution_clock::now();
				float cpuTime = std::chrono::duration<float>(stopTime - startTime).count() * engine->internal->sampleRate;
				// Smooth cpu time
				module->cpuTime += (cpuTime - module->cpuTime) * engine->internal->sampleTime / 0.5f;
			}
			else {
				module->step();
			}
		}

		// Iterate ports and step plug lights
		for (Input &input : module->inputs) {
			if (input.active) {
				float value = input.value / 5.f;
				input.plugLights[0].setBrightnessSmooth(value);
				input.plugLights[1].setBrightnessSmooth(-value);
			}
		}
		for (Output &output : module->outputs) {
			if (output.active) {
				float value = output.value / 5.f;
				output.plugLights[0].setBrightnessSmooth(value);
				output.plugLights[1].setBrightnessSmooth(-value);
			}
		}
	}

	// Step cables
	for (Wire *wire : engine->wires) {
		wire->step();
	}
}

static void Engine_run(Engine *engine) {
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

		if (!engine->paused) {
			std::lock_guard<std::mutex> lock(engine->internal->mutex);
			for (int i = 0; i < mutexSteps; i++) {
				Engine_step(engine);
			}
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
}

void Engine::start() {
	internal->running = true;
	internal->thread = std::thread(Engine_run, this);
}

void Engine::stop() {
	internal->running = false;
	internal->thread.join();
}

void Engine::addModule(Module *module) {
	assert(module);
	VIPLock vipLock(internal->vipMutex);
	std::lock_guard<std::mutex> lock(internal->mutex);
	// Set ID
	assert(module->id == 0);
	module->id = internal->nextModuleId++;
	// Check that the module is not already added
	auto it = std::find(modules.begin(), modules.end(), module);
	assert(it == modules.end());
	modules.push_back(module);
}

void Engine::removeModule(Module *module) {
	assert(module);
	VIPLock vipLock(internal->vipMutex);
	std::lock_guard<std::mutex> lock(internal->mutex);
	// If a param is being smoothed on this module, stop smoothing it immediately
	if (module == internal->smoothModule) {
		internal->smoothModule = NULL;
	}
	// Check that all wires are disconnected
	for (Wire *wire : wires) {
		assert(wire->outputModule != module);
		assert(wire->inputModule != module);
	}
	// Check that the module actually exists
	auto it = std::find(modules.begin(), modules.end(), module);
	assert(it != modules.end());
	// Remove the module
	modules.erase(it);
	// Remove id
	module->id = 0;
}

void Engine::resetModule(Module *module) {
	internal->resetModule = module;
}

void Engine::randomizeModule(Module *module) {
	internal->randomizeModule = module;
}

static void Engine_updateActive(Engine *engine) {
	// Set everything to inactive
	for (Module *module : engine->modules) {
		for (Input &input : module->inputs) {
			input.active = false;
		}
		for (Output &output : module->outputs) {
			output.active = false;
		}
	}
	// Set inputs/outputs to active
	for (Wire *wire : engine->wires) {
		wire->outputModule->outputs[wire->outputId].active = true;
		wire->inputModule->inputs[wire->inputId].active = true;
	}
}

void Engine::addWire(Wire *wire) {
	assert(wire);
	VIPLock vipLock(internal->vipMutex);
	std::lock_guard<std::mutex> lock(internal->mutex);
	// Check wire properties
	assert(wire->outputModule);
	assert(wire->inputModule);
	// Check that the wire is not already added, and that the input is not already used by another cable
	for (Wire *wire2 : wires) {
		assert(wire2 != wire);
		assert(!(wire2->inputModule == wire->inputModule && wire2->inputId == wire->inputId));
	}
	// Set ID
	assert(wire->id == 0);
	wire->id = internal->nextWireId++;
	// Add the wire
	wires.push_back(wire);
	Engine_updateActive(this);
}

void Engine::removeWire(Wire *wire) {
	assert(wire);
	VIPLock vipLock(internal->vipMutex);
	std::lock_guard<std::mutex> lock(internal->mutex);
	// Check that the wire is already added
	auto it = std::find(wires.begin(), wires.end(), wire);
	assert(it != wires.end());
	// Set input to 0V
	wire->inputModule->inputs[wire->inputId].value = 0.f;
	// Remove the wire
	wires.erase(it);
	Engine_updateActive(this);
	// Remove ID
	wire->id = 0;
}

void Engine::setParam(Module *module, int paramId, float value) {
	// TODO Make thread safe
	module->params[paramId].value = value;
}

void Engine::setParamSmooth(Module *module, int paramId, float value) {
	// If another param is being smoothed, jump value
	if (internal->smoothModule && !(internal->smoothModule == module && internal->smoothParamId == paramId)) {
		internal->smoothModule->params[internal->smoothParamId].value = internal->smoothValue;
	}
	internal->smoothParamId = paramId;
	internal->smoothValue = value;
	internal->smoothModule = module;
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


} // namespace rack

#include <algorithm>
#include <set>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <tuple>
#include <pmmintrin.h>
#include <pthread.h>

#include <engine/Engine.hpp>
#include <settings.hpp>
#include <system.hpp>
#include <random.hpp>
#include <context.hpp>
#include <patch.hpp>
#include <plugin.hpp>


namespace rack {
namespace engine {


static void initMXCSR() {
	// Set CPU to flush-to-zero (FTZ) and denormals-are-zero (DAZ) mode
	// https://software.intel.com/en-us/node/682949
	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
	_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
	// Reset other flags
	_MM_SET_ROUNDING_MODE(_MM_ROUND_NEAREST);
}


/** Allows multiple "reader" threads to obtain a lock simultaneously, but only one "writer" thread.
This implementation is just a wrapper for pthreads.
This is available in C++14 as std::shared_mutex, but unfortunately we're using C++11.
*/
struct SharedMutex {
	pthread_rwlock_t rwlock;
	SharedMutex() {
		if (pthread_rwlock_init(&rwlock, NULL))
			throw Exception("pthread_rwlock_init failed");
	}
	~SharedMutex() {
		pthread_rwlock_destroy(&rwlock);
	}
};

struct SharedLock {
	SharedMutex& m;
	SharedLock(SharedMutex& m) : m(m) {
		if (pthread_rwlock_rdlock(&m.rwlock))
			throw Exception("pthread_rwlock_rdlock failed");
	}
	~SharedLock() {
		pthread_rwlock_unlock(&m.rwlock);
	}
};

struct ExclusiveSharedLock {
	SharedMutex& m;
	ExclusiveSharedLock(SharedMutex& m) : m(m) {
		if (pthread_rwlock_wrlock(&m.rwlock))
			throw Exception("pthread_rwlock_wrlock failed");
	}
	~ExclusiveSharedLock() {
		pthread_rwlock_unlock(&m.rwlock);
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
		int id = ++count;
		if (id == total) {
			count = 0;
			cv.notify_all();
		}
		else {
			cv.wait(lock);
		}
	}
};


struct SpinBarrier {
	std::atomic<int> count{0};
	int total = 0;

	void wait() {
		int id = ++count;
		if (id == total) {
			count = 0;
		}
		else {
			while (count != 0) {
				_mm_pause();
			}
		}
	}
};


/** Spinlocks until all `total` threads are waiting.
If `yield` is set to true at any time, all threads will switch to waiting on a mutex instead.
All threads must return before beginning a new phase. Alternating between two barriers solves this problem.
*/
struct HybridBarrier {
	std::atomic<int> count {0};
	int total = 0;

	std::mutex mutex;
	std::condition_variable cv;

	std::atomic<bool> yield {false};

	void wait() {
		int id = ++count;

		// End and reset phase if this is the last thread
		if (id == total) {
			count = 0;
			if (yield) {
				std::unique_lock<std::mutex> lock(mutex);
				cv.notify_all();
				yield = false;
			}
			return;
		}

		// Spinlock
		while (!yield) {
			if (count == 0)
				return;
			_mm_pause();
		}

		// Wait on mutex
		{
			std::unique_lock<std::mutex> lock(mutex);
			cv.wait(lock, [&] {
				return count == 0;
			});
		}
	}
};


struct EngineWorker {
	Engine* engine;
	int id;
	std::thread thread;
	bool running = false;

	void start() {
		assert(!running);
		running = true;
		thread = std::thread([&] {
			run();
		});
	}

	void requestStop() {
		running = false;
	}

	void join() {
		assert(thread.joinable());
		thread.join();
	}

	void run();
};


struct Engine::Internal {
	std::vector<Module*> modules;
	std::vector<Cable*> cables;
	std::set<ParamHandle*> paramHandles;

	// moduleId
	std::map<int64_t, Module*> modulesCache;
	// cableId
	std::map<int64_t, Cable*> cablesCache;
	// (moduleId, paramId)
	std::map<std::tuple<int64_t, int>, ParamHandle*> paramHandlesCache;

	float sampleRate = 0.f;
	float sampleTime = 0.f;
	int64_t frame = 0;
	int64_t stepFrame = 0;
	int64_t stepTime = 0;
	int stepFrames = 0;
	Module* primaryModule = NULL;

	// Parameter smoothing
	Module* smoothModule = NULL;
	int smoothParamId = 0;
	float smoothValue = 0.f;

	/** Engine mutex
	Writers lock when mutating the engine's Modules, Cables, etc.
	Readers lock when using the engine's Modules, Cables, etc.
	*/
	SharedMutex mutex;
	/** Step mutex
	step() locks to guarantee its exclusivity.
	*/
	std::mutex stepMutex;

	int threadCount = 0;
	std::vector<EngineWorker> workers;
	HybridBarrier engineBarrier;
	HybridBarrier workerBarrier;
	std::atomic<int> workerModuleIndex;
	Context* context;
};


static void Engine_updateExpander(Engine* that, Module* module, bool side) {
	Module::Expander& expander = side ? module->rightExpander : module->leftExpander;
	Module* oldExpanderModule = expander.module;

	if (expander.moduleId >= 0) {
		if (!expander.module || expander.module->id != expander.moduleId) {
			expander.module = that->getModule(expander.moduleId);
		}
	}
	else {
		if (expander.module) {
			expander.module = NULL;
		}
	}

	if (expander.module != oldExpanderModule) {
		// Trigger ExpanderChangeEvent event
		Module::ExpanderChangeEvent e;
		e.side = side;
		module->onExpanderChange(e);
	}
}


static void Engine_relaunchWorkers(Engine* that, int threadCount) {
	Engine::Internal* internal = that->internal;
	if (threadCount == internal->threadCount)
		return;

	if (internal->threadCount > 0) {
		// Stop engine workers
		for (EngineWorker& worker : internal->workers) {
			worker.requestStop();
		}
		internal->engineBarrier.wait();

		// Join and destroy engine workers
		for (EngineWorker& worker : internal->workers) {
			worker.join();
		}
		internal->workers.resize(0);
	}

	// Configure engine
	internal->threadCount = threadCount;

	// Set barrier counts
	internal->engineBarrier.total = threadCount;
	internal->workerBarrier.total = threadCount;

	if (threadCount > 0) {
		// Create and start engine workers
		internal->workers.resize(threadCount - 1);
		for (int id = 1; id < threadCount; id++) {
			EngineWorker& worker = internal->workers[id - 1];
			worker.id = id;
			worker.engine = that;
			worker.start();
		}
	}
}


static void Port_step(Port* that, float deltaTime) {
	// Set plug lights
	if (that->channels == 0) {
		that->plugLights[0].setBrightness(0.f);
		that->plugLights[1].setBrightness(0.f);
		that->plugLights[2].setBrightness(0.f);
	}
	else if (that->channels == 1) {
		float v = that->getVoltage() / 10.f;
		that->plugLights[0].setSmoothBrightness(v, deltaTime);
		that->plugLights[1].setSmoothBrightness(-v, deltaTime);
		that->plugLights[2].setBrightness(0.f);
	}
	else {
		float v2 = 0.f;
		for (int c = 0; c < that->channels; c++) {
			v2 += std::pow(that->getVoltage(c), 2);
		}
		float v = std::sqrt(v2) / 10.f;
		that->plugLights[0].setBrightness(0.f);
		that->plugLights[1].setBrightness(0.f);
		that->plugLights[2].setSmoothBrightness(v, deltaTime);
	}
}


static void Engine_stepModulesWorker(Engine* that, int threadId) {
	Engine::Internal* internal = that->internal;

	// int threadCount = internal->threadCount;
	int modulesLen = internal->modules.size();

	Module::ProcessArgs processArgs;
	processArgs.sampleRate = internal->sampleRate;
	processArgs.sampleTime = internal->sampleTime;

	bool cpuMeter = settings::cpuMeter;

	// Step each module
	while (true) {
		// Choose next module
		// First-come-first serve module-to-thread allocation algorithm
		int i = internal->workerModuleIndex++;
		if (i >= modulesLen)
			break;

		Module* module = internal->modules[i];

		// Start CPU timer
		int64_t startTime;
		if (cpuMeter) {
			startTime = system::getNanoseconds();
		}

		// Step module
		if (!module->bypass())
			module->process(processArgs);
		else
			module->processBypass(processArgs);

		// Stop CPU timer
		if (cpuMeter) {
			int64_t endTime = system::getNanoseconds();
			float duration = (endTime - startTime) * 1e-9f;

			// Smooth CPU time
			const float cpuTau = 2.f /* seconds */;
			module->cpuTime() += (duration - module->cpuTime()) * processArgs.sampleTime / cpuTau;
		}

		// Iterate ports to step plug lights
		const int portDivider = 8;
		if (internal->frame % portDivider == 0) {
			float portTime = processArgs.sampleTime * portDivider;
			for (Input& input : module->inputs) {
				Port_step(&input, portTime);
			}
			for (Output& output : module->outputs) {
				Port_step(&output, portTime);
			}
		}
	}
}


static void Cable_step(Cable* that) {
	Output* output = &that->outputModule->outputs[that->outputId];
	Input* input = &that->inputModule->inputs[that->inputId];
	// Match number of polyphonic channels to output port
	int channels = output->channels;
	// Copy all voltages from output to input
	for (int c = 0; c < channels; c++) {
		float v = output->voltages[c];
		// Set 0V if infinite or NaN
		if (!std::isfinite(v))
			v = 0.f;
		input->voltages[c] = v;
	}
	// Set higher channel voltages to 0
	for (int c = channels; c < input->channels; c++) {
		input->voltages[c] = 0.f;
	}
	input->channels = channels;
}


/** Steps a single frame
*/
static void Engine_stepFrame(Engine* that) {
	Engine::Internal* internal = that->internal;

	// Param smoothing
	Module* smoothModule = internal->smoothModule;
	if (smoothModule) {
		int smoothParamId = internal->smoothParamId;
		float smoothValue = internal->smoothValue;
		Param* smoothParam = &smoothModule->params[smoothParamId];
		float value = smoothParam->value;
		// Use decay rate of roughly 1 graphics frame
		const float smoothLambda = 60.f;
		float newValue = value + (smoothValue - value) * smoothLambda * internal->sampleTime;
		if (value == newValue) {
			// Snap to actual smooth value if the value doesn't change enough (due to the granularity of floats)
			smoothParam->setValue(smoothValue);
			internal->smoothModule = NULL;
			internal->smoothParamId = 0;
		}
		else {
			smoothParam->setValue(newValue);
		}
	}

	// Step cables
	for (Cable* cable : that->internal->cables) {
		Cable_step(cable);
	}

	// Flip messages for each module
	for (Module* module : that->internal->modules) {
		if (module->leftExpander.messageFlipRequested) {
			std::swap(module->leftExpander.producerMessage, module->leftExpander.consumerMessage);
			module->leftExpander.messageFlipRequested = false;
		}
		if (module->rightExpander.messageFlipRequested) {
			std::swap(module->rightExpander.producerMessage, module->rightExpander.consumerMessage);
			module->rightExpander.messageFlipRequested = false;
		}
	}

	// Step modules along with workers
	internal->workerModuleIndex = 0;
	internal->engineBarrier.wait();
	Engine_stepModulesWorker(that, 0);
	internal->workerBarrier.wait();

	internal->frame++;
}


static void Port_setDisconnected(Port* that) {
	that->channels = 0;
	for (int c = 0; c < PORT_MAX_CHANNELS; c++) {
		that->voltages[c] = 0.f;
	}
}


static void Port_setConnected(Port* that) {
	if (that->channels > 0)
		return;
	that->channels = 1;
}


static void Engine_updateConnected(Engine* that) {
	// Find disconnected ports
	std::set<Port*> disconnectedPorts;
	for (Module* module : that->internal->modules) {
		for (Input& input : module->inputs) {
			disconnectedPorts.insert(&input);
		}
		for (Output& output : module->outputs) {
			disconnectedPorts.insert(&output);
		}
	}
	for (Cable* cable : that->internal->cables) {
		// Connect input
		Input& input = cable->inputModule->inputs[cable->inputId];
		auto inputIt = disconnectedPorts.find(&input);
		if (inputIt != disconnectedPorts.end())
			disconnectedPorts.erase(inputIt);
		Port_setConnected(&input);
		// Connect output
		Output& output = cable->outputModule->outputs[cable->outputId];
		auto outputIt = disconnectedPorts.find(&output);
		if (outputIt != disconnectedPorts.end())
			disconnectedPorts.erase(outputIt);
		Port_setConnected(&output);
	}
	// Disconnect ports that have no cable
	for (Port* port : disconnectedPorts) {
		Port_setDisconnected(port);
	}
}


static void Engine_refreshParamHandleCache(Engine* that) {
	// Clear cache
	that->internal->paramHandlesCache.clear();
	// Add active ParamHandles to cache
	for (ParamHandle* paramHandle : that->internal->paramHandles) {
		if (paramHandle->moduleId >= 0) {
			that->internal->paramHandlesCache[std::make_tuple(paramHandle->moduleId, paramHandle->paramId)] = paramHandle;
		}
	}
}


Engine::Engine() {
	internal = new Internal;

	internal->context = contextGet();
	internal->sampleRate = 44100.f;
	internal->sampleTime = 1 / internal->sampleRate;
}


Engine::~Engine() {
	Engine_relaunchWorkers(this, 0);
	clear();

	// Make sure there are no cables or modules in the rack on destruction.
	// If this happens, a module must have failed to remove itself before the RackWidget was destroyed.
	assert(internal->cables.empty());
	assert(internal->modules.empty());
	assert(internal->paramHandles.empty());

	assert(internal->modulesCache.empty());
	assert(internal->cablesCache.empty());
	assert(internal->paramHandlesCache.empty());

	delete internal;
}


void Engine::clear() {
	// TODO This needs a lock that doesn't interfere with removeParamHandle, removeCable, and removeModule.

	// Copy lists because we'll be removing while iterating
	std::set<ParamHandle*> paramHandles = internal->paramHandles;
	for (ParamHandle* paramHandle : paramHandles) {
		removeParamHandle(paramHandle);
		// Don't delete paramHandle because they're owned by other things (e.g. Modules)
	}
	std::vector<Cable*> cables = internal->cables;
	for (Cable* cable : cables) {
		removeCable(cable);
		delete cable;
	}
	std::vector<Module*> modules = internal->modules;
	for (Module* module : modules) {
		removeModule(module);
		delete module;
	}
}


void Engine::step(int frames) {
	std::lock_guard<std::mutex> stepLock(internal->stepMutex);
	SharedLock lock(internal->mutex);
	// Configure thread
	initMXCSR();
	random::init();

	internal->stepFrame = internal->frame;
	internal->stepTime = system::getNanoseconds();
	internal->stepFrames = frames;

	// Set sample rate
	if (internal->sampleRate != settings::sampleRate) {
		internal->sampleRate = settings::sampleRate;
		internal->sampleTime = 1.f / internal->sampleRate;
		Module::SampleRateChangeEvent e;
		e.sampleRate = internal->sampleRate;
		e.sampleTime = internal->sampleTime;
		for (Module* module : internal->modules) {
			module->onSampleRateChange(e);
		}
	}

	// Update expander pointers
	for (Module* module : internal->modules) {
		Engine_updateExpander(this, module, false);
		Engine_updateExpander(this, module, true);
	}

	// Launch workers
	Engine_relaunchWorkers(this, settings::threadCount);

	// Step individual frames
	for (int i = 0; i < frames; i++) {
		Engine_stepFrame(this);
	}

	yieldWorkers();
}


void Engine::setPrimaryModule(Module* module) {
	SharedLock lock(internal->mutex);
	// Don't allow module to be set if not added to the Engine.
	// NULL will unset the primary module.
	if (module) {
		auto it = std::find(internal->modules.begin(), internal->modules.end(), module);
		if (it == internal->modules.end())
			return;
	}
	internal->primaryModule = module;
}


Module* Engine::getPrimaryModule() {
	return internal->primaryModule;
}


float Engine::getSampleRate() {
	return internal->sampleRate;
}


float Engine::getSampleTime() {
	return internal->sampleTime;
}


void Engine::yieldWorkers() {
	internal->workerBarrier.yield = true;
}


int64_t Engine::getFrame() {
	return internal->frame;
}


int64_t Engine::getFrameTime() {
	double timeSinceStep = (internal->frame - internal->stepFrame) * internal->sampleTime;
	return internal->stepTime + int64_t(timeSinceStep * 1e9);
}


int64_t Engine::getStepFrame() {
	return internal->stepFrame;
}


int64_t Engine::getStepTime() {
	return internal->stepTime;
}


int Engine::getStepFrames() {
	return internal->stepFrames;
}


int64_t Engine::getStepDuration() {
	double duration = internal->stepFrames * internal->sampleTime;
	return int64_t(duration * 1e9);
}


size_t Engine::getNumModules() {
	return internal->modules.size();
}


size_t Engine::getModuleIds(int64_t* moduleIds, size_t len) {
	SharedLock lock(internal->mutex);
	size_t i = 0;
	for (Module* m : internal->modules) {
		if (i >= len)
			break;
		moduleIds[i] = m->id;
		i++;
	}
	return i;
}


std::vector<int64_t> Engine::getModuleIds() {
	SharedLock lock(internal->mutex);
	std::vector<int64_t> moduleIds;
	moduleIds.reserve(internal->modules.size());
	for (Module* m : internal->modules) {
		moduleIds.push_back(m->id);
	}
	return moduleIds;
}


void Engine::addModule(Module* module) {
	ExclusiveSharedLock lock(internal->mutex);
	assert(module);
	// Check that the module is not already added
	auto it = std::find(internal->modules.begin(), internal->modules.end(), module);
	assert(it == internal->modules.end());
	// Set ID if unset or collides with an existing ID
	while (module->id < 0 || internal->modulesCache.find(module->id) != internal->modulesCache.end()) {
		// Randomly generate ID
		module->id = random::u64() % (1ul << 53);
	}
	// Add module
	internal->modules.push_back(module);
	internal->modulesCache[module->id] = module;
	// Trigger Add event
	Module::AddEvent eAdd;
	module->onAdd(eAdd);
	// Update ParamHandles' module pointers
	for (ParamHandle* paramHandle : internal->paramHandles) {
		if (paramHandle->moduleId == module->id)
			paramHandle->module = module;
	}
}


void Engine::removeModule(Module* module) {
	ExclusiveSharedLock lock(internal->mutex);
	assert(module);
	// Check that the module actually exists
	auto it = std::find(internal->modules.begin(), internal->modules.end(), module);
	assert(it != internal->modules.end());
	// If a param is being smoothed on this module, stop smoothing it immediately
	if (module == internal->smoothModule) {
		internal->smoothModule = NULL;
	}
	// Check that all cables are disconnected
	for (Cable* cable : internal->cables) {
		assert(cable->inputModule != module);
		assert(cable->outputModule != module);
	}
	// Update ParamHandles' module pointers
	for (ParamHandle* paramHandle : internal->paramHandles) {
		if (paramHandle->moduleId == module->id)
			paramHandle->module = NULL;
	}
	// Update expander pointers
	for (Module* m : internal->modules) {
		if (m->leftExpander.module == module) {
			m->leftExpander.moduleId = -1;
			m->leftExpander.module = NULL;
		}
		if (m->rightExpander.module == module) {
			m->rightExpander.moduleId = -1;
			m->rightExpander.module = NULL;
		}
	}
	// Trigger Remove event
	Module::RemoveEvent eRemove;
	module->onRemove(eRemove);
	// Unset primary module
	if (internal->primaryModule == module)
		internal->primaryModule = NULL;
	// Remove module
	internal->modulesCache.erase(module->id);
	internal->modules.erase(it);
}


Module* Engine::getModule(int64_t moduleId) {
	SharedLock lock(internal->mutex);
	auto it = internal->modulesCache.find(moduleId);
	if (it == internal->modulesCache.end())
		return NULL;
	return it->second;
}


void Engine::resetModule(Module* module) {
	ExclusiveSharedLock lock(internal->mutex);
	assert(module);

	Module::ResetEvent eReset;
	module->onReset(eReset);
}


void Engine::randomizeModule(Module* module) {
	ExclusiveSharedLock lock(internal->mutex);
	assert(module);

	Module::RandomizeEvent eRandomize;
	module->onRandomize(eRandomize);
}


void Engine::bypassModule(Module* module, bool bypass) {
	ExclusiveSharedLock lock(internal->mutex);
	assert(module);

	if (module->bypass() == bypass)
		return;
	// Clear outputs and set to 1 channel
	for (Output& output : module->outputs) {
		// This zeros all voltages, but the channel is set to 1 if connected
		output.setChannels(0);
	}
	// Set bypass state
	module->bypass() = bypass;
	// Trigger event
	if (bypass) {
		Module::BypassEvent eBypass;
		module->onBypass(eBypass);
	}
	else {
		Module::UnBypassEvent eUnBypass;
		module->onUnBypass(eUnBypass);
	}
}


json_t* Engine::moduleToJson(Module* module) {
	ExclusiveSharedLock lock(internal->mutex);
	return module->toJson();
}


void Engine::moduleFromJson(Module* module, json_t* rootJ) {
	ExclusiveSharedLock lock(internal->mutex);
	module->fromJson(rootJ);
}


size_t Engine::getNumCables() {
	return internal->cables.size();
}


size_t Engine::getCableIds(int64_t* cableIds, size_t len) {
	SharedLock lock(internal->mutex);
	size_t i = 0;
	for (Cable* c : internal->cables) {
		if (i >= len)
			break;
		cableIds[i] = c->id;
		i++;
	}
	return i;
}


std::vector<int64_t> Engine::getCableIds() {
	SharedLock lock(internal->mutex);
	std::vector<int64_t> cableIds;
	cableIds.reserve(internal->cables.size());
	for (Cable* c : internal->cables) {
		cableIds.push_back(c->id);
	}
	return cableIds;
}


void Engine::addCable(Cable* cable) {
	ExclusiveSharedLock lock(internal->mutex);
	assert(cable);
	// Check cable properties
	assert(cable->inputModule);
	assert(cable->outputModule);
	bool outputWasConnected = false;
	for (Cable* cable2 : internal->cables) {
		// Check that the cable is not already added
		assert(cable2 != cable);
		// Check that the input is not already used by another cable
		assert(!(cable2->inputModule == cable->inputModule && cable2->inputId == cable->inputId));
		// Get connected status of output, to decide whether we need to call a PortChangeEvent.
		// It's best to not trust `cable->outputModule->outputs[cable->outputId]->isConnected()`
		if (cable2->outputModule == cable->outputModule && cable2->outputId == cable->outputId)
			outputWasConnected = true;
	}
	// Set ID if unset or collides with an existing ID
	while (cable->id < 0 || internal->cablesCache.find(cable->id) != internal->cablesCache.end()) {
		// Randomly generate ID
		cable->id = random::u64() % (1ul << 53);
	}
	// Add the cable
	internal->cables.push_back(cable);
	internal->cablesCache[cable->id] = cable;
	Engine_updateConnected(this);
	// Trigger input port event
	{
		Module::PortChangeEvent e;
		e.connecting = true;
		e.type = Port::INPUT;
		e.portId = cable->inputId;
		cable->inputModule->onPortChange(e);
	}
	// Trigger output port event if its state went from disconnected to connected.
	if (!outputWasConnected) {
		Module::PortChangeEvent e;
		e.connecting = true;
		e.type = Port::OUTPUT;
		e.portId = cable->outputId;
		cable->outputModule->onPortChange(e);
	}
}


void Engine::removeCable(Cable* cable) {
	ExclusiveSharedLock lock(internal->mutex);
	assert(cable);
	// Check that the cable is already added
	auto it = std::find(internal->cables.begin(), internal->cables.end(), cable);
	assert(it != internal->cables.end());
	// Remove the cable
	internal->cablesCache.erase(cable->id);
	internal->cables.erase(it);
	Engine_updateConnected(this);
	bool outputIsConnected = false;
	for (Cable* cable2 : internal->cables) {
		// Get connected status of output, to decide whether we need to call a PortChangeEvent.
		// It's best to not trust `cable->outputModule->outputs[cable->outputId]->isConnected()`
		if (cable2->outputModule == cable->outputModule && cable2->outputId == cable->outputId)
			outputIsConnected = true;
	}
	// Trigger input port event
	{
		Module::PortChangeEvent e;
		e.connecting = false;
		e.type = Port::INPUT;
		e.portId = cable->inputId;
		cable->inputModule->onPortChange(e);
	}
	// Trigger output port event if its state went from connected to disconnected.
	if (!outputIsConnected) {
		Module::PortChangeEvent e;
		e.connecting = false;
		e.type = Port::OUTPUT;
		e.portId = cable->outputId;
		cable->outputModule->onPortChange(e);
	}
}


Cable* Engine::getCable(int64_t cableId) {
	SharedLock lock(internal->mutex);
	auto it = internal->cablesCache.find(cableId);
	if (it == internal->cablesCache.end())
		return NULL;
	return it->second;
}


void Engine::setParam(Module* module, int paramId, float value) {
	// If param is being smoothed, cancel smoothing.
	if (internal->smoothModule == module && internal->smoothParamId == paramId) {
		internal->smoothModule = NULL;
		internal->smoothParamId = 0;
	}
	module->params[paramId].value = value;
}


float Engine::getParam(Module* module, int paramId) {
	return module->params[paramId].value;
}


void Engine::setSmoothParam(Module* module, int paramId, float value) {
	// If another param is being smoothed, jump value
	if (internal->smoothModule && !(internal->smoothModule == module && internal->smoothParamId == paramId)) {
		internal->smoothModule->params[internal->smoothParamId].value = internal->smoothValue;
	}
	internal->smoothParamId = paramId;
	internal->smoothValue = value;
	// Set this last so the above values are valid as soon as it is set
	internal->smoothModule = module;
}


float Engine::getSmoothParam(Module* module, int paramId) {
	if (internal->smoothModule == module && internal->smoothParamId == paramId)
		return internal->smoothValue;
	return module->params[paramId].value;
}


void Engine::addParamHandle(ParamHandle* paramHandle) {
	ExclusiveSharedLock lock(internal->mutex);
	// New ParamHandles must be blank.
	// This means we don't have to refresh the cache.
	assert(paramHandle->moduleId < 0);

	// Check that the ParamHandle is not already added
	auto it = internal->paramHandles.find(paramHandle);
	assert(it == internal->paramHandles.end());

	// Add it
	internal->paramHandles.insert(paramHandle);
	// No need to refresh the cache because the moduleId is not set.
}


void Engine::removeParamHandle(ParamHandle* paramHandle) {
	ExclusiveSharedLock lock(internal->mutex);
	// Check that the ParamHandle is already added
	auto it = internal->paramHandles.find(paramHandle);
	assert(it != internal->paramHandles.end());

	// Remove it
	paramHandle->module = NULL;
	internal->paramHandles.erase(it);
	Engine_refreshParamHandleCache(this);
}


ParamHandle* Engine::getParamHandle(int64_t moduleId, int paramId) {
	SharedLock lock(internal->mutex);
	auto it = internal->paramHandlesCache.find(std::make_tuple(moduleId, paramId));
	if (it == internal->paramHandlesCache.end())
		return NULL;
	return it->second;
}


ParamHandle* Engine::getParamHandle(Module* module, int paramId) {
	return getParamHandle(module->id, paramId);
}


void Engine::updateParamHandle(ParamHandle* paramHandle, int64_t moduleId, int paramId, bool overwrite) {
	SharedLock lock(internal->mutex);
	// Check that it exists
	auto it = internal->paramHandles.find(paramHandle);
	assert(it != internal->paramHandles.end());

	// Set IDs
	paramHandle->moduleId = moduleId;
	paramHandle->paramId = paramId;
	paramHandle->module = NULL;
	// At this point, the ParamHandle cache might be invalid.

	if (paramHandle->moduleId >= 0) {
		// Replace old ParamHandle, or reset the current ParamHandle
		ParamHandle* oldParamHandle = getParamHandle(moduleId, paramId);
		if (oldParamHandle) {
			if (overwrite) {
				oldParamHandle->moduleId = -1;
				oldParamHandle->paramId = 0;
				oldParamHandle->module = NULL;
			}
			else {
				paramHandle->moduleId = -1;
				paramHandle->paramId = 0;
				paramHandle->module = NULL;
			}
		}
	}

	// Set module pointer if the above block didn't reset it
	if (paramHandle->moduleId >= 0) {
		paramHandle->module = getModule(paramHandle->moduleId);
	}

	Engine_refreshParamHandleCache(this);
}


json_t* Engine::toJson() {
	ExclusiveSharedLock lock(internal->mutex);
	json_t* rootJ = json_object();

	// modules
	json_t* modulesJ = json_array();
	for (Module* module : internal->modules) {
		// module
		json_t* moduleJ = module->toJson();
		json_array_append_new(modulesJ, moduleJ);
	}
	json_object_set_new(rootJ, "modules", modulesJ);

	// cables
	json_t* cablesJ = json_array();
	for (Cable* cable : internal->cables) {
		// cable
		json_t* cableJ = cable->toJson();
		json_array_append_new(cablesJ, cableJ);
	}
	json_object_set_new(rootJ, "cables", cablesJ);

	return rootJ;
}


void Engine::fromJson(json_t* rootJ) {
	// We can't lock here because addModule() and addCable() are called inside.
	// Also, AudioInterface::fromJson() can open the audio device, which can call Engine::step() before this method exits.
	// ExclusiveSharedLock lock(internal->mutex);
	clear();
	// modules
	json_t* modulesJ = json_object_get(rootJ, "modules");
	if (!modulesJ)
		return;
	size_t moduleIndex;
	json_t* moduleJ;
	json_array_foreach(modulesJ, moduleIndex, moduleJ) {
		try {
			plugin::Model* model = plugin::modelFromJson(moduleJ);
			Module* module = model->createModule();
			assert(module);
			// This doesn't need a lock because the Module is not added to the Engine yet.
			module->fromJson(moduleJ);

			// Before 1.0, the module ID was the index in the "modules" array
			if (APP->patch->isLegacy(2)) {
				module->id = moduleIndex;
			}

			// This method exclusively locks
			addModule(module);
		}
		catch (Exception& e) {
			APP->patch->log(e.what());
		}
	}

	// cables
	json_t* cablesJ = json_object_get(rootJ, "cables");
	// Before 1.0, cables were called wires
	if (!cablesJ)
		cablesJ = json_object_get(rootJ, "wires");
	if (!cablesJ)
		return;
	size_t cableIndex;
	json_t* cableJ;
	json_array_foreach(cablesJ, cableIndex, cableJ) {
		// cable
		Cable* cable = new Cable;
		try {
			cable->fromJson(cableJ);
			// This method exclusively locks
			addCable(cable);
		}
		catch (Exception& e) {
			delete cable;
			// Don't log exceptions because missing modules create unnecessary complaining when cables try to connect to them.
		}
	}
}


void EngineWorker::run() {
	// Configure thread
	contextSet(engine->internal->context);
	system::setThreadName(string::f("Worker %d", id));
	initMXCSR();
	random::init();

	while (true) {
		engine->internal->engineBarrier.wait();
		if (!running)
			return;
		Engine_stepModulesWorker(engine, id);
		engine->internal->workerBarrier.wait();
	}
}


} // namespace engine
} // namespace rack

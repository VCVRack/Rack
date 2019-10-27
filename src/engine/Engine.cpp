#include <engine/Engine.hpp>
#include <settings.hpp>
#include <system.hpp>
#include <random.hpp>
#include <app.hpp>
#include <patch.hpp>
#include <plugin.hpp>

#include <algorithm>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <tuple>
#include <pmmintrin.h>


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
	std::map<std::tuple<int, int>, ParamHandle*> paramHandleCache;
	bool paused = false;

	float sampleRate = 0.f;
	float sampleTime = 0.f;
	uint64_t frame = 0;
	Module* primaryModule = NULL;

	int nextModuleId = 0;
	int nextCableId = 0;

	// Parameter smoothing
	Module* smoothModule = NULL;
	int smoothParamId = 0;
	float smoothValue = 0.f;

	std::recursive_mutex mutex;

	bool realTime = false;
	int threadCount = 0;
	std::vector<EngineWorker> workers;
	HybridBarrier engineBarrier;
	HybridBarrier workerBarrier;
	std::atomic<int> workerModuleIndex;
};


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


static void Engine_stepModules(Engine* that, int threadId) {
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
		if (!module->disabled) {
			// Step module
			if (cpuMeter) {
				auto beginTime = std::chrono::high_resolution_clock::now();
				module->process(processArgs);
				auto endTime = std::chrono::high_resolution_clock::now();
				float duration = std::chrono::duration<float>(endTime - beginTime).count();

				// Smooth CPU time
				const float cpuTau = 2.f /* seconds */;
				module->cpuTime += (duration - module->cpuTime) * processArgs.sampleTime / cpuTau;
			}
			else {
				module->process(processArgs);
			}
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
	input->channels = channels;
	// Copy all voltages from output to input
	for (int i = 0; i < channels; i++) {
		input->voltages[i] = output->voltages[i];
	}
	// Clear all voltages of higher channels
	for (int i = channels; i < PORT_MAX_CHANNELS; i++) {
		input->voltages[i] = 0.f;
	}
}


static void Engine_step(Engine* that) {
	Engine::Internal* internal = that->internal;

	// Param smoothing
	Module* smoothModule = internal->smoothModule;
	int smoothParamId = internal->smoothParamId;
	float smoothValue = internal->smoothValue;
	if (smoothModule) {
		Param* param = &smoothModule->params[smoothParamId];
		float value = param->value;
		// Decay rate is 1 graphics frame
		const float smoothLambda = 60.f;
		float newValue = value + (smoothValue - value) * smoothLambda * internal->sampleTime;
		if (value == newValue) {
			// Snap to actual smooth value if the value doesn't change enough (due to the granularity of floats)
			param->setValue(smoothValue);
			internal->smoothModule = NULL;
			internal->smoothParamId = 0;
		}
		else {
			param->value = newValue;
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
	Engine_stepModules(that, 0);
	internal->workerBarrier.wait();

	internal->frame++;
}


static void Engine_updateExpander(Engine* that, Module::Expander* expander) {
	if (expander->moduleId >= 0) {
		if (!expander->module || expander->module->id != expander->moduleId) {
			expander->module = that->getModule(expander->moduleId);
		}
	}
	else {
		if (expander->module) {
			expander->module = NULL;
		}
	}
}


static void Engine_relaunchWorkers(Engine* that, int threadCount, bool realTime) {
	Engine::Internal* internal = that->internal;

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
	internal->realTime = realTime;

	// Set barrier counts
	internal->engineBarrier.total = threadCount;
	internal->workerBarrier.total = threadCount;

	// Configure main thread
	system::setThreadRealTime(realTime);

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


Engine::Engine() {
	internal = new Internal;

	internal->sampleRate = 44100.f;
	internal->sampleTime = 1 / internal->sampleRate;
}


Engine::~Engine() {
	Engine_relaunchWorkers(this, 0, false);
	clear();

	// Make sure there are no cables or modules in the rack on destruction.
	// If this happens, a module must have failed to remove itself before the RackWidget was destroyed.
	assert(internal->cables.empty());
	assert(internal->modules.empty());
	assert(internal->paramHandles.empty());
	assert(internal->paramHandleCache.empty());

	delete internal;
}


void Engine::clear() {
	std::lock_guard<std::recursive_mutex> lock(internal->mutex);
	// Copy lists because we'll be removing while iterating
	std::set<ParamHandle*> paramHandles = internal->paramHandles;
	for (ParamHandle* paramHandle : paramHandles) {
		removeParamHandle(paramHandle);
	}
	std::vector<Cable*> cables = internal->cables;
	for (Cable* cable : cables) {
		removeCable(cable);
	}
	std::vector<Module*> modules = internal->modules;
	for (Module* module : modules) {
		removeModule(module);
	}
	// Reset engine state
	internal->nextModuleId = 0;
	internal->nextCableId = 0;
}


void Engine::step(int frames) {
	initMXCSR();

	// Set sample rate
	if (internal->sampleRate != settings::sampleRate) {
		internal->sampleRate = settings::sampleRate;
		internal->sampleTime = 1 / internal->sampleRate;
		Module::SampleRateChangeEvent e;
		e.sampleRate = internal->sampleRate;
		e.sampleTime = internal->sampleTime;
		for (Module* module : internal->modules) {
			module->onSampleRateChange(e);
		}
	}

	if (!internal->paused) {
		// Launch workers
		if (internal->threadCount != settings::threadCount || internal->realTime != settings::realTime) {
			Engine_relaunchWorkers(this, settings::threadCount, settings::realTime);
		}

		std::lock_guard<std::recursive_mutex> lock(internal->mutex);

		// Update expander pointers
		for (Module* module : internal->modules) {
			Engine_updateExpander(this, &module->leftExpander);
			Engine_updateExpander(this, &module->rightExpander);
		}

		// Step modules
		for (int i = 0; i < frames; i++) {
			Engine_step(this);
		}
	}
	else {
		// Stop workers while paused
		if (internal->threadCount != 1) {
			Engine_relaunchWorkers(this, 1, settings::realTime);
		}
	}

	yieldWorkers();
}


void Engine::setPrimaryModule(Module* module) {
	std::lock_guard<std::recursive_mutex> lock(internal->mutex);
	internal->primaryModule = module;
}


Module* Engine::getPrimaryModule() {
	return internal->primaryModule;
}


void Engine::setPaused(bool paused) {
	std::lock_guard<std::recursive_mutex> lock(internal->mutex);
	internal->paused = paused;
}


bool Engine::isPaused() {
	// No lock
	return internal->paused;
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


uint64_t Engine::getFrame() {
	return internal->frame;
}


void Engine::addModule(Module* module) {
	assert(module);
	std::lock_guard<std::recursive_mutex> lock(internal->mutex);
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
		for (Module* m : internal->modules) {
			assert(module->id != m->id);
		}
		if (module->id >= internal->nextModuleId) {
			internal->nextModuleId = module->id + 1;
		}
	}
	// Add module
	internal->modules.push_back(module);
	// Trigger Add event
	Module::AddEvent eAdd;
	module->onAdd(eAdd);
	// Update ParamHandles' module pointers
	for (ParamHandle* paramHandle : internal->paramHandles) {
		if (paramHandle->moduleId == module->id)
			paramHandle->module = module;
	}
	DEBUG("Added module %d to engine", module->id);
}


void Engine::removeModule(Module* module) {
	assert(module);
	std::lock_guard<std::recursive_mutex> lock(internal->mutex);
	// Check that the module actually exists
	auto it = std::find(internal->modules.begin(), internal->modules.end(), module);
	assert(it != internal->modules.end());
	// If a param is being smoothed on this module, stop smoothing it immediately
	if (module == internal->smoothModule) {
		internal->smoothModule = NULL;
	}
	// Check that all cables are disconnected
	for (Cable* cable : internal->cables) {
		assert(cable->outputModule != module);
		assert(cable->inputModule != module);
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
	internal->modules.erase(it);
	DEBUG("Removed module %d to engine", module->id);
}


Module* Engine::getModule(int moduleId) {
	std::lock_guard<std::recursive_mutex> lock(internal->mutex);
	// Find module
	for (Module* module : internal->modules) {
		if (module->id == moduleId)
			return module;
	}
	return NULL;
}


void Engine::resetModule(Module* module) {
	assert(module);
	std::lock_guard<std::recursive_mutex> lock(internal->mutex);

	Module::ResetEvent eReset;
	module->onReset(eReset);
}


void Engine::randomizeModule(Module* module) {
	assert(module);
	std::lock_guard<std::recursive_mutex> lock(internal->mutex);

	Module::RandomizeEvent eRandomize;
	module->onRandomize(eRandomize);
}


void Engine::disableModule(Module* module, bool disabled) {
	assert(module);
	std::lock_guard<std::recursive_mutex> lock(internal->mutex);
	if (module->disabled == disabled)
		return;
	// Clear outputs and set to 1 channel
	for (Output& output : module->outputs) {
		// This zeros all voltages, but the channel is set to 1 if connected
		output.setChannels(0);
	}
	module->disabled = disabled;
	// Trigger event
	if (disabled) {
		Module::DisableEvent eDisable;
		module->onDisable(eDisable);
	}
	else {
		Module::EnableEvent eEnable;
		module->onEnable(eEnable);
	}
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
		for (Output& output : module->outputs) {
			disconnectedPorts.insert(&output);
		}
		for (Input& input : module->inputs) {
			disconnectedPorts.insert(&input);
		}
	}
	for (Cable* cable : that->internal->cables) {
		// Connect output
		Output& output = cable->outputModule->outputs[cable->outputId];
		auto outputIt = disconnectedPorts.find(&output);
		if (outputIt != disconnectedPorts.end())
			disconnectedPorts.erase(outputIt);
		Port_setConnected(&output);
		// Connect input
		Input& input = cable->inputModule->inputs[cable->inputId];
		auto inputIt = disconnectedPorts.find(&input);
		if (inputIt != disconnectedPorts.end())
			disconnectedPorts.erase(inputIt);
		Port_setConnected(&input);
	}
	// Disconnect ports that have no cable
	for (Port* port : disconnectedPorts) {
		Port_setDisconnected(port);
	}
}


void Engine::addCable(Cable* cable) {
	assert(cable);
	std::lock_guard<std::recursive_mutex> lock(internal->mutex);
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
	// Set ID
	if (cable->id < 0) {
		// Automatically assign ID
		cable->id = internal->nextCableId++;
	}
	else {
		// Manual ID
		// Check that the ID is not already taken
		for (Cable* w : internal->cables) {
			assert(cable->id != w->id);
		}
		if (cable->id >= internal->nextCableId) {
			internal->nextCableId = cable->id + 1;
		}
	}
	// Add the cable
	internal->cables.push_back(cable);
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
	DEBUG("Added cable %d to engine", cable->id);
}


void Engine::removeCable(Cable* cable) {
	assert(cable);
	std::lock_guard<std::recursive_mutex> lock(internal->mutex);
	// Check that the cable is already added
	auto it = std::find(internal->cables.begin(), internal->cables.end(), cable);
	assert(it != internal->cables.end());
	// Remove the cable
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
	DEBUG("Removed cable %d to engine", cable->id);
}


Cable* Engine::getCable(int cableId) {
	std::lock_guard<std::recursive_mutex> lock(internal->mutex);
	// Find Cable
	for (Cable* cable : internal->cables) {
		if (cable->id == cableId)
			return cable;
	}
	return NULL;
}

void Engine::setParam(Module* module, int paramId, float value) {
	// TODO Does this need to be thread-safe?
	// If being smoothed, cancel smoothing
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
	return getParam(module, paramId);
}


static void Engine_refreshParamHandleCache(Engine* that) {
	// Clear cache
	that->internal->paramHandleCache.clear();
	// Add active ParamHandles to cache
	for (ParamHandle* paramHandle : that->internal->paramHandles) {
		if (paramHandle->moduleId >= 0) {
			that->internal->paramHandleCache[std::make_tuple(paramHandle->moduleId, paramHandle->paramId)] = paramHandle;
		}
	}
}


void Engine::addParamHandle(ParamHandle* paramHandle) {
	std::lock_guard<std::recursive_mutex> lock(internal->mutex);

	// New ParamHandles must be blank.
	// This means we don't have to refresh the cache.
	assert(paramHandle->moduleId < 0);

	// Check that the ParamHandle is not already added
	auto it = internal->paramHandles.find(paramHandle);
	assert(it == internal->paramHandles.end());

	// Add it
	internal->paramHandles.insert(paramHandle);
}


void Engine::removeParamHandle(ParamHandle* paramHandle) {
	std::lock_guard<std::recursive_mutex> lock(internal->mutex);

	// Check that the ParamHandle is already added
	auto it = internal->paramHandles.find(paramHandle);
	assert(it != internal->paramHandles.end());

	// Remove it
	paramHandle->module = NULL;
	internal->paramHandles.erase(it);
	Engine_refreshParamHandleCache(this);
}


ParamHandle* Engine::getParamHandle(int moduleId, int paramId) {
	// Don't lock because this method is called potentially thousands of times per screen frame.

	auto it = internal->paramHandleCache.find(std::make_tuple(moduleId, paramId));
	if (it == internal->paramHandleCache.end())
		return NULL;
	return it->second;
}


ParamHandle* Engine::getParamHandle(Module* module, int paramId) {
	return getParamHandle(module->id, paramId);
}


void Engine::updateParamHandle(ParamHandle* paramHandle, int moduleId, int paramId, bool overwrite) {
	std::lock_guard<std::recursive_mutex> lock(internal->mutex);

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
	// modules
	json_t* modulesJ = json_object_get(rootJ, "modules");
	if (!modulesJ)
		return;
	size_t moduleIndex;
	json_t* moduleJ;
	json_array_foreach(modulesJ, moduleIndex, moduleJ) {
		try {
			Module* module = moduleFromJson(moduleJ);

			// Before 1.0, the module ID was the index in the "modules" array
			if (APP->patch->isLegacy(2)) {
				module->id = moduleIndex;
			}

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
			// DEBUG("%p %d %p %d", cable->inputModule, cable->inputId, cable->)
			addCable(cable);
		}
		catch (Exception& e) {
			delete cable;
			// Don't log exceptions because missing modules create unnecessary complaining when cables try to connect to them.
		}
	}
}


void EngineWorker::run() {
	system::setThreadName(string::f("Worker %d", id));
	initMXCSR();
	random::init();

	while (true) {
		engine->internal->engineBarrier.wait();
		if (!running)
			return;
		Engine_stepModules(engine, id);
		engine->internal->workerBarrier.wait();
	}
}


Module* moduleFromJson(json_t* moduleJ) {
	// Get slugs
	json_t* pluginSlugJ = json_object_get(moduleJ, "plugin");
	if (!pluginSlugJ)
		throw Exception("\"plugin\" property not found in module JSON");
	json_t* modelSlugJ = json_object_get(moduleJ, "model");
	if (!modelSlugJ)
		throw Exception("\"model\" property not found in module JSON");
	std::string pluginSlug = json_string_value(pluginSlugJ);
	std::string modelSlug = json_string_value(modelSlugJ);

	// Get Model
	plugin::Model* model = plugin::getModel(pluginSlug, modelSlug);
	if (!model)
		throw Exception(string::f("Could not find module \"%s\" of plugin \"%s\"", modelSlug.c_str(), pluginSlug.c_str()));

	// Create Module
	Module* module = model->createModule();
	assert(module);
	module->fromJson(moduleJ);
	return module;
}


} // namespace engine
} // namespace rack

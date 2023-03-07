#include <algorithm>
#include <set>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <tuple>
#if defined ARCH_X64
	#include <pmmintrin.h>
#endif

#include <engine/Engine.hpp>
#include <settings.hpp>
#include <system.hpp>
#include <random.hpp>
#include <context.hpp>
#include <patch.hpp>
#include <plugin.hpp>
#include <mutex.hpp>


namespace rack {
namespace engine {


#if defined ARCH_X64
static void initMXCSR() {
	// Set CPU to flush-to-zero (FTZ) and denormals-are-zero (DAZ) mode
	// https://software.intel.com/en-us/node/682949
	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
	_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
	// Reset other flags
	_MM_SET_ROUNDING_MODE(_MM_ROUND_NEAREST);
}
#endif


/** Barrier based on mutexes.
Not finished or tested, do not use.
*/
struct Barrier {
	int count = 0;
	uint8_t step = 0;
	int threads = 0;

	std::mutex mutex;
	std::condition_variable cv;

	void setThreads(int threads) {
		this->threads = threads;
	}

	void wait() {
		std::unique_lock<std::mutex> lock(mutex);
		uint8_t s = step;
		if (++count >= threads) {
			// We're the last thread. Reset next phase.
			count = 0;
			// Allow other threads to exit wait()
			step++;
			cv.notify_all();
			return;
		}

		cv.wait(lock, [&] {
			return step != s;
		});
	}
};


/** 2-phase barrier based on spin-locking.
*/
struct SpinBarrier {
	std::atomic<int> count{0};
	std::atomic<uint8_t> step{0};
	int threads = 0;

	/** Must be called when no threads are calling wait().
	*/
	void setThreads(int threads) {
		this->threads = threads;
	}

	void wait() {
		uint8_t s = step;
		if (count.fetch_add(1, std::memory_order_acquire) + 1 >= threads) {
			// We're the last thread. Reset next phase.
			count = 0;
			// Allow other threads to exit wait()
			step++;
			return;
		}

		// Spin until the last thread begins waiting
		while (true) {
			if (step.load(std::memory_order_relaxed) != s)
				return;
#if defined ARCH_X64
			__builtin_ia32_pause();
#endif
		}
	}
};


/** Barrier that spin-locks until yield() is called, and then all threads switch to a mutex.
yield() should be called if it is likely that all threads will block for a while and continuing to spin-lock is unnecessary.
Saves CPU power after yield is called.
*/
struct HybridBarrier {
	std::atomic<int> count{0};
	std::atomic<uint8_t> step{0};
	int threads = 0;

	std::atomic<bool> yielded{false};
	std::mutex mutex;
	std::condition_variable cv;

	void setThreads(int threads) {
		this->threads = threads;
	}

	void yield() {
		yielded = true;
	}

	void wait() {
		uint8_t s = step;
		if (count.fetch_add(1, std::memory_order_acquire) + 1 >= threads) {
			// We're the last thread. Reset next phase.
			count = 0;
			bool wasYielded = yielded;
			yielded = false;
			// Allow other threads to exit wait()
			step++;
			if (wasYielded) {
				std::unique_lock<std::mutex> lock(mutex);
				cv.notify_all();
			}
			return;
		}

		// Spin until the last thread begins waiting
		while (!yielded.load(std::memory_order_relaxed)) {
			if (step.load(std::memory_order_relaxed) != s)
				return;
#if defined ARCH_X64
			__builtin_ia32_pause();
#endif
		}

		// Wait on mutex CV
		std::unique_lock<std::mutex> lock(mutex);
		cv.wait(lock, [&] {
			return step != s;
		});
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
	Module* masterModule = NULL;

	// moduleId
	std::map<int64_t, Module*> modulesCache;
	// cableId
	std::map<int64_t, Cable*> cablesCache;
	// (moduleId, paramId)
	std::map<std::tuple<int64_t, int>, ParamHandle*> paramHandlesCache;

	float sampleRate = 0.f;
	float sampleTime = 0.f;
	int64_t frame = 0;
	int64_t block = 0;
	int64_t blockFrame = 0;
	double blockTime = 0.0;
	int blockFrames = 0;

	// Meter
	int meterCount = 0;
	double meterTotal = 0.0;
	double meterMax = 0.0;
	double meterLastTime = -INFINITY;
	double meterLastAverage = 0.0;
	double meterLastMax = 0.0;

	// Parameter smoothing
	Module* smoothModule = NULL;
	int smoothParamId = 0;
	float smoothValue = 0.f;

	/** Mutex that guards the Engine state, such as settings, Modules, and Cables.
	Writers lock when mutating the engine's state or stepping the block.
	Readers lock when using the engine's state.
	*/
	SharedMutex mutex;
	/** Mutex that guards stepBlock() so it's not called simultaneously.
	*/
	std::mutex blockMutex;

	int threadCount = 0;
	std::vector<EngineWorker> workers;
	HybridBarrier engineBarrier;
	HybridBarrier workerBarrier;
	std::atomic<int> workerModuleIndex;
	// For worker threads
	Context* context;

	bool fallbackRunning = false;
	std::thread fallbackThread;
	std::mutex fallbackMutex;
	std::condition_variable fallbackCv;
};


static void Engine_updateExpander_NoLock(Engine* that, Module* module, bool side) {
	Module::Expander& expander = side ? module->rightExpander : module->leftExpander;
	Module* oldExpanderModule = expander.module;

	if (expander.moduleId >= 0) {
		if (!expander.module || expander.module->id != expander.moduleId) {
			expander.module = that->getModule_NoLock(expander.moduleId);
		}
	}
	else {
		if (expander.module) {
			expander.module = NULL;
		}
	}

	if (expander.module != oldExpanderModule) {
		// Dispatch ExpanderChangeEvent
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
	internal->engineBarrier.setThreads(threadCount);
	internal->workerBarrier.setThreads(threadCount);

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


static void Engine_stepWorker(Engine* that, int threadId) {
	Engine::Internal* internal = that->internal;

	// int threadCount = internal->threadCount;
	int modulesLen = internal->modules.size();

	// Build ProcessArgs
	Module::ProcessArgs processArgs;
	processArgs.sampleRate = internal->sampleRate;
	processArgs.sampleTime = internal->sampleTime;
	processArgs.frame = internal->frame;

	// Step each module
	while (true) {
		// Choose next module
		// First-come-first serve module-to-thread allocation algorithm
		int i = internal->workerModuleIndex++;
		if (i >= modulesLen)
			break;

		Module* module = internal->modules[i];
		module->doProcess(processArgs);
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
	Engine_stepWorker(that, 0);
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
	setSuggestedSampleRate(0.f);
}


Engine::~Engine() {
	// Stop fallback thread if running
	{
		std::lock_guard<std::mutex> lock(internal->fallbackMutex);
		internal->fallbackRunning = false;
		internal->fallbackCv.notify_all();
	}
	if (internal->fallbackThread.joinable())
		internal->fallbackThread.join();

	// Shut down workers
	Engine_relaunchWorkers(this, 0);

	// Clear modules, cables, etc
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
	std::lock_guard<SharedMutex> lock(internal->mutex);
	clear_NoLock();
}


void Engine::clear_NoLock() {
	// Copy lists because we'll be removing while iterating
	std::set<ParamHandle*> paramHandles = internal->paramHandles;
	for (ParamHandle* paramHandle : paramHandles) {
		removeParamHandle_NoLock(paramHandle);
		// Don't delete paramHandle because they're normally owned by Module subclasses
	}
	std::vector<Cable*> cables = internal->cables;
	for (Cable* cable : cables) {
		removeCable_NoLock(cable);
		delete cable;
	}
	std::vector<Module*> modules = internal->modules;
	for (Module* module : modules) {
		removeModule_NoLock(module);
		delete module;
	}
}


void Engine::stepBlock(int frames) {
	// Start timer before locking
	double startTime = system::getTime();

	std::lock_guard<std::mutex> stepLock(internal->blockMutex);
	SharedLock<SharedMutex> lock(internal->mutex);
	// Configure thread
#if defined ARCH_X64
	uint32_t csr = _mm_getcsr();
	initMXCSR();
#endif
	random::init();

	internal->blockFrame = internal->frame;
	internal->blockTime = system::getTime();
	internal->blockFrames = frames;

	// Update expander pointers
	for (Module* module : internal->modules) {
		Engine_updateExpander_NoLock(this, module, false);
		Engine_updateExpander_NoLock(this, module, true);
	}

	// Launch workers
	Engine_relaunchWorkers(this, settings::threadCount);

	// Step individual frames
	for (int i = 0; i < frames; i++) {
		Engine_stepFrame(this);
	}

	yieldWorkers();

	internal->block++;

	// Stop timer
	double endTime = system::getTime();
	double meter = (endTime - startTime) / (frames * internal->sampleTime);
	internal->meterTotal += meter;
	internal->meterMax = std::fmax(internal->meterMax, meter);
	internal->meterCount++;

	// Update meter values
	const double meterUpdateDuration = 1.0;
	if (startTime - internal->meterLastTime >= meterUpdateDuration) {
		internal->meterLastAverage = internal->meterTotal / internal->meterCount;
		internal->meterLastMax = internal->meterMax;
		internal->meterLastTime = startTime;
		internal->meterCount = 0;
		internal->meterTotal = 0.0;
		internal->meterMax = 0.0;
	}

#if defined ARCH_X64
	// Reset MXCSR back to original value
	_mm_setcsr(csr);
#endif
}


void Engine::setMasterModule(Module* module) {
	if (module == internal->masterModule)
		return;
	std::lock_guard<SharedMutex> lock(internal->mutex);
	setMasterModule_NoLock(module);
}


void Engine::setMasterModule_NoLock(Module* module) {
	if (module == internal->masterModule)
		return;

	if (internal->masterModule) {
		// Dispatch UnsetMasterEvent
		Module::UnsetMasterEvent e;
		internal->masterModule->onUnsetMaster(e);
	}

	internal->masterModule = module;

	if (internal->masterModule) {
		// Dispatch SetMasterEvent
		Module::SetMasterEvent e;
		internal->masterModule->onSetMaster(e);
	}

	// Wake up fallback thread if master module was unset
	if (!internal->masterModule) {
		internal->fallbackCv.notify_all();
	}
}


Module* Engine::getMasterModule() {
	return internal->masterModule;
}


float Engine::getSampleRate() {
	return internal->sampleRate;
}


void Engine::setSampleRate(float sampleRate) {
	if (sampleRate == internal->sampleRate)
		return;
	std::lock_guard<SharedMutex> lock(internal->mutex);

	internal->sampleRate = sampleRate;
	internal->sampleTime = 1.f / sampleRate;
	// Dispatch SampleRateChangeEvent
	Module::SampleRateChangeEvent e;
	e.sampleRate = internal->sampleRate;
	e.sampleTime = internal->sampleTime;
	for (Module* module : internal->modules) {
		module->onSampleRateChange(e);
	}
}


void Engine::setSuggestedSampleRate(float suggestedSampleRate) {
	if (settings::sampleRate > 0) {
		setSampleRate(settings::sampleRate);
	}
	else if (suggestedSampleRate > 0) {
		setSampleRate(suggestedSampleRate);
	}
	else {
		// Fallback sample rate
		setSampleRate(44100.f);
	}
}


float Engine::getSampleTime() {
	return internal->sampleTime;
}


void Engine::yieldWorkers() {
	internal->workerBarrier.yield();
}


int64_t Engine::getFrame() {
	return internal->frame;
}


int64_t Engine::getBlock() {
	return internal->block;
}


int64_t Engine::getBlockFrame() {
	return internal->blockFrame;
}


double Engine::getBlockTime() {
	return internal->blockTime;
}


int Engine::getBlockFrames() {
	return internal->blockFrames;
}


double Engine::getBlockDuration() {
	return internal->blockFrames * internal->sampleTime;
}


double Engine::getMeterAverage() {
	return internal->meterLastAverage;
}


double Engine::getMeterMax() {
	return internal->meterLastMax;
}


size_t Engine::getNumModules() {
	return internal->modules.size();
}


size_t Engine::getModuleIds(int64_t* moduleIds, size_t len) {
	SharedLock<SharedMutex> lock(internal->mutex);
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
	SharedLock<SharedMutex> lock(internal->mutex);
	std::vector<int64_t> moduleIds;
	moduleIds.reserve(internal->modules.size());
	for (Module* m : internal->modules) {
		moduleIds.push_back(m->id);
	}
	return moduleIds;
}


void Engine::addModule(Module* module) {
	std::lock_guard<SharedMutex> lock(internal->mutex);
	assert(module);
	// Check that the module is not already added
	auto it = std::find(internal->modules.begin(), internal->modules.end(), module);
	assert(it == internal->modules.end());
	// Set ID if unset or collides with an existing ID
	while (module->id < 0 || internal->modulesCache.find(module->id) != internal->modulesCache.end()) {
		// Randomly generate ID
		module->id = random::u64() % (1ull << 53);
	}
	// Add module
	internal->modules.push_back(module);
	internal->modulesCache[module->id] = module;
	// Dispatch AddEvent
	Module::AddEvent eAdd;
	module->onAdd(eAdd);
	// Dispatch SampleRateChangeEvent
	Module::SampleRateChangeEvent eSrc;
	eSrc.sampleRate = internal->sampleRate;
	eSrc.sampleTime = internal->sampleTime;
	module->onSampleRateChange(eSrc);
	// Update ParamHandles' module pointers
	for (ParamHandle* paramHandle : internal->paramHandles) {
		if (paramHandle->moduleId == module->id)
			paramHandle->module = module;
	}
}


void Engine::removeModule(Module* module) {
	std::lock_guard<SharedMutex> lock(internal->mutex);
	removeModule_NoLock(module);
}


void Engine::removeModule_NoLock(Module* module) {
	assert(module);
	// Check that the module actually exists
	auto it = std::find(internal->modules.begin(), internal->modules.end(), module);
	assert(it != internal->modules.end());
	// Dispatch RemoveEvent
	Module::RemoveEvent eRemove;
	module->onRemove(eRemove);
	// Update ParamHandles' module pointers
	for (ParamHandle* paramHandle : internal->paramHandles) {
		if (paramHandle->moduleId == module->id)
			paramHandle->module = NULL;
	}
	// Unset master module
	if (getMasterModule() == module) {
		setMasterModule_NoLock(NULL);
	}
	// If a param is being smoothed on this module, stop smoothing it immediately
	if (module == internal->smoothModule) {
		internal->smoothModule = NULL;
	}
	// Check that all cables are disconnected
	for (Cable* cable : internal->cables) {
		assert(cable->inputModule != module);
		assert(cable->outputModule != module);
	}
	// Update expanders of other modules
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
	// Remove module
	internal->modulesCache.erase(module->id);
	internal->modules.erase(it);
	// Reset expanders
	module->leftExpander.moduleId = -1;
	module->leftExpander.module = NULL;
	module->rightExpander.moduleId = -1;
	module->rightExpander.module = NULL;
}


bool Engine::hasModule(Module* module) {
	SharedLock<SharedMutex> lock(internal->mutex);
	// TODO Performance could be improved by searching modulesCache, but more testing would be needed to make sure it's always valid.
	auto it = std::find(internal->modules.begin(), internal->modules.end(), module);
	return it != internal->modules.end();
}


Module* Engine::getModule(int64_t moduleId) {
	SharedLock<SharedMutex> lock(internal->mutex);
	return getModule_NoLock(moduleId);
}


Module* Engine::getModule_NoLock(int64_t moduleId) {
	auto it = internal->modulesCache.find(moduleId);
	if (it == internal->modulesCache.end())
		return NULL;
	return it->second;
}


void Engine::resetModule(Module* module) {
	std::lock_guard<SharedMutex> lock(internal->mutex);
	assert(module);

	Module::ResetEvent eReset;
	module->onReset(eReset);
}


void Engine::randomizeModule(Module* module) {
	std::lock_guard<SharedMutex> lock(internal->mutex);
	assert(module);

	Module::RandomizeEvent eRandomize;
	module->onRandomize(eRandomize);
}


void Engine::bypassModule(Module* module, bool bypassed) {
	assert(module);
	if (module->isBypassed() == bypassed)
		return;

	std::lock_guard<SharedMutex> lock(internal->mutex);

	// Clear outputs and set to 1 channel
	for (Output& output : module->outputs) {
		// This zeros all voltages, but the channel is set to 1 if connected
		output.setChannels(0);
	}
	// Set bypassed state
	module->setBypassed(bypassed);
	if (bypassed) {
		// Dispatch BypassEvent
		Module::BypassEvent eBypass;
		module->onBypass(eBypass);
	}
	else {
		// Dispatch UnBypassEvent
		Module::UnBypassEvent eUnBypass;
		module->onUnBypass(eUnBypass);
	}
}


json_t* Engine::moduleToJson(Module* module) {
	SharedLock<SharedMutex> lock(internal->mutex);
	return module->toJson();
}


void Engine::moduleFromJson(Module* module, json_t* rootJ) {
	std::lock_guard<SharedMutex> lock(internal->mutex);
	module->fromJson(rootJ);
}


void Engine::prepareSaveModule(Module* module) {
	SharedLock<SharedMutex> lock(internal->mutex);
	Module::SaveEvent e;
	module->onSave(e);
}


void Engine::prepareSave() {
	SharedLock<SharedMutex> lock(internal->mutex);
	for (Module* module : internal->modules) {
		Module::SaveEvent e;
		module->onSave(e);
	}
}


size_t Engine::getNumCables() {
	return internal->cables.size();
}


size_t Engine::getCableIds(int64_t* cableIds, size_t len) {
	SharedLock<SharedMutex> lock(internal->mutex);
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
	SharedLock<SharedMutex> lock(internal->mutex);
	std::vector<int64_t> cableIds;
	cableIds.reserve(internal->cables.size());
	for (Cable* c : internal->cables) {
		cableIds.push_back(c->id);
	}
	return cableIds;
}


void Engine::addCable(Cable* cable) {
	std::lock_guard<SharedMutex> lock(internal->mutex);
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
		cable->id = random::u64() % (1ull << 53);
	}
	// Add the cable
	internal->cables.push_back(cable);
	internal->cablesCache[cable->id] = cable;
	Engine_updateConnected(this);
	// Dispatch input port event
	{
		Module::PortChangeEvent e;
		e.connecting = true;
		e.type = Port::INPUT;
		e.portId = cable->inputId;
		cable->inputModule->onPortChange(e);
	}
	// Dispatch output port event if its state went from disconnected to connected.
	if (!outputWasConnected) {
		Module::PortChangeEvent e;
		e.connecting = true;
		e.type = Port::OUTPUT;
		e.portId = cable->outputId;
		cable->outputModule->onPortChange(e);
	}
}


void Engine::removeCable(Cable* cable) {
	std::lock_guard<SharedMutex> lock(internal->mutex);
	removeCable_NoLock(cable);
}


void Engine::removeCable_NoLock(Cable* cable) {
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
	// Dispatch input port event
	{
		Module::PortChangeEvent e;
		e.connecting = false;
		e.type = Port::INPUT;
		e.portId = cable->inputId;
		cable->inputModule->onPortChange(e);
	}
	// Dispatch output port event if its state went from connected to disconnected.
	if (!outputIsConnected) {
		Module::PortChangeEvent e;
		e.connecting = false;
		e.type = Port::OUTPUT;
		e.portId = cable->outputId;
		cable->outputModule->onPortChange(e);
	}
}


bool Engine::hasCable(Cable* cable) {
	SharedLock<SharedMutex> lock(internal->mutex);
	// TODO Performance could be improved by searching cablesCache, but more testing would be needed to make sure it's always valid.
	auto it = std::find(internal->cables.begin(), internal->cables.end(), cable);
	return it != internal->cables.end();
}


Cable* Engine::getCable(int64_t cableId) {
	SharedLock<SharedMutex> lock(internal->mutex);
	auto it = internal->cablesCache.find(cableId);
	if (it == internal->cablesCache.end())
		return NULL;
	return it->second;
}


void Engine::setParamValue(Module* module, int paramId, float value) {
	// If param is being smoothed, cancel smoothing.
	if (internal->smoothModule == module && internal->smoothParamId == paramId) {
		internal->smoothModule = NULL;
		internal->smoothParamId = 0;
	}
	module->params[paramId].setValue(value);
}


float Engine::getParamValue(Module* module, int paramId) {
	return module->params[paramId].getValue();
}


void Engine::setParamSmoothValue(Module* module, int paramId, float value) {
	// If another param is being smoothed, jump value
	if (internal->smoothModule && !(internal->smoothModule == module && internal->smoothParamId == paramId)) {
		internal->smoothModule->params[internal->smoothParamId].setValue(internal->smoothValue);
	}
	internal->smoothParamId = paramId;
	internal->smoothValue = value;
	// Set this last so the above values are valid as soon as it is set
	internal->smoothModule = module;
}


float Engine::getParamSmoothValue(Module* module, int paramId) {
	if (internal->smoothModule == module && internal->smoothParamId == paramId)
		return internal->smoothValue;
	return module->params[paramId].getValue();
}


void Engine::addParamHandle(ParamHandle* paramHandle) {
	std::lock_guard<SharedMutex> lock(internal->mutex);
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
	std::lock_guard<SharedMutex> lock(internal->mutex);
	removeParamHandle_NoLock(paramHandle);
}


void Engine::removeParamHandle_NoLock(ParamHandle* paramHandle) {
	// Check that the ParamHandle is already added
	auto it = internal->paramHandles.find(paramHandle);
	assert(it != internal->paramHandles.end());

	// Remove it
	paramHandle->module = NULL;
	internal->paramHandles.erase(it);
	Engine_refreshParamHandleCache(this);
}


ParamHandle* Engine::getParamHandle(int64_t moduleId, int paramId) {
	SharedLock<SharedMutex> lock(internal->mutex);
	return getParamHandle_NoLock(moduleId, paramId);
}


ParamHandle* Engine::getParamHandle_NoLock(int64_t moduleId, int paramId) {
	auto it = internal->paramHandlesCache.find(std::make_tuple(moduleId, paramId));
	if (it == internal->paramHandlesCache.end())
		return NULL;
	return it->second;
}


ParamHandle* Engine::getParamHandle(Module* module, int paramId) {
	return getParamHandle(module->id, paramId);
}


void Engine::updateParamHandle(ParamHandle* paramHandle, int64_t moduleId, int paramId, bool overwrite) {
	std::lock_guard<SharedMutex> lock(internal->mutex);
	updateParamHandle_NoLock(paramHandle, moduleId, paramId, overwrite);
}


void Engine::updateParamHandle_NoLock(ParamHandle* paramHandle, int64_t moduleId, int paramId, bool overwrite) {
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
		ParamHandle* oldParamHandle = getParamHandle_NoLock(moduleId, paramId);
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
		paramHandle->module = getModule_NoLock(paramHandle->moduleId);
	}

	Engine_refreshParamHandleCache(this);
}


json_t* Engine::toJson() {
	SharedLock<SharedMutex> lock(internal->mutex);
	json_t* rootJ = json_object();

	// modules
	json_t* modulesJ = json_array();
	for (Module* module : internal->modules) {
		json_t* moduleJ = module->toJson();
		json_array_append_new(modulesJ, moduleJ);
	}
	json_object_set_new(rootJ, "modules", modulesJ);

	// cables
	json_t* cablesJ = json_array();
	for (Cable* cable : internal->cables) {
		json_t* cableJ = cable->toJson();
		json_array_append_new(cablesJ, cableJ);
	}
	json_object_set_new(rootJ, "cables", cablesJ);

	// masterModule
	if (internal->masterModule) {
		json_object_set_new(rootJ, "masterModuleId", json_integer(internal->masterModule->id));
	}

	return rootJ;
}


void Engine::fromJson(json_t* rootJ) {
	// Don't write-lock the entire method because most of it doesn't need it.

	// Write-locks
	clear();
	// modules
	json_t* modulesJ = json_object_get(rootJ, "modules");
	if (!modulesJ)
		return;
	size_t moduleIndex;
	json_t* moduleJ;
	json_array_foreach(modulesJ, moduleIndex, moduleJ) {
		// Get model
		plugin::Model* model;
		try {
			model = plugin::modelFromJson(moduleJ);
		}
		catch (Exception& e) {
			WARN("Cannot load model: %s", e.what());
			APP->patch->log(e.what());
			continue;
		}

		// Create module
		INFO("Creating module %s", model->getFullName().c_str());
		Module* module = model->createModule();
		assert(module);

		try {
			// This doesn't need a lock because the Module is not added to the Engine yet.
			module->fromJson(moduleJ);

			// Before 1.0, the module ID was the index in the "modules" array
			if (module->id < 0) {
				module->id = moduleIndex;
			}

			// Write-locks
			addModule(module);
		}
		catch (Exception& e) {
			WARN("Cannot load module: %s", e.what());
			APP->patch->log(e.what());
			delete module;
			continue;
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

			// Before 1.0, the cable ID was the index in the "cables" array
			if (cable->id < 0) {
				cable->id = cableIndex;
			}

			// Write-locks
			addCable(cable);
		}
		catch (Exception& e) {
			WARN("Cannot load cable: %s", e.what());
			delete cable;
			// Don't log exceptions because missing modules create unnecessary complaining when cables try to connect to them.
			continue;
		}
	}

	// masterModule
	json_t* masterModuleIdJ = json_object_get(rootJ, "masterModuleId");
	if (masterModuleIdJ) {
		Module* masterModule = getModule(json_integer_value(masterModuleIdJ));
		setMasterModule(masterModule);
	}
}


void EngineWorker::run() {
	// Configure thread
	contextSet(engine->internal->context);
	system::setThreadName(string::f("Worker %d", id));
#if defined ARCH_X64
	initMXCSR();
#endif
	random::init();

	while (true) {
		engine->internal->engineBarrier.wait();
		if (!running)
			return;
		Engine_stepWorker(engine, id);
		engine->internal->workerBarrier.wait();
	}
}


static void Engine_fallbackRun(Engine* that) {
	system::setThreadName("Engine fallback");
	contextSet(that->internal->context);

	while (that->internal->fallbackRunning) {
		if (!that->getMasterModule()) {
			// Step blocks and wait
			double start = system::getTime();
			int frames = std::floor(that->getSampleRate() / 60);
			that->stepBlock(frames);
			double end = system::getTime();

			double duration = frames * that->getSampleTime() - (end - start);
			if (duration > 0.0) {
				std::this_thread::sleep_for(std::chrono::duration<double>(duration));
			}
		}
		else {
			// Wait for master module to be unset, or for the request to stop running
			std::unique_lock<std::mutex> lock(that->internal->fallbackMutex);
			that->internal->fallbackCv.wait(lock, [&]() {
				return !that->internal->fallbackRunning || !that->getMasterModule();
			});
		}
	}
}


void Engine::startFallbackThread() {
	if (internal->fallbackThread.joinable())
		return;

	internal->fallbackRunning = true;
	internal->fallbackThread = std::thread(Engine_fallbackRun, this);
}


} // namespace engine
} // namespace rack

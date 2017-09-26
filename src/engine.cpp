#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include <xmmintrin.h>

#include "engine.hpp"
#include "util.hpp"


namespace rack {

float gSampleRate;


static bool running = false;

static std::mutex mutex;
static std::thread thread;
static VIPMutex vipMutex;

static std::vector<Module*> modules;
// Merely used for keeping track of which module inputs point to which module outputs, to prevent pointer mistakes and make the rack API more rigorous
static std::vector<Wire*> wires;

// Parameter interpolation
static Module *smoothModule = NULL;
static int smoothParamId;
static float smoothValue;


void engineInit() {
	gSampleRate = 44100.0;
}

void engineDestroy() {
	// Make sure there are no wires or modules in the rack on destruction. This suggests that a module failed to remove itself before the GUI was destroyed.
	assert(wires.empty());
	assert(modules.empty());
}

static void engineStep() {
	// Param interpolation
	if (smoothModule) {
		float value = smoothModule->params[smoothParamId];
		const float lambda = 60.0; // decay rate is 1 graphics frame
		const float snap = 0.0001;
		float delta = smoothValue - value;
		if (fabsf(delta) < snap) {
			smoothModule->params[smoothParamId] = smoothValue;
			smoothModule = NULL;
		}
		else {
			value += delta * lambda / gSampleRate;
			smoothModule->params[smoothParamId] = value;
		}
	}
	// Step modules
	for (size_t i = 0; i < modules.size(); i++) {
		Module *module = modules[i];
		module->step();
	}
	// Step cables by moving their output values to inputs
	for (Wire *wire : wires) {
		wire->inputValue = wire->outputValue;
		wire->outputValue = 0.0;
	}
}

static void engineRun() {
	// Set CPU to denormals-are-zero mode
	// http://carlh.net/plugins/denormals.php
	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);

	// Every time the engine waits and locks a mutex, it steps this many frames
	const int mutexSteps = 64;
	// Time in seconds that the engine is rushing ahead of the estimated clock time
	float ahead = 0.0;
	auto lastTime = std::chrono::high_resolution_clock::now();

	while (running) {
		vipMutex.wait();

		{
			std::lock_guard<std::mutex> lock(mutex);
			for (int i = 0; i < mutexSteps; i++) {
				engineStep();
			}
		}

		float stepTime = mutexSteps / gSampleRate;
		ahead += stepTime;
		auto currTime = std::chrono::high_resolution_clock::now();
		const float aheadFactor = 2.0;
		ahead -= aheadFactor * std::chrono::duration<float>(currTime - lastTime).count();
		lastTime = currTime;
		ahead = fmaxf(ahead, 0.0);

		// Avoid pegging the CPU at 100% when there are no "blocking" modules like AudioInterface, but still step audio at a reasonable rate
		// The number of steps to wait before possibly sleeping
		const float aheadMax = 1.0; // seconds
		if (ahead > aheadMax) {
			std::this_thread::sleep_for(std::chrono::duration<float>(stepTime));
		}
	}
}

void engineStart() {
	running = true;
	thread = std::thread(engineRun);
}

void engineStop() {
	running = false;
	thread.join();
}

void engineAddModule(Module *module) {
	assert(module);
	VIPLock vipLock(vipMutex);
	std::lock_guard<std::mutex> lock(mutex);
	// Check that the module is not already added
	auto it = std::find(modules.begin(), modules.end(), module);
	assert(it == modules.end());
	modules.push_back(module);
}

void engineRemoveModule(Module *module) {
	assert(module);
	VIPLock vipLock(vipMutex);
	std::lock_guard<std::mutex> lock(mutex);
	// If a param is being smoothed on this module, remove it immediately
	if (module == smoothModule) {
		smoothModule = NULL;
	}
	// Check that all wires are disconnected
	for (Wire *wire : wires) {
		assert(wire->outputModule != module);
		assert(wire->inputModule != module);
	}
	auto it = std::find(modules.begin(), modules.end(), module);
	assert(it != modules.end());
	modules.erase(it);
}

void engineAddWire(Wire *wire) {
	assert(wire);
	VIPLock vipLock(vipMutex);
	std::lock_guard<std::mutex> lock(mutex);
	// Check that the wire is not already added
	auto it = std::find(wires.begin(), wires.end(), wire);
	assert(it == wires.end());
	assert(wire->outputModule);
	assert(wire->inputModule);
	// Check that the inputs/outputs are not already used by another cable
	for (Wire *wire2 : wires) {
		assert(wire2 != wire);
		assert(!(wire2->outputModule == wire->outputModule && wire2->outputId == wire->outputId));
		assert(!(wire2->inputModule == wire->inputModule && wire2->inputId == wire->inputId));
	}
	// Add the wire
	wires.push_back(wire);
	// Connect the wire to inputModule
	wire->inputModule->inputs[wire->inputId] = &wire->inputValue;
	wire->outputModule->outputs[wire->outputId] = &wire->outputValue;
}

void engineRemoveWire(Wire *wire) {
	assert(wire);
	VIPLock vipLock(vipMutex);
	std::lock_guard<std::mutex> lock(mutex);
	// Disconnect wire from inputModule
	wire->inputModule->inputs[wire->inputId] = NULL;
	wire->outputModule->outputs[wire->outputId] = NULL;

	// Remove the wire
	auto it = std::find(wires.begin(), wires.end(), wire);
	assert(it != wires.end());
	wires.erase(it);
}

void engineSetParamSmooth(Module *module, int paramId, float value) {
	VIPLock vipLock(vipMutex);
	std::lock_guard<std::mutex> lock(mutex);
	// Since only one param can be smoothed at a time, if another param is currently being smoothed, skip to its final state
	if (smoothModule && !(smoothModule == module && smoothParamId == paramId)) {
		smoothModule->params[smoothParamId] = smoothValue;
	}
	smoothModule = module;
	smoothParamId = paramId;
	smoothValue = value;
}


} // namespace rack

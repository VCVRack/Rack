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

float sampleRate;
bool gPaused = false;


static bool running = false;

static std::mutex mutex;
static std::thread thread;
static VIPMutex vipMutex;

static std::vector<Module*> modules;
static std::vector<Wire*> wires;

// Parameter interpolation
static Module *smoothModule = NULL;
static int smoothParamId;
static float smoothValue;


void Wire::step() {
	float value = outputModule->outputs[outputId].value;
	inputModule->inputs[inputId].value = value;
}

void engineInit() {
	engineSetSampleRate(44100.0);
}

void engineDestroy() {
	// Make sure there are no wires or modules in the rack on destruction. This suggests that a module failed to remove itself before the GUI was destroyed.
	assert(wires.empty());
	assert(modules.empty());
}

static void engineStep() {
	// Param interpolation
	if (smoothModule) {
		float value = smoothModule->params[smoothParamId].value;
		const float lambda = 60.0; // decay rate is 1 graphics frame
		const float snap = 0.0001;
		float delta = smoothValue - value;
		if (fabsf(delta) < snap) {
			smoothModule->params[smoothParamId].value = smoothValue;
			smoothModule = NULL;
		}
		else {
			value += delta * lambda / sampleRate;
			smoothModule->params[smoothParamId].value = value;
		}
	}

	// Step modules
	for (Module *module : modules) {
		module->step();
	}

	// Step cables by moving their output values to inputs
	for (Wire *wire : wires) {
		wire->step();
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

		if (!gPaused) {
			std::lock_guard<std::mutex> lock(mutex);
			for (int i = 0; i < mutexSteps; i++) {
				engineStep();
			}
		}

		float stepTime = mutexSteps / sampleRate;
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
	// If a param is being smoothed on this module, stop smoothing it immediately
	if (module == smoothModule) {
		smoothModule = NULL;
	}
	// Check that all wires are disconnected
	for (Wire *wire : wires) {
		assert(wire->outputModule != module);
		assert(wire->inputModule != module);
	}
	// Check that the module actually exists
	auto it = std::find(modules.begin(), modules.end(), module);
	assert(it != modules.end());
	// Remove it
	modules.erase(it);
}

static void updateActive() {
	// Set everything to inactive
	for (Module *module : modules) {
		for (Input &input : module->inputs) {
			input.active = false;
		}
		for (Output &output : module->outputs) {
			output.active = false;
		}
	}
	// Set inputs/outputs to active
	for (Wire *wire : wires) {
		wire->outputModule->outputs[wire->outputId].active = true;
		wire->inputModule->inputs[wire->inputId].active = true;
	}
}

void engineAddWire(Wire *wire) {
	assert(wire);
	VIPLock vipLock(vipMutex);
	std::lock_guard<std::mutex> lock(mutex);
	// Check wire properties
	assert(wire->outputModule);
	assert(wire->inputModule);
	// Check that the wire is not already added, and that the input is not already used by another cable
	for (Wire *wire2 : wires) {
		assert(wire2 != wire);
		assert(!(wire2->inputModule == wire->inputModule && wire2->inputId == wire->inputId));
	}
	// Add the wire
	wires.push_back(wire);
	updateActive();
}

void engineRemoveWire(Wire *wire) {
	assert(wire);
	VIPLock vipLock(vipMutex);
	std::lock_guard<std::mutex> lock(mutex);
	// Check that the wire is already added
	auto it = std::find(wires.begin(), wires.end(), wire);
	assert(it != wires.end());
	// Set input to 0V
	wire->inputModule->inputs[wire->inputId].value = 0.0;
	// Remove the wire
	wires.erase(it);
	updateActive();
}

void engineSetParam(Module *module, int paramId, float value) {
	module->params[paramId].value = value;
}

void engineSetParamSmooth(Module *module, int paramId, float value) {
	VIPLock vipLock(vipMutex);
	std::lock_guard<std::mutex> lock(mutex);
	// Since only one param can be smoothed at a time, if another param is currently being smoothed, skip to its final state
	if (smoothModule && !(smoothModule == module && smoothParamId == paramId)) {
		smoothModule->params[smoothParamId].value = smoothValue;
	}
	smoothModule = module;
	smoothParamId = paramId;
	smoothValue = value;
}

void engineSetSampleRate(float newSampleRate) {
	VIPLock vipLock(vipMutex);
	std::lock_guard<std::mutex> lock(mutex);
	sampleRate = newSampleRate;
	// onSampleRateChange
	for (Module *module : modules) {
		module->onSampleRateChange();
	}
}

float engineGetSampleRate() {
	return sampleRate;
}


} // namespace rack

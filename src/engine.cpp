#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <set>
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

static std::set<Module*> modules;
// Merely used for keeping track of which module inputs point to which module outputs, to prevent pointer mistakes and make the rack API more rigorous
static std::set<Wire*> wires;

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
	// Step all modules
	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
	for (Module *module : modules) {
		// Start clock for CPU usage
		start = std::chrono::high_resolution_clock::now();
		// Step module by one frame
		module->step();
		// Stop clock and smooth step time value
		end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> diff = end - start;
		float elapsed = diff.count() * gSampleRate;
		const float lambda = 1.0;
		module->cpuTime += (elapsed - module->cpuTime) * lambda / gSampleRate;
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
	const int stepSize = 32;

	while (running) {
		vipMutex.wait();

		auto start = std::chrono::high_resolution_clock::now();
		{
			std::lock_guard<std::mutex> lock(mutex);
			for (int i = 0; i < stepSize; i++) {
				engineStep();
			}
		}
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::nanoseconds((long) (0.10 * 1e9 * stepSize / gSampleRate)) - (end - start);
		// Avoid pegging the CPU at 100% when there are no "blocking" modules like AudioInterface, but still step audio at a reasonable rate
		// if (duration > 0)
		// 	std::this_thread::sleep_for(duration);
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
	assert(modules.find(module) == modules.end());
	modules.insert(module);
}

void engineRemoveModule(Module *module) {
	assert(module);
	VIPLock vipLock(vipMutex);
	std::lock_guard<std::mutex> lock(mutex);
	// Remove parameter interpolation which point to this module
	if (module == smoothModule) {
		smoothModule = NULL;
	}
	// Check that all wires are disconnected
	for (Wire *wire : wires) {
		assert(wire->outputModule != module);
		assert(wire->inputModule != module);
	}
	auto it = modules.find(module);
	if (it != modules.end()) {
		modules.erase(it);
	}
}

void engineAddWire(Wire *wire) {
	assert(wire);
	VIPLock vipLock(vipMutex);
	std::lock_guard<std::mutex> lock(mutex);
	// Check that the wire is not already added
	assert(wires.find(wire) == wires.end());
	assert(wire->outputModule);
	assert(wire->inputModule);
	// Check that the inputs/outputs are not already used by another cable
	for (Wire *wire2 : wires) {
		assert(wire2 != wire);
		assert(!(wire2->outputModule == wire->outputModule && wire2->outputId == wire->outputId));
		assert(!(wire2->inputModule == wire->inputModule && wire2->inputId == wire->inputId));
	}
	// Connect the wire to inputModule
	wires.insert(wire);
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

	auto it = wires.find(wire);
	assert(it != wires.end());
	wires.erase(it);
}

void engineSetParamSmooth(Module *module, int paramId, float value) {
	VIPLock vipLock(vipMutex);
	std::lock_guard<std::mutex> lock(mutex);
	// Check existing parameter interpolation
	if (smoothModule) {
		if (!(smoothModule == module && smoothParamId == paramId)) {
			// Jump param value to smooth value
			smoothModule->params[smoothParamId] = smoothValue;
		}
	}
	smoothModule = module;
	smoothParamId = paramId;
	smoothValue = value;
}


} // namespace rack

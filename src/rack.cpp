#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <set>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "Rack.hpp"


namespace rack {

static std::thread thread;
static std::mutex mutex;
static bool running;

static std::set<Module*> modules;
// Merely used for keeping track of which module inputs point to which module outputs, to prevent pointer mistakes and make the rack API rigorous
static std::set<Wire*> wires;

// Parameter interpolation
static Module *smoothModule = NULL;
static int smoothParamId;
static float smoothValue;


// HACK
static std::mutex requestMutex;
static std::condition_variable requestCv;
static bool request = false;

struct RackRequest {
	RackRequest() {
		std::unique_lock<std::mutex> lock(requestMutex);
		request = true;
	}
	~RackRequest() {
		std::unique_lock<std::mutex> lock(requestMutex);
		request = false;
		lock.unlock();
		requestCv.notify_all();
	}
};
// END HACK


void rackInit() {
}

void rackDestroy() {
	// Make sure there are no wires or modules in the rack on destruction. This suggests that a module failed to remove itself before the GUI was destroyed.
	assert(wires.empty());
	assert(modules.empty());
}

void rackStep() {
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
			value += delta * lambda / SAMPLE_RATE;
			smoothModule->params[smoothParamId] = value;
		}
	}
	// Step all modules
	for (Module *module : modules) {
		module->step();
	}
}

static void rackRun() {
	// Every time the rack waits and locks a mutex, it steps this many frames
	const int stepSize = 32;

	while (running) {
		auto start = std::chrono::high_resolution_clock::now();
		// This lock is to make sure the GUI gets higher priority than this thread
		{
			std::unique_lock<std::mutex> lock(requestMutex);
			while (request)
				requestCv.wait(lock);
		}
		{
			std::lock_guard<std::mutex> lock(mutex);
			for (int i = 0; i < stepSize; i++) {
				rackStep();
			}
		}
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::nanoseconds((long) (0.9 * 1e9 * stepSize / SAMPLE_RATE)) - (end - start);
		std::this_thread::sleep_for(duration);
	}
}

void rackStart() {
	running = true;
	thread = std::thread(rackRun);
}

void rackStop() {
	running = false;
	thread.join();
}

void rackAddModule(Module *module) {
	assert(module);
	RackRequest rm;
	std::lock_guard<std::mutex> lock(mutex);
	// Check that the module is not already added
	assert(modules.find(module) == modules.end());
	modules.insert(module);
}

void rackRemoveModule(Module *module) {
	assert(module);
	RackRequest rm;
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

void rackConnectWire(Wire *wire) {
	assert(wire);
	RackRequest rm;
	std::lock_guard<std::mutex> lock(mutex);
	// It would probably be good to reset the wire voltage
	wire->value = 0.0;
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
	wire->inputModule->inputs[wire->inputId] = &wire->value;
	wire->outputModule->outputs[wire->outputId] = &wire->value;
}

void rackDisconnectWire(Wire *wire) {
	assert(wire);
	RackRequest rm;
	std::lock_guard<std::mutex> lock(mutex);
	// Disconnect wire from inputModule
	wire->inputModule->inputs[wire->inputId] = NULL;
	wire->outputModule->outputs[wire->outputId] = NULL;

	auto it = wires.find(wire);
	assert(it != wires.end());
	wires.erase(it);
}

void rackSetParamSmooth(Module *module, int paramId, float value) {
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

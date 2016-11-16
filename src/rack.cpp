#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "rack.hpp"


static std::thread thread;
static std::mutex mutex;
static std::condition_variable cv;
static long frame;
static long frameLimit;
static bool running;

static std::set<Module*> modules;
// Merely used for keeping track of which module inputs point to which module outputs, to prevent pointer mistakes and make the rack API rigorous
static std::set<Wire*> wires;


static Module *smoothModule = NULL;
static int smoothParamId;
static float smoothValue;


void rackInit() {
}

void rackDestroy() {
	// Make sure there are no wires or modules in the rack on destruction. This suggests that a module failed to remove itself when the GUI was destroyed.
	assert(wires.empty());
	assert(modules.empty());
}

void rackStep() {
	// Param interpolation
	if (smoothModule) {
		float value = smoothModule->params[smoothParamId];
		const float minSpeed = 0.001 * 60.0 / SAMPLE_RATE;
		const float lpCoeff = 60.0 / SAMPLE_RATE / 1.0; // decay rate is 1 graphics frame
		float delta = smoothValue - value;
		float speed = fmaxf(fabsf(delta) * lpCoeff, minSpeed);

		if (delta < 0) {
			value -= speed;
			if (value < smoothValue) value = smoothValue;
		}
		else if (delta > 0) {
			value += speed;
			if (value > smoothValue) value = smoothValue;
		}

		smoothModule->params[smoothParamId] = value;
		if (value == smoothValue) {
			smoothModule = NULL;
		}
	}
	// Step all modules
	for (Module *module : modules) {
		module->step();
	}
}

void rackRun() {
	while (1) {
		std::unique_lock<std::mutex> lock(mutex);
		if (!running)
			break;
		if (frame >= frameLimit) {
			// Delay for at most 1ms if there are no needed frames
			cv.wait_for(lock, std::chrono::milliseconds(1));
		}
		frame++;
		// Speed up
		for (int i = 0; i < 16; i++)
			rackStep();
	}
}

void rackStart() {
	frame = 0;
	frameLimit = 0;
	running = true;
	thread = std::thread(rackRun);
}

void rackStop() {
	{
		std::unique_lock<std::mutex> lock(mutex);
		running = false;
	}
	cv.notify_all();
	thread.join();
}

void rackAddModule(Module *module) {
	assert(module);
	std::unique_lock<std::mutex> lock(mutex);
	// Check that the module is not already added
	assert(modules.find(module) == modules.end());
	modules.insert(module);
}

void rackRemoveModule(Module *module) {
	assert(module);
	std::unique_lock<std::mutex> lock(mutex);
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
	std::unique_lock<std::mutex> lock(mutex);
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
	std::unique_lock<std::mutex> lock(mutex);
	// Disconnect wire from inputModule
	wire->inputModule->inputs[wire->inputId] = NULL;
	wire->outputModule->outputs[wire->outputId] = NULL;

	auto it = wires.find(wire);
	assert(it != wires.end());
	wires.erase(it);
}

long rackGetFrame() {
	return frame;
}

void rackRequestFrame(long f) {
	std::unique_lock<std::mutex> lock(mutex);
	if (f > frameLimit) {
		frameLimit = f;
		lock.unlock();
		cv.notify_all();
	}
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

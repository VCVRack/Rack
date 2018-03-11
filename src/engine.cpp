#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include <xmmintrin.h>
#include <pmmintrin.h>

#include "engine.hpp"


namespace rack {

float sampleRate;
float sampleTime;
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


float Light::getBrightness() {
	// LEDs are diodes, so don't allow reverse current.
	// For some reason, instead of the RMS, the sqrt of RMS looks better
	return powf(fmaxf(0.f, value), 0.25f);
}

void Light::setBrightnessSmooth(float brightness, float frames) {
	float v = (brightness > 0.f) ? brightness * brightness : 0.f;
	if (v < value) {
		// Fade out light with lambda = framerate
		value += (v - value) * sampleTime * frames * 60.f;
	}
	else {
		// Immediately illuminate light
		value = v;
	}
}


void Wire::step() {
	float value = outputModule->outputs[outputId].value;
	inputModule->inputs[inputId].value = value;
}


void engineInit() {
	engineSetSampleRate(44100.0);
}

void engineDestroy() {
	// Make sure there are no wires or modules in the rack on destruction. This suggests that a module failed to remove itself before the WINDOW was destroyed.
	assert(wires.empty());
	assert(modules.empty());
}

static void engineStep() {
	// Param interpolation
	if (smoothModule) {
		float value = smoothModule->params[smoothParamId].value;
		const float lambda = 60.0; // decay rate is 1 graphics frame
		float delta = smoothValue - value;
		float newValue = value + delta * lambda * sampleTime;
		if (value == newValue) {
			// Snap to actual smooth value if the value doesn't change enough (due to the granularity of floats)
			smoothModule->params[smoothParamId].value = smoothValue;
			smoothModule = NULL;
		}
		else {
			smoothModule->params[smoothParamId].value = newValue;
		}
	}

	// Step modules
	for (Module *module : modules) {
		module->step();

		// TODO skip this step when plug lights are disabled
		// Step ports
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

	// Step cables by moving their output values to inputs
	for (Wire *wire : wires) {
		wire->step();
	}
}

static void engineRun() {
	// Set CPU to flush-to-zero (FTZ) and denormals-are-zero (DAZ) mode
	// https://software.intel.com/en-us/node/682949
	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
	_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

	// Every time the engine waits and locks a mutex, it steps this many frames
	const int mutexSteps = 64;
	// Time in seconds that the engine is rushing ahead of the estimated clock time
	double ahead = 0.0;
	auto lastTime = std::chrono::high_resolution_clock::now();

	while (running) {
		vipMutex.wait();

		if (!gPaused) {
			std::lock_guard<std::mutex> lock(mutex);
			for (int i = 0; i < mutexSteps; i++) {
				engineStep();
			}
		}

		double stepTime = mutexSteps * sampleTime;
		ahead += stepTime;
		auto currTime = std::chrono::high_resolution_clock::now();
		const double aheadFactor = 2.0;
		ahead -= aheadFactor * std::chrono::duration<double>(currTime - lastTime).count();
		lastTime = currTime;
		ahead = fmaxf(ahead, 0.0);

		// Avoid pegging the CPU at 100% when there are no "blocking" modules like AudioInterface, but still step audio at a reasonable rate
		// The number of steps to wait before possibly sleeping
		const double aheadMax = 1.0; // seconds
		if (ahead > aheadMax) {
			std::this_thread::sleep_for(std::chrono::duration<double>(stepTime));
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
	sampleTime = 1.0 / sampleRate;
	// onSampleRateChange
	for (Module *module : modules) {
		module->onSampleRateChange();
	}
}

float engineGetSampleRate() {
	return sampleRate;
}

float engineGetSampleTime() {
	return sampleTime;
}

} // namespace rack

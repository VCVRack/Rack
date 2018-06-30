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

bool gPaused = false;
std::vector<Module*> gModules;
std::vector<Wire*> gWires;
bool gPowerMeter = false;

static bool running = false;
static float sampleRate = 44100.f;
static float sampleTime = 1.f / sampleRate;
static float sampleRateRequested = sampleRate;

static Module *resetModule = NULL;
static Module *randomizeModule = NULL;

static std::mutex mutex;
static std::thread thread;
static VIPMutex vipMutex;

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
}

void engineDestroy() {
	// Make sure there are no wires or modules in the rack on destruction. This suggests that a module failed to remove itself before the RackWidget was destroyed.
	assert(gWires.empty());
	assert(gModules.empty());
}

static void engineStep() {
	// Sample rate
	if (sampleRateRequested != sampleRate) {
		sampleRate = sampleRateRequested;
		sampleTime = 1.f / sampleRate;
		for (Module *module : gModules) {
			module->onSampleRateChange();
		}
	}

	// Events
	if (resetModule) {
		resetModule->onReset();
		resetModule = NULL;
	}
	if (randomizeModule) {
		randomizeModule->onRandomize();
		randomizeModule = NULL;
	}

	// Param smoothing
	{
		Module *localSmoothModule = smoothModule;
		int localSmoothParamId = smoothParamId;
		float localSmoothValue = smoothValue;
		if (localSmoothModule) {
			float value = localSmoothModule->params[localSmoothParamId].value;
			const float lambda = 60.0; // decay rate is 1 graphics frame
			float delta = localSmoothValue - value;
			float newValue = value + delta * lambda * sampleTime;
			if (value == newValue) {
				// Snap to actual smooth value if the value doesn't change enough (due to the granularity of floats)
				localSmoothModule->params[localSmoothParamId].value = localSmoothValue;
				smoothModule = NULL;
			}
			else {
				localSmoothModule->params[localSmoothParamId].value = newValue;
			}
		}
	}

	// Step modules
	for (Module *module : gModules) {
		std::chrono::high_resolution_clock::time_point startTime;
		if (gPowerMeter) {
			startTime = std::chrono::high_resolution_clock::now();

			module->step();

			auto stopTime = std::chrono::high_resolution_clock::now();
			float cpuTime = std::chrono::duration<float>(stopTime - startTime).count() * sampleRate;
			module->cpuTime += (cpuTime - module->cpuTime) * sampleTime / 0.5f;
		}
		else {
			module->step();
		}

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
	for (Wire *wire : gWires) {
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
	auto it = std::find(gModules.begin(), gModules.end(), module);
	assert(it == gModules.end());
	gModules.push_back(module);
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
	for (Wire *wire : gWires) {
		assert(wire->outputModule != module);
		assert(wire->inputModule != module);
	}
	// Check that the module actually exists
	auto it = std::find(gModules.begin(), gModules.end(), module);
	assert(it != gModules.end());
	// Remove it
	gModules.erase(it);
}

void engineResetModule(Module *module) {
	resetModule = module;
}

void engineRandomizeModule(Module *module) {
	randomizeModule = module;
}

static void updateActive() {
	// Set everything to inactive
	for (Module *module : gModules) {
		for (Input &input : module->inputs) {
			input.active = false;
		}
		for (Output &output : module->outputs) {
			output.active = false;
		}
	}
	// Set inputs/outputs to active
	for (Wire *wire : gWires) {
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
	for (Wire *wire2 : gWires) {
		assert(wire2 != wire);
		assert(!(wire2->inputModule == wire->inputModule && wire2->inputId == wire->inputId));
	}
	// Add the wire
	gWires.push_back(wire);
	updateActive();
}

void engineRemoveWire(Wire *wire) {
	assert(wire);
	VIPLock vipLock(vipMutex);
	std::lock_guard<std::mutex> lock(mutex);
	// Check that the wire is already added
	auto it = std::find(gWires.begin(), gWires.end(), wire);
	assert(it != gWires.end());
	// Set input to 0V
	wire->inputModule->inputs[wire->inputId].value = 0.0;
	// Remove the wire
	gWires.erase(it);
	updateActive();
}

void engineSetParam(Module *module, int paramId, float value) {
	module->params[paramId].value = value;
}

void engineSetParamSmooth(Module *module, int paramId, float value) {
	// If another param is being smoothed, jump value
	if (smoothModule && !(smoothModule == module && smoothParamId == paramId)) {
		smoothModule->params[smoothParamId].value = smoothValue;
	}
	smoothParamId = paramId;
	smoothValue = value;
	smoothModule = module;
}

void engineSetSampleRate(float newSampleRate) {
	sampleRateRequested = newSampleRate;
}

float engineGetSampleRate() {
	return sampleRate;
}

float engineGetSampleTime() {
	return sampleTime;
}

} // namespace rack

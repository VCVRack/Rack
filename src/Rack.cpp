#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <chrono>
#include <condition_variable>

#include "rack.hpp"


namespace rack {

/** Threads which obtain a VIPLock will cause wait() to block for other less important threads.
This does not provide the VIPs with an exclusive lock. That should be left up to another mutex shared between the less important thread.
*/
struct VIPMutex {
	int count = 0;
	std::condition_variable cv;
	std::mutex countMutex;

	/** Blocks until there are no remaining VIPLocks */
	void wait() {
		std::unique_lock<std::mutex> lock(countMutex);
		while (count > 0)
			cv.wait(lock);
	}
};

struct VIPLock {
	VIPMutex &m;
	VIPLock(VIPMutex &m) : m(m) {
		std::unique_lock<std::mutex> lock(m.countMutex);
		m.count++;
	}
	~VIPLock() {
		std::unique_lock<std::mutex> lock(m.countMutex);
		m.count--;
		lock.unlock();
		m.cv.notify_all();
	}
};


struct Rack::Impl {
	bool running = false;

	std::mutex mutex;
	std::thread audioThread;
	VIPMutex vipMutex;

	std::set<Module*> modules;
	// Merely used for keeping track of which module inputs point to which module outputs, to prevent pointer mistakes and make the rack API more rigorous
	std::set<Wire*> wires;

	// Parameter interpolation
	Module *smoothModule = NULL;
	int smoothParamId;
	float smoothValue;
};


Rack::Rack() {
	impl = new Rack::Impl();
	sampleRate = 44100.0;
}

Rack::~Rack() {
	// Make sure there are no wires or modules in the rack on destruction. This suggests that a module failed to remove itself before the GUI was destroyed.
	assert(impl->wires.empty());
	assert(impl->modules.empty());
	delete impl;
}

void Rack::start() {
	impl->running = true;
	impl->audioThread = std::thread(&Rack::run, this);
}

void Rack::stop() {
	impl->running = false;
	impl->audioThread.join();
}

void Rack::run() {
	// Every time the rack waits and locks a mutex, it steps this many frames
	const int stepSize = 32;

	while (impl->running) {
		impl->vipMutex.wait();

		auto start = std::chrono::high_resolution_clock::now();
		{
			std::lock_guard<std::mutex> lock(impl->mutex);
			for (int i = 0; i < stepSize; i++) {
				step();
			}
		}
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::nanoseconds((long) (0.9 * 1e9 * stepSize / sampleRate)) - (end - start);
		// Avoid pegging the CPU at 100% when there are no "blocking" modules like AudioInterface, but still step audio at a reasonable rate
		std::this_thread::sleep_for(duration);
	}
}

void Rack::step() {
	// Param interpolation
	if (impl->smoothModule) {
		float value = impl->smoothModule->params[impl->smoothParamId];
		const float lambda = 60.0; // decay rate is 1 graphics frame
		const float snap = 0.0001;
		float delta = impl->smoothValue - value;
		if (fabsf(delta) < snap) {
			impl->smoothModule->params[impl->smoothParamId] = impl->smoothValue;
			impl->smoothModule = NULL;
		}
		else {
			value += delta * lambda / sampleRate;
			impl->smoothModule->params[impl->smoothParamId] = value;
		}
	}
	// Step all modules
	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
	for (Module *module : impl->modules) {
		// Start clock for CPU usage
		start = std::chrono::high_resolution_clock::now();
		// Step module by one frame
		module->step();
		// Stop clock and smooth step time value
		end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> diff = end - start;
		float elapsed = diff.count() * sampleRate;
		const float lambda = 1.0;
		module->cpuTime += (elapsed - module->cpuTime) * lambda / sampleRate;
	}
}

void Rack::addModule(Module *module) {
	assert(module);
	VIPLock vipLock(impl->vipMutex);
	std::lock_guard<std::mutex> lock(impl->mutex);
	// Check that the module is not already added
	assert(impl->modules.find(module) == impl->modules.end());
	impl->modules.insert(module);
}

void Rack::removeModule(Module *module) {
	assert(module);
	VIPLock vipLock(impl->vipMutex);
	std::lock_guard<std::mutex> lock(impl->mutex);
	// Remove parameter interpolation which point to this module
	if (module == impl->smoothModule) {
		impl->smoothModule = NULL;
	}
	// Check that all wires are disconnected
	for (Wire *wire : impl->wires) {
		assert(wire->outputModule != module);
		assert(wire->inputModule != module);
	}
	auto it = impl->modules.find(module);
	if (it != impl->modules.end()) {
		impl->modules.erase(it);
	}
}

void Rack::addWire(Wire *wire) {
	assert(wire);
	VIPLock vipLock(impl->vipMutex);
	std::lock_guard<std::mutex> lock(impl->mutex);
	// It would probably be good to reset the wire voltage
	wire->value = 0.0;
	// Check that the wire is not already added
	assert(impl->wires.find(wire) == impl->wires.end());
	assert(wire->outputModule);
	assert(wire->inputModule);
	// Check that the inputs/outputs are not already used by another cable
	for (Wire *wire2 : impl->wires) {
		assert(wire2 != wire);
		assert(!(wire2->outputModule == wire->outputModule && wire2->outputId == wire->outputId));
		assert(!(wire2->inputModule == wire->inputModule && wire2->inputId == wire->inputId));
	}
	// Connect the wire to inputModule
	impl->wires.insert(wire);
	wire->inputModule->inputs[wire->inputId] = &wire->value;
	wire->outputModule->outputs[wire->outputId] = &wire->value;
}

void Rack::removeWire(Wire *wire) {
	assert(wire);
	VIPLock vipLock(impl->vipMutex);
	std::lock_guard<std::mutex> lock(impl->mutex);
	// Disconnect wire from inputModule
	wire->inputModule->inputs[wire->inputId] = NULL;
	wire->outputModule->outputs[wire->outputId] = NULL;

	auto it = impl->wires.find(wire);
	assert(it != impl->wires.end());
	impl->wires.erase(it);
}

void Rack::setParamSmooth(Module *module, int paramId, float value) {
	VIPLock vipLock(impl->vipMutex);
	std::lock_guard<std::mutex> lock(impl->mutex);
	// Check existing parameter interpolation
	if (impl->smoothModule) {
		if (!(impl->smoothModule == module && impl->smoothParamId == paramId)) {
			// Jump param value to smooth value
			impl->smoothModule->params[impl->smoothParamId] = impl->smoothValue;
		}
	}
	impl->smoothModule = module;
	impl->smoothParamId = paramId;
	impl->smoothValue = value;
}


} // namespace rack

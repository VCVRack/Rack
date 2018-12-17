#pragma once
#include <vector>
#include "common.hpp"
#include "engine/Module.hpp"
#include "engine/Wire.hpp"


namespace rack {


struct Engine {
	/** Plugins should not manipulate other modules or wires unless that is the entire purpose of the module.
	Your plugin needs to have a clear purpose for manipulating other modules and wires and must be done with a good UX.
	*/
	std::vector<Module*> modules;
	std::vector<Wire*> wires;
	bool paused = false;
	bool powerMeter = false;

	struct Internal;
	Internal *internal;

	Engine();
	~Engine();
	/** Starts engine thread */
	void start();
	/** Stops engine thread */
	void stop();
	/** Does not transfer pointer ownership */
	void addModule(Module *module);
	void removeModule(Module *module);
	void resetModule(Module *module);
	void randomizeModule(Module *module);
	/** Does not transfer pointer ownership */
	void addWire(Wire *wire);
	void removeWire(Wire *wire);
	void setParam(Module *module, int paramId, float value);
	void setParamSmooth(Module *module, int paramId, float value);

	void setSampleRate(float sampleRate);
	float getSampleRate();
	/** Returns the inverse of the current sample rate */
	float getSampleTime();
};


// TODO Move to global state header
extern Engine *gEngine;


} // namespace rack

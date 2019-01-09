#pragma once
#include "common.hpp"
#include "engine/Module.hpp"
#include "engine/Cable.hpp"
#include <vector>


namespace rack {


struct Engine {
	/** Plugins should not manipulate other modules or cables unless that is the entire purpose of the module.
	Your plugin needs to have a clear purpose for manipulating other modules and cables and must be done with a good UX.
	*/
	std::vector<Module*> modules;
	std::vector<Cable*> cables;
	bool paused = false;

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
	void addCable(Cable *cable);
	void removeCable(Cable *cable);
	void setParam(Module *module, int paramId, float value);
	void setParamSmooth(Module *module, int paramId, float value);
	int getNextModuleId();

	void setSampleRate(float sampleRate);
	float getSampleRate();
	/** Returns the inverse of the current sample rate */
	float getSampleTime();
};


} // namespace rack

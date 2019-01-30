#pragma once
#include "common.hpp"
#include "engine/Module.hpp"
#include "engine/Cable.hpp"
#include <vector>


namespace rack {
namespace engine {


struct Engine {
	struct Internal;
	Internal *internal;

	Engine();
	~Engine();
	/** Starts engine thread */
	void start();
	/** Stops engine thread */
	void stop();
	void setThreadCount(int threadCount);
	int getThreadCount();
	void setPaused(bool paused);
	bool isPaused();
	/** Does not transfer pointer ownership */
	void addModule(Module *module);
	void removeModule(Module *module);
	void resetModule(Module *module);
	void randomizeModule(Module *module);
	void bypassModule(Module *module, bool bypass);
	/** Does not transfer pointer ownership */
	void addCable(Cable *cable);
	void removeCable(Cable *cable);
	void setParam(Module *module, int paramId, float value);
	float getParam(Module *module, int paramId);
	void setSmoothParam(Module *module, int paramId, float value);
	float getSmoothParam(Module *module, int paramId);
	int getNextModuleId();

	void setSampleRate(float sampleRate);
	float getSampleRate();
	/** Returns the inverse of the current sample rate */
	float getSampleTime();
};


} // namespace engine
} // namespace rack

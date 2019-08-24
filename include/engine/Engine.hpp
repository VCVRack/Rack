#pragma once
#include <common.hpp>
#include <engine/Module.hpp>
#include <engine/Cable.hpp>
#include <engine/ParamHandle.hpp>
#include <vector>


namespace rack {
namespace engine {


struct Engine {
	struct Internal;
	Internal* internal;

	Engine();
	~Engine();
	/** Starts engine thread. */
	void start();
	/** Stops engine thread. */
	void stop();
	void setPaused(bool paused);
	bool isPaused();
	float getSampleRate();
	/** Returns the inverse of the current sample rate. */
	float getSampleTime();
	/** Causes worker threads to block on a mutex instead of spinlock.
	Call this in your Module::step() method to hint that the operation will take more than ~0.1 ms.
	*/
	void yieldWorkers();
	uint64_t getFrame();

	// Modules
	/** Adds a module to the rack engine.
	The module ID must not be taken by another module.
	If the module ID is -1, an ID is automatically assigned.
	Does not transfer pointer ownership.
	*/
	void addModule(Module* module);
	void removeModule(Module* module);
	Module* getModule(int moduleId);
	void resetModule(Module* module);
	void randomizeModule(Module* module);
	void bypassModule(Module* module, bool bypass);

	// Cables
	/** Adds a cable to the rack engine.
	The cable ID must not be taken by another cable.
	If the cable ID is -1, an ID is automatically assigned.
	Does not transfer pointer ownership.
	*/
	void addCable(Cable* cable);
	void removeCable(Cable* cable);

	// Params
	void setParam(Module* module, int paramId, float value);
	float getParam(Module* module, int paramId);
	void setSmoothParam(Module* module, int paramId, float value);
	float getSmoothParam(Module* module, int paramId);

	// ParamHandles
	void addParamHandle(ParamHandle* paramHandle);
	void removeParamHandle(ParamHandle* paramHandle);
	/** Returns the unique ParamHandle for the given paramId */
	ParamHandle* getParamHandle(int moduleId, int paramId);
	/** Use getParamHandle(int, int) instead. */
	DEPRECATED ParamHandle* getParamHandle(Module* module, int paramId);
	/** Sets the ParamHandle IDs and module pointer.
	If `overwrite` is true and another ParamHandle points to the same param, unsets that one and replaces it with the given handle.
	*/
	void updateParamHandle(ParamHandle* paramHandle, int moduleId, int paramId, bool overwrite = true);
};


} // namespace engine
} // namespace rack

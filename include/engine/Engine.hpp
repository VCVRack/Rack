#pragma once
#include <vector>

#include <common.hpp>
#include <engine/Module.hpp>
#include <engine/Cable.hpp>
#include <engine/ParamHandle.hpp>


namespace rack {
namespace engine {


/** Manages Modules and Cables and steps them in time.

All methods are thread-safe and can safely be called from anywhere.

The methods clear, addModule, removeModule, moduleToJson, moduleFromJson, addCable, removeCable, addParamHandle, removeParamHandle, toJson, and fromJson cannot be run simultaneously with any other Engine method.
Calling these methods inside any Engine method will result in a deadlock.
*/
struct Engine {
	struct Internal;
	Internal* internal;

	Engine();
	~Engine();

	/** Removes all modules and cables. */
	void clear();
	/** Advances the engine by `frames` frames.
	Only call this method from the primary module.
	*/
	void step(int frames);
	void setPrimaryModule(Module* module);
	Module* getPrimaryModule();

	/** Returns the sample rate used by the engine for stepping each module.
	*/
	float getSampleRate();
	/** Returns the inverse of the current sample rate.
	*/
	float getSampleTime();
	/** Causes worker threads to block on a mutex instead of spinlock.
	Call this in your Module::step() method to hint that the operation will take more than ~0.1 ms.
	*/
	void yieldWorkers();
	/** Returns the number of audio samples since the Engine's first sample.
	*/
	int64_t getFrame();
	/** Returns the estimated timestamp corresponding to the current frame, based on the timestamp of when step() was last called.
	Calculated by `stepTime + framesSinceStep / sampleRate`.
	*/
	int64_t getFrameTime();
	/** Returns the frame when step() was last called.
	*/
	int64_t getStepFrame();
	/** Returns the timestamp in nanoseconds when step() was last called.
	*/
	int64_t getStepTime();
	/** Returns the total number of frames in the current step() call.
	*/
	int getStepFrames();
	/** Returns the total time that step() is advancing, in nanoseconds.
	Calculated by `stepFrames / sampleRate`.
	*/
	int64_t getStepDuration();

	// Modules
	size_t getNumModules();
	void getModuleIds(int* moduleIds, int len);
	std::vector<int> getModuleIds();
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
	void bypassModule(Module* module, bool bypassed);
	/** Serializes/deserializes with locking, ensuring that Module::process() is not called during toJson()/fromJson().
	*/
	json_t* moduleToJson(Module* module);
	void moduleFromJson(Module* module, json_t* rootJ);

	// Cables
	/** Adds a cable to the rack engine.
	The cable ID must not be taken by another cable.
	If the cable ID is -1, an ID is automatically assigned.
	Does not transfer pointer ownership.
	*/
	void addCable(Cable* cable);
	void removeCable(Cable* cable);
	Cable* getCable(int cableId);

	// Params
	void setParam(Module* module, int paramId, float value);
	float getParam(Module* module, int paramId);
	/** Requests the parameter to smoothly change toward `value`.
	*/
	void setSmoothParam(Module* module, int paramId, float value);
	/** Returns the target value before smoothing.
	*/
	float getSmoothParam(Module* module, int paramId);

	// ParamHandles
	void addParamHandle(ParamHandle* paramHandle);
	void removeParamHandle(ParamHandle* paramHandle);
	/** Returns the unique ParamHandle for the given paramId
	*/
	ParamHandle* getParamHandle(int moduleId, int paramId);
	/** Use getParamHandle(int, int) instead.
	*/
	DEPRECATED ParamHandle* getParamHandle(Module* module, int paramId);
	/** Sets the ParamHandle IDs and module pointer.
	If `overwrite` is true and another ParamHandle points to the same param, unsets that one and replaces it with the given handle.
	*/
	void updateParamHandle(ParamHandle* paramHandle, int moduleId, int paramId, bool overwrite = true);

	json_t* toJson();
	void fromJson(json_t* rootJ);
};


} // namespace engine
} // namespace rack

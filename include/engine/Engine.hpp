#pragma once
#include <vector>

#include <common.hpp>
#include <engine/Module.hpp>
#include <engine/Cable.hpp>
#include <engine/ParamHandle.hpp>


namespace rack {
/** High-performance classes handling modules and voltage signals between them
*/
namespace engine {


/** Manages Modules and Cables and steps them in time.

Engine contains a shared mutex that locks when the Engine state is being read or written (manipulated).
Methods that share-lock (stated in their documentation) can be called simultaneously with other share-locking methods.
Methods that exclusively lock cannot be called simultaneously or recursively with another share-locking or exclusive-locking method.
*/
struct Engine {
	struct Internal;
	Internal* internal;

	PRIVATE Engine();
	PRIVATE ~Engine();

	/** Removes all modules and cables.
	Exclusively locks.
	*/
	void clear();
	PRIVATE void clear_NoLock();
	/** Advances the engine by `frames` frames.
	Only call this method from the master module.
	Share-locks. Also locks so only one stepBlock() can be called simultaneously or recursively.
	*/
	void stepBlock(int frames);
	/** Module does not need to belong to the Engine.
	However, Engine will unset the master module when it is removed from the Engine.
	NULL will unset the master module.
	Exclusively locks.
	*/
	void setMasterModule(Module* module);
	void setMasterModule_NoLock(Module* module);
	Module* getMasterModule();

	/** Returns the sample rate used by the engine for stepping each module.
	*/
	float getSampleRate();
	/** Sets the sample rate to step the modules.
	Exclusively locks.
	*/
	PRIVATE void setSampleRate(float sampleRate);
	/** Sets the sample rate if the sample rate in the settings is "Auto".
	Exclusively locks.
	*/
	void setSuggestedSampleRate(float suggestedSampleRate);
	/** Returns the inverse of the current sample rate.
	*/
	float getSampleTime();
	/** Causes worker threads to block on a mutex instead of spinlock.
	Call this in your Module::stepBlock() method to hint that the operation will take more than ~0.1 ms.
	*/
	void yieldWorkers();
	/** Returns the number of sample frames since the Engine was created.
	*/
	int64_t getFrame();
	/** Returns the number of stepBlock() calls since the Engine was created.
	*/
	int64_t getBlock();
	/** Returns the frame when stepBlock() was last called.
	*/
	int64_t getBlockFrame();
	/** Returns the time in seconds when stepBlock() was last called.
	*/
	double getBlockTime();
	/** Returns the number of frames requested by the last stepBlock() call.
	*/
	int getBlockFrames();
	/** Returns the total time that stepBlock() is advancing, in seconds.
	Calculated by `blockFrames / sampleRate`.
	*/
	double getBlockDuration();
	/** Returns the average block processing time divided by block time in the last T seconds.
	*/
	double getMeterAverage();
	double getMeterMax();

	// Modules
	size_t getNumModules();
	/** Fills `moduleIds` with up to `len` module IDs in the rack.
	Returns the number of IDs written.
	This C-like method does no allocations. The vector C++ version below does.
	Share-locks.
	*/
	size_t getModuleIds(int64_t* moduleIds, size_t len);
	/** Returns a vector of module IDs in the rack.
	Share-locks.
	*/
	std::vector<int64_t> getModuleIds();
	/** Adds a Module to the rack.
	The module ID must not be taken by another Module.
	If the module ID is -1, an ID is automatically assigned.
	Does not transfer pointer ownership.
	Exclusively locks.
	*/
	void addModule(Module* module);
	/** Removes a Module from the rack.
	Exclusively locks.
	*/
	void removeModule(Module* module);
	PRIVATE void removeModule_NoLock(Module* module);
	/** Checks whether a Module is in the rack.
	Share-locks.
	*/
	bool hasModule(Module* module);
	/** Returns the Module with the given ID in the rack.
	Share-locks.
	*/
	Module* getModule(int64_t moduleId);
	Module* getModule_NoLock(int64_t moduleId);
	/** Triggers a ResetEvent for the given Module.
	Exclusively locks.
	*/
	void resetModule(Module* module);
	/** Triggers a RandomizeEvent for the given Module.
	Exclusively locks.
	*/
	void randomizeModule(Module* module);
	/** Sets the bypassed state and triggers a BypassEvent or UnBypassEvent of the given Module.
	Exclusively locks.
	*/
	void bypassModule(Module* module, bool bypassed);
	/** Serializes the given Module with locking, ensuring that Module::process() is not called simultaneously.
	Share-locks.
	*/
	json_t* moduleToJson(Module* module);
	/** Serializes the given Module with locking, ensuring that Module::process() is not called simultaneously.
	Exclusively locks.
	*/
	void moduleFromJson(Module* module, json_t* rootJ);
	/** Dispatches Save event to a module.
	Share-locks.
	*/
	void prepareSaveModule(Module* module);
	/** Dispatches Save event to all modules.
	Share-locks.
	*/
	void prepareSave();

	// Cables
	size_t getNumCables();
	/** Fills `cableIds` with up to `len` cable IDs in the rack.
	Returns the number of IDs written.
	This C-like method does no allocations. The vector C++ version below does.
	Share-locks.
	*/
	size_t getCableIds(int64_t* cableIds, size_t len);
	/** Returns a vector of cable IDs in the rack.
	Share-locks.
	*/
	std::vector<int64_t> getCableIds();
	/** Adds a Cable to the rack.
	The cable ID must not be taken by another cable.
	If the cable ID is -1, an ID is automatically assigned.
	Does not transfer pointer ownership.
	Exclusively locks.
	*/
	void addCable(Cable* cable);
	/** Removes a Cable from the rack.
	Exclusively locks.
	*/
	void removeCable(Cable* cable);
	PRIVATE void removeCable_NoLock(Cable* cable);
	/** Checks whether a Cable is in the rack.
	Share-locks.
	*/
	bool hasCable(Cable* cable);
	/** Returns the Cable with the given ID in the rack.
	Share-locks.
	*/
	Cable* getCable(int64_t cableId);

	// Params
	void setParamValue(Module* module, int paramId, float value);
	float getParamValue(Module* module, int paramId);
	/** Requests the parameter to smoothly change toward `value`.
	*/
	void setParamSmoothValue(Module* module, int paramId, float value);
	/** Returns the target value before smoothing.
	*/
	float getParamSmoothValue(Module* module, int paramId);

	// ParamHandles
	/** Adds a ParamHandle to the rack.
	Does not automatically update the ParamHandle.
	Exclusively locks.
	*/
	void addParamHandle(ParamHandle* paramHandle);
	/**
	Exclusively locks.
	*/
	void removeParamHandle(ParamHandle* paramHandle);
	PRIVATE void removeParamHandle_NoLock(ParamHandle* paramHandle);
	/** Returns the unique ParamHandle for the given paramId
	Share-locks.
	*/
	ParamHandle* getParamHandle(int64_t moduleId, int paramId);
	ParamHandle* getParamHandle_NoLock(int64_t moduleId, int paramId);
	/** Use getParamHandle(moduleId, paramId) instead.
	Share-locks.
	*/
	DEPRECATED ParamHandle* getParamHandle(Module* module, int paramId);
	/** Sets the ParamHandle IDs and module pointer.
	If `overwrite` is true and another ParamHandle points to the same param, unsets that one and replaces it with the given handle.
	Exclusively locks.
	*/
	void updateParamHandle(ParamHandle* paramHandle, int64_t moduleId, int paramId, bool overwrite = true);
	void updateParamHandle_NoLock(ParamHandle* paramHandle, int64_t moduleId, int paramId, bool overwrite = true);

	/** Serializes the rack.
	Share-locks.
	*/
	json_t* toJson();
	/** Deserializes the rack.
	Exclusively locks.
	*/
	void fromJson(json_t* rootJ);

	/** If no master module is set, the fallback Engine thread will step blocks, using the CPU clock for timing.
	*/
	PRIVATE void startFallbackThread();
};


} // namespace engine
} // namespace rack

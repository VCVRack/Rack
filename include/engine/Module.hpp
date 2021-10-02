#pragma once
#include <vector>

#include <jansson.h>

#include <common.hpp>
#include <string.hpp>
#include <plugin/Model.hpp>
#include <engine/Param.hpp>
#include <engine/Port.hpp>
#include <engine/Light.hpp>
#include <engine/ParamQuantity.hpp>
#include <engine/PortInfo.hpp>
#include <engine/LightInfo.hpp>


namespace rack {


namespace plugin {
struct Model;
}


namespace engine {


/** DSP processor instance for your module. */
struct Module {
	struct Internal;
	Internal* internal;

	plugin::Model* model = NULL;
	/** Unique ID for referring to the module in the engine.
	Between 0 and 2^53 since this is serialized with JSON.
	Assigned when added to the engine.
	*/
	int64_t id = -1;

	/** Arrays of components.
	Initialized with config().
	*/
	std::vector<Param> params;
	std::vector<Input> inputs;
	std::vector<Output> outputs;
	std::vector<Light> lights;

	/** Arrays of component metadata.
	Initialized with configParam(), configInput(), configOutput(), and configLight().
	LightInfos are initialized to null unless configLight() is called.
	*/
	std::vector<ParamQuantity*> paramQuantities;
	std::vector<PortInfo*> inputInfos;
	std::vector<PortInfo*> outputInfos;
	std::vector<LightInfo*> lightInfos;

	/** Represents a message-passing channel for an adjacent module. */
	struct Expander {
		/** ID of the expander module, or -1 if nonexistent. */
		int64_t moduleId = -1;
		/** Pointer to the expander Module, or NULL if nonexistent. */
		Module* module = NULL;
		/** Double buffer for receiving messages from the expander module.
		If you intend to receive messages from an expander, allocate both message buffers with identical blocks of memory (arrays, structs, etc).
		Remember to free the buffer in the Module destructor.
		Example:

			rightExpander.producerMessage = new MyExpanderMessage;
			rightExpander.consumerMessage = new MyExpanderMessage;

		You must check the expander module's `model` before attempting to write its message buffer.
		Once the module is checked, you can reinterpret_cast its producerMessage at no performance cost.

		Producer messages are intended to be write-only.
		Consumer messages are intended to be read-only.

		Once you write a message, set messageFlipRequested to true to request that the messages are flipped at the end of the timestep.
		This means that message-passing has 1-sample latency.

		You may choose for your Module to instead write to its own message buffer for consumption by other modules, i.e. the expander "pulls" rather than this module "pushing".
		As long as this convention is followed by the other module, this is fine.
		*/
		void* producerMessage = NULL;
		void* consumerMessage = NULL;
		bool messageFlipRequested = false;

		void requestMessageFlip() {
			messageFlipRequested = true;
		}
	};

	Expander leftExpander;
	Expander rightExpander;

	struct BypassRoute {
		int inputId = -1;
		int outputId = -1;
	};
	std::vector<BypassRoute> bypassRoutes;

	/** Constructs a Module with no params, inputs, outputs, and lights. */
	Module();
	/** Use config() instead. */
	DEPRECATED Module(int numParams, int numInputs, int numOutputs, int numLights = 0) : Module() {
		config(numParams, numInputs, numOutputs, numLights);
	}
	virtual ~Module();

	/** Configures the number of Params, Outputs, Inputs, and Lights. */
	void config(int numParams, int numInputs, int numOutputs, int numLights = 0);

	/** Helper for creating a ParamQuantity and setting its properties.
	See ParamQuantity for documentation of arguments.
	*/
	template <class TParamQuantity = ParamQuantity>
	TParamQuantity* configParam(int paramId, float minValue, float maxValue, float defaultValue, std::string name = "", std::string unit = "", float displayBase = 0.f, float displayMultiplier = 1.f, float displayOffset = 0.f) {
		assert(paramId < (int) params.size() && paramId < (int) paramQuantities.size());
		if (paramQuantities[paramId])
			delete paramQuantities[paramId];

		TParamQuantity* q = new TParamQuantity;
		q->module = this;
		q->paramId = paramId;
		q->minValue = minValue;
		q->maxValue = maxValue;
		q->defaultValue = defaultValue;
		q->name = name;
		q->unit = unit;
		q->displayBase = displayBase;
		q->displayMultiplier = displayMultiplier;
		q->displayOffset = displayOffset;
		paramQuantities[paramId] = q;

		Param* p = &params[paramId];
		p->value = q->getDefaultValue();
		return q;
	}

	/** Helper for creating a SwitchQuantity and setting its label strings.
	See ParamQuantity and SwitchQuantity for documentation of arguments.
	*/
	template <class TSwitchQuantity = SwitchQuantity>
	TSwitchQuantity* configSwitch(int paramId, float minValue, float maxValue, float defaultValue, std::string name = "", std::vector<std::string> labels = {}) {
		TSwitchQuantity* sq = configParam<TSwitchQuantity>(paramId, minValue, maxValue, defaultValue, name);
		sq->labels = labels;
		return sq;
	}

	template <class TSwitchQuantity = SwitchQuantity>
	TSwitchQuantity* configButton(int paramId, std::string name = "") {
		TSwitchQuantity* sq = configParam<TSwitchQuantity>(paramId, 0.f, 1.f, 0.f, name);
		sq->randomizeEnabled = false;
		return sq;
	}

	/** Helper for creating a PortInfo for an input port and setting its properties.
	See PortInfo for documentation of arguments.
	*/
	template <class TPortInfo = PortInfo>
	TPortInfo* configInput(int portId, std::string name = "") {
		assert(portId < (int) inputs.size() && portId < (int) inputInfos.size());
		if (inputInfos[portId])
			delete inputInfos[portId];

		TPortInfo* info = new TPortInfo;
		info->module = this;
		info->type = Port::INPUT;
		info->portId = portId;
		info->name = name;
		inputInfos[portId] = info;
		return info;
	}

	/** Helper for creating a PortInfo for an output port and setting its properties.
	See PortInfo for documentation of arguments.
	*/
	template <class TPortInfo = PortInfo>
	TPortInfo* configOutput(int portId, std::string name = "") {
		assert(portId < (int) outputs.size() && portId < (int) outputInfos.size());
		if (outputInfos[portId])
			delete outputInfos[portId];

		TPortInfo* info = new TPortInfo;
		info->module = this;
		info->type = Port::OUTPUT;
		info->portId = portId;
		info->name = name;
		outputInfos[portId] = info;
		return info;
	}

	/** Helper for creating a LightInfo and setting its properties.
	For multi-colored lights, use the first lightId.
	See LightInfo for documentation of arguments.
	*/
	template <class TLightInfo = LightInfo>
	TLightInfo* configLight(int lightId, std::string name = "") {
		assert(lightId < (int) lights.size() && lightId < (int) lightInfos.size());
		if (lightInfos[lightId])
			delete lightInfos[lightId];

		TLightInfo* info = new TLightInfo;
		info->module = this;
		info->lightId = lightId;
		info->name = name;
		lightInfos[lightId] = info;
		return info;
	}

	/** Adds a direct route from an input to an output when the module is bypassed.
	*/
	void configBypass(int inputId, int outputId) {
		assert(inputId < (int) inputs.size());
		assert(outputId < (int) outputs.size());
		// Check that output is not yet routed
		for (BypassRoute& br : bypassRoutes) {
			assert(br.outputId != outputId);
		}

		BypassRoute br;
		br.inputId = inputId;
		br.outputId = outputId;
		bypassRoutes.push_back(br);
	}

	/** Creates and returns the module's patch storage directory path.
	Do not call this method in process() since filesystem operations block the audio thread.

	Throws an Exception if Module is not yet added to the Engine.
	Therefore, you may not call these methods in your Module constructor.
	Instead, load patch storage files in onAdd() and save them in onSave().

	Patch storage files of deleted modules are garbage collected when user saves the patch.
	To allow the Undo feature to restore patch storage if the module is accidentally deleted, it is recommended to not delete patch storage in onRemove().
	*/
	std::string createPatchStorageDirectory();
	std::string getPatchStorageDirectory();

	/** Getters for members */
	plugin::Model* getModel() {
		return model;
	}
	int64_t getId() {
		return id;
	}
	int getNumParams() {
		return params.size();
	}
	Param& getParam(int index) {
		return params[index];
	}
	int getNumInputs() {
		return inputs.size();
	}
	Input& getInput(int index) {
		return inputs[index];
	}
	int getNumOutputs() {
		return outputs.size();
	}
	Output& getOutput(int index) {
		return outputs[index];
	}
	int getNumLights() {
		return lights.size();
	}
	Light& getLight(int index) {
		return lights[index];
	}
	ParamQuantity* getParamQuantity(int index) {
		return paramQuantities[index];
	}
	PortInfo* getInputInfo(int index) {
		return inputInfos[index];
	}
	PortInfo* getOutputInfo(int index) {
		return outputInfos[index];
	}
	LightInfo* getLightInfo(int index) {
		return lightInfos[index];
	}
	Expander& getLeftExpander() {
		return leftExpander;
	}
	Expander& getRightExpander() {
		return rightExpander;
	}
	/** Returns the left Expander if `side` is false, and the right Expander if `side` is true. */
	Expander& getExpander(bool side) {
		return side ? rightExpander : leftExpander;
	}

	// Virtual methods

	struct ProcessArgs {
		/** The current sample rate in Hz. */
		float sampleRate;
		/** The timestep of process() in seconds.
		Defined by `1 / sampleRate`.
		*/
		float sampleTime;
		/** Number of audio samples since the Engine's first sample. */
		int64_t frame;
	};
	/** Advances the module by one audio sample.
	Override this method to read Inputs and Params and to write Outputs and Lights.
	*/
	virtual void process(const ProcessArgs& args) {
		step();
	}

	/** DEPRECATED. Override `process(const ProcessArgs& args)` instead. */
	virtual void step() {}

	/** Called instead of process() when Module is bypassed.
	Typically you do not need to override this. Use configBypass() instead.
	If you do override it, avoid reading param values, since the state of the module should have no effect on routing.
	*/
	virtual void processBypass(const ProcessArgs& args);

	/** Usually you should override dataToJson() instead.
	There are very few reasons you should override this (perhaps to lock a mutex while serialization is occurring).
	*/
	virtual json_t* toJson();
	/** This is virtual only for the purpose of unserializing legacy data when you could set properties of the `.modules[]` object itself.
	Normally you should override dataFromJson().
	Remember to call `Module::fromJson(rootJ)` within your overridden method.
	*/
	virtual void fromJson(json_t* rootJ);

	/** Serializes the "params" object. */
	virtual json_t* paramsToJson();
	virtual void paramsFromJson(json_t* rootJ);

	/** Override to store extra internal data in the "data" property of the module's JSON object. */
	virtual json_t* dataToJson() {
		return NULL;
	}
	/** Override to load internal data from the "data" property of the module's JSON object.
	Not called if "data" property is not present.
	*/
	virtual void dataFromJson(json_t* rootJ) {}

	///////////////////////
	// Events
	///////////////////////

	// All of these events are thread-safe with process().

	struct AddEvent {};
	/** Called after adding the module to the Engine.
	*/
	virtual void onAdd(const AddEvent& e) {
		// Call deprecated event method by default
		onAdd();
	}

	struct RemoveEvent {};
	/** Called before removing the module from the Engine.
	*/
	virtual void onRemove(const RemoveEvent& e) {
		// Call deprecated event method by default
		onRemove();
	}

	struct BypassEvent {};
	/** Called after bypassing the module.
	*/
	virtual void onBypass(const BypassEvent& e) {}

	struct UnBypassEvent {};
	/** Called after enabling the module.
	*/
	virtual void onUnBypass(const UnBypassEvent& e) {}

	struct PortChangeEvent {
		/** True if connecting, false if disconnecting. */
		bool connecting;
		/** Port::INPUT or Port::OUTPUT */
		Port::Type type;
		int portId;
	};
	/** Called after a cable connects to or disconnects from a port.
	This event is not called for output ports if a stackable cable was added/removed and did not change the port's connected state.
	*/
	virtual void onPortChange(const PortChangeEvent& e) {}

	struct SampleRateChangeEvent {
		float sampleRate;
		float sampleTime;
	};
	/** Called after the Engine sample rate changes.
	*/
	virtual void onSampleRateChange(const SampleRateChangeEvent& e) {
		// Call deprecated event method by default
		onSampleRateChange();
	}

	struct ExpanderChangeEvent {
		/** False for left, true for right. */
		bool side;
	};
	/** Called after an expander is added, removed, or changed on either the left or right side of the Module.
	*/
	virtual void onExpanderChange(const ExpanderChangeEvent& e) {}

	struct ResetEvent {};
	/** Called when the user resets (initializes) the module.
	The default implementation resets all parameters to their default value, so you must call `Module::onReset(e)` in your overridden method if you want to keep this behavior.
	*/
	virtual void onReset(const ResetEvent& e);

	struct RandomizeEvent {};
	/** Called when the user randomizes the module.
	The default implementation randomizes all parameters by default, so you must call `Module::onRandomize(e)` in your overridden method if you want to keep this behavior.
	*/
	virtual void onRandomize(const RandomizeEvent& e);

	struct SaveEvent {};
	/** Called when the user saves the patch to a file.
	If your module uses patch asset storage, make sure all files are saved in this event.
	*/
	virtual void onSave(const SaveEvent& e) {}

	struct SetMasterEvent {};
	virtual void onSetMaster(const SetMasterEvent& e) {}

	struct UnsetMasterEvent {};
	virtual void onUnsetMaster(const UnsetMasterEvent& e) {}

	/** DEPRECATED. Override `onAdd(e)` instead. */
	virtual void onAdd() {}
	/** DEPRECATED. Override `onRemove(e)` instead. */
	virtual void onRemove() {}
	/** DEPRECATED. Override `onReset(e)` instead. */
	virtual void onReset() {}
	/** DEPRECATED. Override `onRandomize(e)` instead. */
	virtual void onRandomize() {}
	/** DEPRECATED. Override `onSampleRateChange(e)` instead. */
	virtual void onSampleRateChange() {}

	bool isBypassed();
	INTERNAL void setBypassed(bool bypassed);
	INTERNAL const float* meterBuffer();
	INTERNAL int meterLength();
	INTERNAL int meterIndex();
	INTERNAL void doProcess(const ProcessArgs& args);
	INTERNAL static void jsonStripIds(json_t* rootJ);
};


} // namespace engine
} // namespace rack

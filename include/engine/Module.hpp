#pragma once
#include <common.hpp>
#include <string.hpp>
#include <plugin/Model.hpp>
#include <engine/Param.hpp>
#include <engine/Port.hpp>
#include <engine/Light.hpp>
#include <engine/ParamQuantity.hpp>
#include <engine/PortInfo.hpp>
#include <vector>
#include <jansson.h>


namespace rack {


namespace plugin {
struct Model;
}


namespace engine {


/** DSP processor instance for your module. */
struct Module {
	plugin::Model* model = NULL;	/** Unique ID for referring to the module in the engine.
	Assigned when added to the engine.
	*/
	int id = -1;

	/** Arrays of components.
	Initialized with config().
	*/
	std::vector<Param> params;
	std::vector<Input> inputs;
	std::vector<Output> outputs;
	std::vector<Light> lights;
	std::vector<ParamQuantity*> paramQuantities;
	std::vector<PortInfo*> inputInfos;
	std::vector<PortInfo*> outputInfos;

	/** Represents a message-passing channel for an adjacent module. */
	struct Expander {
		/** ID of the expander module, or -1 if nonexistent. */
		int moduleId = -1;
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
	};

	Expander leftExpander;
	Expander rightExpander;

	/** Seconds spent in the process() method, with exponential smoothing.
	Only written when CPU timing is enabled, since time measurement is expensive.
	*/
	float cpuTime = 0.f;
	/** Whether the Module is skipped from stepping by the engine.
	Module subclasses should not read/write this variable.
	*/
	bool disabled = false;

	/** Constructs a Module with no params, inputs, outputs, and lights. */
	Module();
	/** Use config() instead. */
	DEPRECATED Module(int numParams, int numInputs, int numOutputs, int numLights = 0) : Module() {
		config(numParams, numInputs, numOutputs, numLights);
	}
	virtual ~Module();

	/** Configures the number of Params, Outputs, Inputs, and Lights. */
	void config(int numParams, int numInputs, int numOutputs, int numLights = 0);

	template <class TParamQuantity = ParamQuantity>
	void configParam(int paramId, float minValue, float maxValue, float defaultValue, std::string label = "", std::string unit = "", float displayBase = 0.f, float displayMultiplier = 1.f, float displayOffset = 0.f) {
		assert(paramId < (int) params.size() && paramId < (int) paramQuantities.size());
		if (paramQuantities[paramId])
			delete paramQuantities[paramId];

		Param* p = &params[paramId];
		p->value = defaultValue;

		ParamQuantity* q = new TParamQuantity;
		q->module = this;
		q->paramId = paramId;
		q->minValue = minValue;
		q->maxValue = maxValue;
		q->defaultValue = defaultValue;
		if (label == "")
			q->label = string::f("Parameter %d", paramId + 1);
		else
			q->label = label;
		q->unit = unit;
		q->displayBase = displayBase;
		q->displayMultiplier = displayMultiplier;
		q->displayOffset = displayOffset;
		paramQuantities[paramId] = q;
	}

	void configInput(int portId, std::string label = "") {
		assert(portId < (int) inputs.size() && portId < (int) inputInfos.size());
		if (inputInfos[portId])
			delete inputInfos[portId];

		PortInfo* p = new PortInfo;
		if (label == "")
			p->label = string::f("Input %d", portId + 1);
		else
			p->label = label;
		inputInfos[portId] = p;
	}

	void configOutput(int portId, std::string label = "") {
		assert(portId < (int) outputs.size() && portId < (int) outputInfos.size());
		if (outputInfos[portId])
			delete outputInfos[portId];

		PortInfo* p = new PortInfo;
		if (label == "")
			p->label = string::f("Output %d", portId + 1);
		else
			p->label = label;
		outputInfos[portId] = p;
	}

	struct ProcessArgs {
		float sampleRate;
		float sampleTime;
	};
	/** Advances the module by one audio sample.
	Override this method to read Inputs and Params and to write Outputs and Lights.
	*/
	virtual void process(const ProcessArgs& args) {
		step();
	}
	/** DEPRECATED. Override `process(const ProcessArgs& args)` instead. */
	virtual void step() {}

	json_t* toJson();
	/** This is virtual only for the purpose of unserializing legacy data when you could set properties of the `.modules[]` object itself.
	Normally you should override dataFromJson().
	Remember to call `Module::fromJson(rootJ)` within your overridden method.
	*/
	virtual void fromJson(json_t* rootJ);

	/** Override to store extra internal data in the "data" property of the module's JSON object. */
	virtual json_t* dataToJson() {
		return NULL;
	}
	virtual void dataFromJson(json_t* root) {}

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
	/** Called before removing the module to the Engine.
	*/
	virtual void onRemove(const RemoveEvent& e) {
		// Call deprecated event method by default
		onRemove();
	}

	struct EnableEvent {};
	/** Called after enabling the module.
	*/
	virtual void onEnable(const EnableEvent& e) {}

	struct DisableEvent {};
	/** Called after disabling the module.
	*/
	virtual void onDisable(const DisableEvent& e) {}

	struct PortChangeEvent {
		/** True if connecting, false if disconnecting. */
		bool connecting;
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

	struct ExpanderChangeEvent {};
	/** Called after the Engine sample rate changes.
	*/
	virtual void onExpanderChange(const ExpanderChangeEvent& e) {}

	struct ResetEvent {};
	/** Called when the user resets (initializes) the module.
	The default implementation resets all parameters to their default value, so you must call `Module::onRandomize(e)` if you want to keep this behavior.
	*/
	virtual void onReset(const ResetEvent& e);

	struct RandomizeEvent {};
	/** Called when the user randomizes the module.
	The default implementation randomizes all parameters by default, so you must call `Module::onRandomize(e)` if you want to keep this behavior.
	*/
	virtual void onRandomize(const RandomizeEvent& e);

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
};


} // namespace engine
} // namespace rack

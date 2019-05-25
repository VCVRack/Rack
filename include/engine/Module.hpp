#pragma once
#include <common.hpp>
#include <string.hpp>
#include <plugin/Model.hpp>
#include <engine/Param.hpp>
#include <engine/Port.hpp>
#include <engine/Light.hpp>
#include <engine/ParamQuantity.hpp>
#include <vector>
#include <jansson.h>


namespace rack {


namespace plugin {
	struct Model;
}


namespace engine {


/** DSP processor instance for your module. */
struct Module {
	plugin::Model *model = NULL;	/** Unique ID for referring to the module in the engine.
	Assigned when added to the engine.
	*/
	int id = -1;

	/** Arrays of components.
	Initialized with config().
	*/
	std::vector<Param> params;
	std::vector<Output> outputs;
	std::vector<Input> inputs;
	std::vector<Light> lights;
	std::vector<ParamQuantity*> paramQuantities;

	/** Represents a message-passing channel for an adjacent module. */
	struct Expander {
		/** ID of the expander module, or -1 if nonexistent. */
		int moduleId = -1;
		/** Pointer to the expander Module, or NULL if nonexistent. */
		Module *module = NULL;
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
		void *producerMessage = NULL;
		void *consumerMessage = NULL;
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
	bool bypass = false;

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

		Param *p = &params[paramId];
		p->value = defaultValue;

		ParamQuantity *q = new TParamQuantity;
		q->module = this;
		q->paramId = paramId;
		q->minValue = minValue;
		q->maxValue = maxValue;
		q->defaultValue = defaultValue;
		if (!label.empty())
			q->label = label;
		else
			q->label = string::f("#%d", paramId + 1);
		q->unit = unit;
		q->displayBase = displayBase;
		q->displayMultiplier = displayMultiplier;
		q->displayOffset = displayOffset;
		paramQuantities[paramId] = q;
	}

	struct ProcessArgs {
		float sampleRate;
		float sampleTime;
	};

	/** Advances the module by one audio sample.
	Override this method to read Inputs and Params and to write Outputs and Lights.
	*/
	virtual void process(const ProcessArgs &args) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
		step();
#pragma GCC diagnostic pop
	}
	/** Override process(const ProcessArgs &args) instead. */
	DEPRECATED virtual void step() {}

	/** Called when the engine sample rate is changed. */
	virtual void onSampleRateChange() {}
	/** Called when user clicks Initialize in the module context menu. */
	virtual void onReset() {}
	/** Called when user clicks Randomize in the module context menu. */
	virtual void onRandomize() {}
	/** Called when the Module is added to the Engine */
	virtual void onAdd() {}
	/** Called when the Module is removed from the Engine */
	virtual void onRemove() {}

	json_t *toJson();
	void fromJson(json_t *rootJ);

	/** Override to store extra internal data in the "data" property of the module's JSON object. */
	virtual json_t *dataToJson() { return NULL; }
	virtual void dataFromJson(json_t *root) {}
};


} // namespace engine
} // namespace rack

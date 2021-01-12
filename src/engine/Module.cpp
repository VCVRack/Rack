#include <engine/Module.hpp>
#include <engine/Engine.hpp>
#include <plugin.hpp>
#include <system.hpp>
#include <settings.hpp>
#include <asset.hpp>
#include <context.hpp>


namespace rack {
namespace engine {



// Arbitrary prime number so it doesn't over- or under-estimate time of buffered processors.
static const int meterDivider = 1;
static const int meterBufferLength = 128;


struct Module::Internal {
	bool bypassed = false;

	int64_t meterLastBlock = 0;
	int meterSamples = 0;
	float meterTimeTotal = 0.f;

	float meterBuffer[meterBufferLength] = {};
	int meterIndex = 0;
};


Module::Module() {
	internal = new Internal;
}


Module::~Module() {
	for (ParamQuantity* paramQuantity : paramQuantities) {
		if (paramQuantity)
			delete paramQuantity;
	}
	for (PortInfo* inputInfo : inputInfos) {
		if (inputInfo)
			delete inputInfo;
	}
	for (PortInfo* outputInfo : outputInfos) {
		if (outputInfo)
			delete outputInfo;
	}
	for (LightInfo* lightInfo : lightInfos) {
		if (lightInfo)
			delete lightInfo;
	}
	delete internal;
}


void Module::config(int numParams, int numInputs, int numOutputs, int numLights) {
	// This method should only be called once.
	assert(params.empty() && inputs.empty() && outputs.empty() && lights.empty() && paramQuantities.empty());
	params.resize(numParams);
	inputs.resize(numInputs);
	outputs.resize(numOutputs);
	lights.resize(numLights);
	// Initialize paramQuantities
	paramQuantities.resize(numParams);
	for (int i = 0; i < numParams; i++) {
		configParam(i, 0.f, 1.f, 0.f);
	}
	// Initialize PortInfos
	inputInfos.resize(numInputs);
	for (int i = 0; i < numInputs; i++) {
		configInput(i);
	}
	outputInfos.resize(numOutputs);
	for (int i = 0; i < numOutputs; i++) {
		configOutput(i);
	}
	// Initialize LightInfos with null
	lightInfos.resize(numLights);
}


void Module::processBypass(const ProcessArgs& args) {
	for (BypassRoute& bypassRoute : bypassRoutes) {
		// Route input voltages to output
		Input& input = inputs[bypassRoute.inputId];
		Output& output = outputs[bypassRoute.outputId];
		int channels = input.getChannels();
		for (int c = 0; c < channels; c++) {
			float v = input.getVoltage(c);
			output.setVoltage(v, c);
		}
		output.setChannels(channels);
	}
}


json_t* Module::toJson() {
	json_t* rootJ = json_object();

	// id
	json_object_set_new(rootJ, "id", json_integer(id));

	// plugin
	json_object_set_new(rootJ, "plugin", json_string(model->plugin->slug.c_str()));

	// version
	json_object_set_new(rootJ, "version", json_string(model->plugin->version.c_str()));

	// model
	json_object_set_new(rootJ, "model", json_string(model->slug.c_str()));

	// params
	json_t* paramsJ = paramsToJson();
	if (paramsJ)
		json_object_set_new(rootJ, "params", paramsJ);

	// bypass
	if (internal->bypassed)
		json_object_set_new(rootJ, "bypass", json_boolean(true));

	// leftModuleId
	if (leftExpander.moduleId >= 0)
		json_object_set_new(rootJ, "leftModuleId", json_integer(leftExpander.moduleId));

	// rightModuleId
	if (rightExpander.moduleId >= 0)
		json_object_set_new(rootJ, "rightModuleId", json_integer(rightExpander.moduleId));

	// data
	json_t* dataJ = dataToJson();
	if (dataJ) {
		json_object_set_new(rootJ, "data", dataJ);
	}

	return rootJ;
}


void Module::fromJson(json_t* rootJ) {
	// Check if plugin and model are incorrect
	json_t* pluginJ = json_object_get(rootJ, "plugin");
	std::string pluginSlug;
	if (pluginJ) {
		pluginSlug = json_string_value(pluginJ);
		pluginSlug = plugin::normalizeSlug(pluginSlug);
		if (pluginSlug != model->plugin->slug) {
			WARN("Plugin %s does not match Module's plugin %s.", pluginSlug.c_str(), model->plugin->slug.c_str());
			return;
		}
	}

	json_t* modelJ = json_object_get(rootJ, "model");
	std::string modelSlug;
	if (modelJ) {
		modelSlug = json_string_value(modelJ);
		modelSlug = plugin::normalizeSlug(modelSlug);
		if (modelSlug != model->slug) {
			WARN("Model %s does not match Module's model %s.", modelSlug.c_str(), model->slug.c_str());
			return;
		}
	}

	// Check plugin version
	json_t* versionJ = json_object_get(rootJ, "version");
	if (versionJ) {
		std::string version = json_string_value(versionJ);
		if (version != model->plugin->version) {
			INFO("Patch created with %s v%s, currently using v%s.", pluginSlug.c_str(), version.c_str(), model->plugin->version.c_str());
		}
	}

	// id
	json_t* idJ = json_object_get(rootJ, "id");
	if (idJ)
		id = json_integer_value(idJ);

	// params
	json_t* paramsJ = json_object_get(rootJ, "params");
	if (paramsJ)
		paramsFromJson(paramsJ);

	// bypass
	json_t* bypassJ = json_object_get(rootJ, "bypass");
	// legacy "disabled" in v1
	if (!bypassJ)
		bypassJ = json_object_get(rootJ, "disabled");
	if (bypassJ)
		internal->bypassed = json_boolean_value(bypassJ);

	// leftModuleId
	json_t *leftModuleIdJ = json_object_get(rootJ, "leftModuleId");
	if (leftModuleIdJ)
		leftExpander.moduleId = json_integer_value(leftModuleIdJ);

	// rightModuleId
	json_t *rightModuleIdJ = json_object_get(rootJ, "rightModuleId");
	if (rightModuleIdJ)
		rightExpander.moduleId = json_integer_value(rightModuleIdJ);

	// data
	json_t* dataJ = json_object_get(rootJ, "data");
	if (dataJ)
		dataFromJson(dataJ);
}


json_t* Module::paramsToJson() {
	json_t* rootJ = json_array();
	for (size_t paramId = 0; paramId < paramQuantities.size(); paramId++) {
		// Don't serialize unbounded Params
		if (!paramQuantities[paramId]->isBounded())
			continue;

		json_t* paramJ = paramQuantities[paramId]->toJson();

		json_object_set_new(paramJ, "id", json_integer(paramId));

		json_array_append(rootJ, paramJ);
	}
	return rootJ;
}


void Module::paramsFromJson(json_t* rootJ) {
	size_t i;
	json_t* paramJ;
	json_array_foreach(rootJ, i, paramJ) {
		// Get paramId
		json_t* paramIdJ = json_object_get(paramJ, "id");
		// Legacy v0.6 to <v1
		if (!paramIdJ)
			paramIdJ = json_object_get(paramJ, "paramId");
		size_t paramId;
		if (paramIdJ)
			paramId = json_integer_value(paramIdJ);
		// Use index if all else fails
		else
			paramId = i;

		// Check ID bounds
		if (paramId >= paramQuantities.size())
			continue;

		// Check that the Param is bounded
		if (!paramQuantities[paramId]->isBounded())
			continue;

		json_t* valueJ = json_object_get(paramJ, "value");
		if (valueJ)
			paramQuantities[paramId]->setValue(json_number_value(valueJ));
	}
}


void Module::onReset(const ResetEvent& e) {
	// Reset all parameters
	for (ParamQuantity* pq : paramQuantities) {
		if (!pq->resetEnabled)
			continue;
		if (!pq->isBounded())
			continue;
		pq->reset();
	}
	// Call deprecated event
	onReset();
}


void Module::onRandomize(const RandomizeEvent& e) {
	// Randomize all parameters
	for (ParamQuantity* pq : paramQuantities) {
		if (!pq->randomizeEnabled)
			continue;
		if (!pq->isBounded())
			continue;
		pq->randomize();
	}
	// Call deprecated event
	onRandomize();
}


bool Module::isBypassed() {
	return internal->bypassed;
}


void Module::setBypassed(bool bypassed) {
	internal->bypassed = bypassed;
}


const float* Module::meterBuffer() {
	return internal->meterBuffer;
}


int Module::meterLength() {
	return meterBufferLength;
}


int Module::meterIndex() {
	return internal->meterIndex;
}


static void Port_step(Port* that, float deltaTime) {
	// Set plug lights
	if (that->channels == 0) {
		that->plugLights[0].setBrightness(0.f);
		that->plugLights[1].setBrightness(0.f);
		that->plugLights[2].setBrightness(0.f);
	}
	else if (that->channels == 1) {
		float v = that->getVoltage() / 10.f;
		that->plugLights[0].setSmoothBrightness(v, deltaTime);
		that->plugLights[1].setSmoothBrightness(-v, deltaTime);
		that->plugLights[2].setBrightness(0.f);
	}
	else {
		float v2 = 0.f;
		for (int c = 0; c < that->channels; c++) {
			v2 += std::pow(that->getVoltage(c), 2);
		}
		float v = std::sqrt(v2) / 10.f;
		that->plugLights[0].setBrightness(0.f);
		that->plugLights[1].setBrightness(0.f);
		that->plugLights[2].setSmoothBrightness(v, deltaTime);
	}
}


void Module::doProcess(const ProcessArgs& args) {
	// This global setting can change while the function is running, so use a local variable.
	bool meterEnabled = settings::cpuMeter && (args.frame % meterDivider == 0);

	// Start CPU timer
	double startTime;
	if (meterEnabled) {
		startTime = system::getTime();
	}

	// Step module
	if (!internal->bypassed)
		process(args);
	else
		processBypass(args);

	// Stop CPU timer
	if (meterEnabled) {
		double endTime = system::getTime();
		float duration = endTime - startTime;

		int64_t block = APP->engine->getBlock();
		if (block > internal->meterLastBlock) {
			// Push time to buffer
			if (internal->meterSamples > 0) {
				internal->meterBuffer[internal->meterIndex++] = internal->meterTimeTotal / internal->meterSamples;
				internal->meterIndex %= meterBufferLength;
			}
			// Reset total
			internal->meterSamples = 0;
			internal->meterTimeTotal = 0.f;
		}

		internal->meterLastBlock = block;
		internal->meterSamples++;
		internal->meterTimeTotal += duration;
	}

	// Iterate ports to step plug lights
	const int portDivider = 8;
	if (args.frame % portDivider == 0) {
		float portTime = args.sampleTime * portDivider;
		for (Input& input : inputs) {
			Port_step(&input, portTime);
		}
		for (Output& output : outputs) {
			Port_step(&output, portTime);
		}
	}
}


} // namespace engine
} // namespace rack

#pragma once

#include "bogaudio.hpp"

namespace bogaudio {

struct DisableOutputLimitModule : Module {
	bool _disableOutputLimit = false;

	DisableOutputLimitModule(int numParams, int numInputs, int numOutputs, int numLights = 0)
	: Module(numParams, numInputs, numOutputs, numLights)
	{
	}

	json_t* toJson() override;
	void fromJson(json_t* root) override;
};

struct DisableOutputLimitMenuItem : MenuItem {
	DisableOutputLimitModule* _module;

	DisableOutputLimitMenuItem(DisableOutputLimitModule* module, const char* label)
	: _module(module)
	{
		this->text = label;
	}

	void onAction(EventAction &e) override {
		_module->_disableOutputLimit = !_module->_disableOutputLimit;
	}

	void step() override {
		rightText = _module->_disableOutputLimit ? "✔" : "";
	}
};

struct DisableOutputLimitModuleWidget : ModuleWidget {
	DisableOutputLimitModuleWidget(Module* module) : ModuleWidget(module) {
	}

	void appendContextMenu(Menu* menu) override {
		DisableOutputLimitModule* dolm = dynamic_cast<DisableOutputLimitModule*>(module);
		assert(dolm);
		menu->addChild(new MenuLabel());
		menu->addChild(new DisableOutputLimitMenuItem(dolm, "Diable Output Limit"));
	}
};

} // namespace bogaudio

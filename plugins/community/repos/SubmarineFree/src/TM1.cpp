#include "SubmarineFree.hpp"
#include "torpedo.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_SubmarineFree {

struct TM_105;

struct TM_105InPort : Torpedo::RawInputPort {
	TM_105 *tmModule;
	TM_105InPort(TM_105 *module, unsigned int portNum) : RawInputPort((Module *)module, portNum) {tmModule = module;}
	void received(std::string appId, std::string message) override;
	void error(unsigned int errorType) override;
};

struct TM_Msg {
	std::string appId;
	std::string msg;
};

struct TM_105 : Module  {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		INPUT_1,
		INPUT_2,
		INPUT_3,
		INPUT_4,
		INPUT_5,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_TOR,
		NUM_OUTPUTS
	};
	enum LightIds {
		LIGHT_Q_1,
		LIGHT_Q_2,
		LIGHT_Q_3,
		LIGHT_Q_4,
		LIGHT_Q_5,
		LIGHT_M_1,
		LIGHT_M_2,
		LIGHT_M_3,
		LIGHT_M_4,
		LIGHT_M_5,
		LIGHT_E_1,
		LIGHT_E_2,
		LIGHT_E_3,
		LIGHT_E_4,
		LIGHT_E_5,
		NUM_LIGHTS
	};

	std::vector<TM_Msg> queue;
	unsigned int count = 0;
	unsigned int index = 0;
	TM_105InPort inPort1 = TM_105InPort(this, INPUT_1);
	TM_105InPort inPort2 = TM_105InPort(this, INPUT_2);
	TM_105InPort inPort3 = TM_105InPort(this, INPUT_3);
	TM_105InPort inPort4 = TM_105InPort(this, INPUT_4);
	TM_105InPort inPort5 = TM_105InPort(this, INPUT_5);
	PulseGenerator msgPulses[5];
	PulseGenerator errPulses[5];
	Torpedo::RawOutputPort outPort = Torpedo::RawOutputPort(this, OUTPUT_TOR);	
	TM_105() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		for (unsigned int i = 0; i < 5; i++) {
			queue.push_back(TM_Msg());
		}
	}
	void step() override;
};

void TM_105InPort::received(std::string appId, std::string msg) {
	if (tmModule->count >= 5) {
		return;
	}
	unsigned int newPos = (tmModule->index + tmModule->count) % 5; 
	tmModule->queue[newPos].appId.assign(appId);
	tmModule->queue[newPos].msg.assign(msg);
	tmModule->count++;
	tmModule->msgPulses[_portNum - TM_105::INPUT_1].trigger(0.1f);
}

void TM_105InPort::error(unsigned int errorType) {
	tmModule->errPulses[_portNum - TM_105::INPUT_1].trigger(0.1f);
}

void TM_105::step() {
	inPort1.process();
	inPort2.process();
	inPort3.process();
	inPort4.process();
	inPort5.process();
	if (!outPort.isBusy()) {
		if (count) {
			unsigned int sendPos = (count + index) % 5;
			outPort.send(queue[sendPos].appId, queue[sendPos].msg);
			index++;
			index %= 5;
			count--;
		}
	}
	for(unsigned int i = 0; i < 5; i++) {
		lights[LIGHT_Q_1 + i].value = count > i;
		lights[LIGHT_M_1 + i].value = msgPulses[i].process(engineGetSampleTime()); 
		lights[LIGHT_E_1 + i].value = errPulses[i].process(engineGetSampleTime()); 
	}
	outPort.process();
}

struct TM105 : ModuleWidget {
	TM105(TM_105 *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/TM-105.svg")));
		for (unsigned int i = 0; i < 5; i++) {
			addInput(Port::create<BlackPort>(Vec(2.5,29 + 32 * i), Port::INPUT, module, TM_105::INPUT_1 + i));
			addChild(ModuleLightWidget::create<TinyLight<BlueLight>>(Vec(2.5, 53 + 32 * i), module, TM_105::LIGHT_M_1 + i));
			addChild(ModuleLightWidget::create<TinyLight<RedLight>>(Vec(24.5, 53 + 32 * i), module, TM_105::LIGHT_E_1 + i));
		}
		addChild(ModuleLightWidget::create<TinyLight<BlueLight>>(Vec(13.5, 260), module, TM_105::LIGHT_Q_1));
		addChild(ModuleLightWidget::create<TinyLight<BlueLight>>(Vec(13.5, 265), module, TM_105::LIGHT_Q_2));
		addChild(ModuleLightWidget::create<TinyLight<BlueLight>>(Vec(13.5, 270), module, TM_105::LIGHT_Q_3));
		addChild(ModuleLightWidget::create<TinyLight<RedLight>>(Vec(13.5, 275), module, TM_105::LIGHT_Q_4));
		addChild(ModuleLightWidget::create<TinyLight<RedLight>>(Vec(13.5, 280), module, TM_105::LIGHT_Q_5));
		addOutput(Port::create<BlackPort>(Vec(2.5,232), Port::OUTPUT, module, TM_105::OUTPUT_TOR));
	}
};

} // namespace rack_plugin_SubmarineFree

using namespace rack_plugin_SubmarineFree;

RACK_PLUGIN_MODEL_INIT(SubmarineFree, TM105) {
   Model *modelTM105 = Model::create<TM_105, TM105>("Submarine (Free)", "TM-105", "TM-105 Torpedo Message Merge", UTILITY_TAG);
   return modelTM105;
}

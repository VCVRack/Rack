#include "Core.hpp"
#include "midi.hpp"


struct QuadMIDIToCVInterface : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		RECT17025_OUTPUT,
		RECT17027_OUTPUT,
		RECT17029_OUTPUT,
		RECT17031_OUTPUT,
		RECT17033_OUTPUT,
		RECT17035_OUTPUT,
		RECT17037_OUTPUT,
		RECT17039_OUTPUT,
		RECT17041_OUTPUT,
		RECT17043_OUTPUT,
		RECT17045_OUTPUT,
		RECT17047_OUTPUT,
		RECT17049_OUTPUT,
		RECT17051_OUTPUT,
		RECT17053_OUTPUT,
		RECT17055_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	MidiInputQueue midiInput;

	QuadMIDIToCVInterface() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

	void step() override {}
};


struct QuadMIDIToCVInterfaceWidget : ModuleWidget {
	QuadMIDIToCVInterfaceWidget(QuadMIDIToCVInterface *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetGlobal("res/Core/QuadMIDIToCVInterface.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addOutput(Port::create<PJ301MPort>(mm2px(Vec(3.894335, 60.144478)), Port::OUTPUT, module, QuadMIDIToCVInterface::RECT17025_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(15.494659, 60.144478)), Port::OUTPUT, module, QuadMIDIToCVInterface::RECT17027_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(27.094986, 60.144478)), Port::OUTPUT, module, QuadMIDIToCVInterface::RECT17029_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(38.693935, 60.144478)), Port::OUTPUT, module, QuadMIDIToCVInterface::RECT17031_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(3.894335, 76.144882)), Port::OUTPUT, module, QuadMIDIToCVInterface::RECT17033_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(15.494659, 76.144882)), Port::OUTPUT, module, QuadMIDIToCVInterface::RECT17035_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(27.094986, 76.144882)), Port::OUTPUT, module, QuadMIDIToCVInterface::RECT17037_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(38.693935, 76.144882)), Port::OUTPUT, module, QuadMIDIToCVInterface::RECT17039_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(3.894335, 92.143906)), Port::OUTPUT, module, QuadMIDIToCVInterface::RECT17041_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(15.494659, 92.143906)), Port::OUTPUT, module, QuadMIDIToCVInterface::RECT17043_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(27.094986, 92.143906)), Port::OUTPUT, module, QuadMIDIToCVInterface::RECT17045_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(38.693935, 92.143906)), Port::OUTPUT, module, QuadMIDIToCVInterface::RECT17047_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(3.894335, 108.1443)), Port::OUTPUT, module, QuadMIDIToCVInterface::RECT17049_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(15.494659, 108.1443)), Port::OUTPUT, module, QuadMIDIToCVInterface::RECT17051_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(27.094986, 108.1443)), Port::OUTPUT, module, QuadMIDIToCVInterface::RECT17053_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(38.693935, 108.1443)), Port::OUTPUT, module, QuadMIDIToCVInterface::RECT17055_OUTPUT));

		MidiWidget *midiWidget = Widget::create<MidiWidget>(mm2px(Vec(3.4009969, 14.837336)));
		midiWidget->box.size = mm2px(Vec(44, 28));
		midiWidget->midiIO = &module->midiInput;
		addChild(midiWidget);
	}
};


Model *modelQuadMIDIToCVInterface = Model::create<QuadMIDIToCVInterface, QuadMIDIToCVInterfaceWidget>("Core", "QuadMIDIToCVInterface", "MIDI-4", MIDI_TAG, EXTERNAL_TAG, QUAD_TAG);


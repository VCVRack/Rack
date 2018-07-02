#include "HetrickCV.hpp"

namespace rack_plugin_HetrickCV {

#ifdef USE_VST2
#define plugin "HetrickCV"
#endif // USE_VST2

struct FlipPan : Module
{
	enum ParamIds
	{
		AMOUNT_PARAM,
        SCALE_PARAM,
        STYLE_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
        LEFT_INPUT,
        RIGHT_INPUT,
        AMOUNT_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
        LEFT_OUTPUT,
        RIGHT_OUTPUT,
		NUM_OUTPUTS
	};

	FlipPan() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS)
	{

	}

    void step() override;

    float paraPanShape(const float _input) const
    {
        return (4.0f - _input) * _input * 0.3333333f;
    }

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - reset, randomize: implements special behavior when user clicks these from the context menu
};


void FlipPan::step()
{
	float inL = inputs[LEFT_INPUT].value;
	float inR = inputs[RIGHT_INPUT].value;

    bool linear = (params[STYLE_PARAM].value == 0.0f);

    float pan = params[AMOUNT_PARAM].value + (inputs[AMOUNT_INPUT].value * params[SCALE_PARAM].value);
    pan = clampf(pan, 0.0f, 5.0f) * 0.2f;

    if(linear)
    {
        outputs[LEFT_OUTPUT].value = LERP(pan, inR, inL);
        outputs[RIGHT_OUTPUT].value = LERP(pan, inL, inR);
    }
    else
    {
        pan = (pan * 2.0f) - 1.0f;
        const float panL = paraPanShape(1.0f - pan);
        const float panR = paraPanShape(1.0f + pan);

        outputs[LEFT_OUTPUT].value = (inL * panL) + (inR * panR);
        outputs[RIGHT_OUTPUT].value = (inL * panR) + (inR * panL);
    }
}

struct CKSSRot : SVGSwitch, ToggleSwitch {
	CKSSRot() {
		addFrame(SVG::load(assetPlugin(plugin, "res/CKSS_rot_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/CKSS_rot_1.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};


struct FlipPanWidget : ModuleWidget { FlipPanWidget(FlipPan *module); };

FlipPanWidget::FlipPanWidget(FlipPan *module) : ModuleWidget(module)
{
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		auto *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/FlipPan.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

	//////PARAMS//////
	addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(27, 62), module, FlipPan::AMOUNT_PARAM, 0.0, 5.0, 2.5));
    addParam(ParamWidget::create<Trimpot>(Vec(36, 112), module, FlipPan::SCALE_PARAM, -1.0, 1.0, 1.0));
    addParam(ParamWidget::create<CKSSRot>(Vec(35, 200), module, FlipPan::STYLE_PARAM, 0.0, 1.0, 0.0));

	//////INPUTS//////
    addInput(Port::create<PJ301MPort>(Vec(10, 235), Port::INPUT, module, FlipPan::LEFT_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(55, 235), Port::INPUT, module, FlipPan::RIGHT_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(33, 145), Port::INPUT, module, FlipPan::AMOUNT_INPUT));

	//////OUTPUTS//////
    addOutput(Port::create<PJ301MPort>(Vec(10, 285), Port::OUTPUT, module, FlipPan::LEFT_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(55, 285), Port::OUTPUT, module, FlipPan::RIGHT_OUTPUT));
}

} // namespace rack_plugin_HetrickCV

using namespace rack_plugin_HetrickCV;

RACK_PLUGIN_MODEL_INIT(HetrickCV, FlipPan) {
   Model *modelFlipPan = Model::create<FlipPan, FlipPanWidget>("HetrickCV", "FlipPan", "Flip Pan", PANNING_TAG);
   return modelFlipPan;
}

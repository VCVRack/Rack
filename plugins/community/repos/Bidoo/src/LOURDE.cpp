#include "Bidoo.hpp"
#include "BidooComponents.hpp"
#include "dsp/samplerate.hpp"

using namespace std;

namespace rack_plugin_Bidoo {

struct LOURDE : Module {
	enum ParamIds {
		WEIGHT1,
		WEIGHT2,
		WEIGHT3,
		OUTFLOOR,
		NUM_PARAMS
	};
	enum InputIds {
		IN1,
		IN2,
		IN3,
    INWEIGHT1,
		INWEIGHT2,
		INWEIGHT3,
    INFLOOR,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};


	LOURDE() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {

	}

	void step() override;
};


void LOURDE::step() {
  float sum = clamp(params[WEIGHT1].value+inputs[INWEIGHT1].value,-5.0f,5.0f)*inputs[IN1].value + clamp(params[WEIGHT2].value+inputs[INWEIGHT2].value,-5.0f,5.0f)*inputs[IN2].value + clamp(params[WEIGHT3].value+inputs[INWEIGHT3].value,-5.0f,5.0f)*inputs[IN3].value;
	outputs[OUT].value = sum >= clamp(params[OUTFLOOR].value+inputs[INFLOOR].value,-10.0f,10.0f) ? 10.0f : 0.0f;
}

struct LabelDisplayWidget : TransparentWidget {
  float *value;
  std::shared_ptr<Font> font;

  LabelDisplayWidget() {
    font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
  };

  void draw(NVGcontext *vg) override
  {
      // text
    nvgFontSize(vg, 18);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 2.0);

		char display[128];
		snprintf(display, sizeof(display), "%2.2f", *value);
		nvgFontSize(vg, 12.0f);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -2.0f);
		nvgFillColor(vg, BLUE_BIDOO);
		nvgTextAlign(vg, NVG_ALIGN_CENTER);
		nvgRotate(vg,-45.0f);
		nvgText(vg, 0.0f, 0.0f, display, NULL);

  }
};

struct LOURDEWidget : ModuleWidget {
	LOURDEWidget(LOURDE *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/LOURDE.svg")));

  	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
  	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
  	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

  	addInput(Port::create<PJ301MPort>(Vec(25.5,85), Port::INPUT, module, LOURDE::IN1));
    addInput(Port::create<PJ301MPort>(Vec(25.5,155), Port::INPUT, module, LOURDE::IN2));
    addInput(Port::create<PJ301MPort>(Vec(25.5,225), Port::INPUT, module, LOURDE::IN3));

    addInput(Port::create<TinyPJ301MPort>(Vec(56.0f, 57.0f), Port::INPUT, module, LOURDE::INWEIGHT1));
    addInput(Port::create<TinyPJ301MPort>(Vec(56.0f, 127.0f), Port::INPUT, module, LOURDE::INWEIGHT2));
    addInput(Port::create<TinyPJ301MPort>(Vec(56.0f, 197.0f), Port::INPUT, module, LOURDE::INWEIGHT3));

    addParam(ParamWidget::create<BidooBlueKnob>(Vec(22.5,50), module, LOURDE::WEIGHT1, -5.0f, 5.0f, 0.0f));
    addParam(ParamWidget::create<BidooBlueKnob>(Vec(22.5,120), module, LOURDE::WEIGHT2, -5.0f, 5.0f, 0.0f));
    addParam(ParamWidget::create<BidooBlueKnob>(Vec(22.5,190), module, LOURDE::WEIGHT3, -5.0f, 5.0f, 0.0f));

		LabelDisplayWidget *displayW1 = new LabelDisplayWidget();
		displayW1->box.pos = Vec(20,55);
		displayW1->value = &module->params[LOURDE::WEIGHT1].value;
		addChild(displayW1);

		LabelDisplayWidget *displayW2 = new LabelDisplayWidget();
		displayW2->box.pos = Vec(20,125);
		displayW2->value = &module->params[LOURDE::WEIGHT2].value;
		addChild(displayW2);

		LabelDisplayWidget *displayW3 = new LabelDisplayWidget();
		displayW3->box.pos = Vec(20,195);
		displayW3->value = &module->params[LOURDE::WEIGHT3].value;
		addChild(displayW3);

    addParam(ParamWidget::create<BidooBlueKnob>(Vec(22.5,270), module, LOURDE::OUTFLOOR, -10.0f, 10.0f, 0.0f));

		LabelDisplayWidget *displayOF = new LabelDisplayWidget();
		displayOF->box.pos = Vec(20,275);
		displayOF->value = &module->params[LOURDE::OUTFLOOR].value;
		addChild(displayOF);

    addInput(Port::create<TinyPJ301MPort>(Vec(56.0f, 277.0f), Port::INPUT, module, LOURDE::INFLOOR));

  	addOutput(Port::create<PJ301MPort>(Vec(25.5,320), Port::OUTPUT, module, LOURDE::OUT));
  }
};

} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, LOURDE) {
   Model *modelLOURDE= Model::create<LOURDE, LOURDEWidget>("Bidoo", "LoURdE", "LoURdE gate", LOGIC_TAG);
   return modelLOURDE;
}

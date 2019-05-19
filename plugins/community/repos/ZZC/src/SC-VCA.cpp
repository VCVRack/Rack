#include "ZZC.hpp"

namespace rack_plugin_ZZC {

struct SCVCA : Module {
  enum ParamIds {
    GAIN_PARAM,
    CLIP_PARAM,
    CLIP_SOFTNESS_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    CV_INPUT,
    SIG1_INPUT,
    SIG2_INPUT,
    GAIN_INPUT,
    CLIP_INPUT,
    CLIP_SOFTNESS_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    SIG1_OUTPUT,
    SIG2_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    CLIPPING_POS_LIGHT,
    CLIPPING_NEG_LIGHT,
    NUM_LIGHTS
  };

  inline float SoftLimit(float x) {
    return x * (27.0f + x * x) / (27.0f + 9.0f * x * x);
  }

  inline float SoftClip(float x) {
    return x > 3.0f ? 1.0f : SoftLimit(x);
  }

  inline void SoftClipTo(float x, float ceiling, float softness, Output &out) {
    float ws_range = ceiling * softness;
    float clean_range = ceiling - ws_range;
    float abso = fabsf(x);

    if (abso > clean_range) {
      out.value = copysignf(clean_range + SoftClip((abso - clean_range) / ws_range) * ws_range, x);
    } else {
      out.value = x;
    }
  }

  inline void HardClipTo(float x, float ceiling, Output &out) {
    out.value = clamp(x, -ceiling, ceiling);
  }

  SCVCA() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
  void step() override;
};


void SCVCA::step() {
  float gain = params[GAIN_PARAM].value;
  float clip = params[CLIP_PARAM].value;
  float softness = params[CLIP_SOFTNESS_PARAM].value;

  if (inputs[GAIN_INPUT].active) { gain = gain * inputs[GAIN_INPUT].value / 10.0f; }
  if (inputs[CLIP_INPUT].active) { clip = clip * clamp(inputs[CLIP_INPUT].value, 0.0f, 10.0f) / 10.0f; }
  if (inputs[CLIP_SOFTNESS_INPUT].active) { softness = softness * clamp(inputs[CLIP_SOFTNESS_INPUT].value, 0.0f, 10.0f) / 10.0f; }

  float gained_1 = inputs[SIG1_INPUT].value * gain;
  float gained_2 = inputs[SIG2_INPUT].value * gain;
  SoftClipTo(gained_1, clip, softness, outputs[SIG1_OUTPUT]);
  SoftClipTo(gained_2, clip, softness, outputs[SIG2_OUTPUT]);

  lights[CLIPPING_POS_LIGHT].setBrightnessSmooth(fmaxf(
    fminf(1.0f, gained_1 - outputs[SIG1_OUTPUT].value),
    fminf(1.0f, gained_2 - outputs[SIG2_OUTPUT].value)
  ));
  lights[CLIPPING_NEG_LIGHT].setBrightnessSmooth(fmaxf(
    fmaxf(-1.0f, -(gained_1 - outputs[SIG1_OUTPUT].value)),
    fmaxf(-1.0f, -(gained_2 - outputs[SIG2_OUTPUT].value))
  ));
}


struct SCVCAWidget : ModuleWidget {
  SCVCAWidget(SCVCA *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/panels/SC-VCA.svg")));

    addParam(ParamWidget::create<ZZC_BigKnob>(Vec(4, 74.7), module, SCVCA::GAIN_PARAM, 0.0f, 2.0f, 1.0f));
    addParam(ParamWidget::create<ZZC_BigKnobInner>(Vec(24, 94.7), module, SCVCA::CLIP_PARAM, 0.0f, 10.0f, 5.0f));
    addParam(ParamWidget::create<ZZC_Knob25>(Vec(42.5, 175.7), module, SCVCA::CLIP_SOFTNESS_PARAM, 0.0f, 1.0f, 0.5f));

    addInput(Port::create<ZZC_PJ_Port>(Vec(8, 221), Port::INPUT, module, SCVCA::GAIN_INPUT));
    addInput(Port::create<ZZC_PJ_Port>(Vec(42.5, 221), Port::INPUT, module, SCVCA::CLIP_INPUT));
    addInput(Port::create<ZZC_PJ_Port>(Vec(8, 176), Port::INPUT, module, SCVCA::CLIP_SOFTNESS_INPUT));

    addInput(Port::create<ZZC_PJ_Port>(Vec(8, 275), Port::INPUT, module, SCVCA::SIG1_INPUT));
    addInput(Port::create<ZZC_PJ_Port>(Vec(42.5, 275), Port::INPUT, module, SCVCA::SIG2_INPUT));
    addOutput(Port::create<ZZC_PJ_Port>(Vec(8, 319.75), Port::OUTPUT, module, SCVCA::SIG1_OUTPUT));
    addOutput(Port::create<ZZC_PJ_Port>(Vec(42.5, 319.75), Port::OUTPUT, module, SCVCA::SIG2_OUTPUT));

    addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(34.2f, 43.9f), module, SCVCA::CLIPPING_POS_LIGHT));

    addChild(Widget::create<ZZC_Screw>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ZZC_Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  }
};

} // namespace rack_plugin_ZZC

using namespace rack_plugin_ZZC;

RACK_PLUGIN_MODEL_INIT(ZZC, SCVCA) {
   Model *modelSCVCA = Model::create<SCVCA, SCVCAWidget>("ZZC", "SC-VCA", "Soft Clipping VCA", AMPLIFIER_TAG, LIMITER_TAG, WAVESHAPER_TAG);
   return modelSCVCA;
}

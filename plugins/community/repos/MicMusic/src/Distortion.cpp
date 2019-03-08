#include "Distortion.hpp"

#include <cmath>

namespace rack_plugin_MicMusic {

namespace math {
float clamp(float value, float min, float max) {
#define sMIN(a,b) (((a)>(b))?(b):(a))
#define sMAX(a,b) (((a)>(b))?(a):(b))
    return sMAX(sMIN(value, max), min);
#undef sMIN
#undef sMAX
}
}

Distortion::Distortion() 
    : Module((int) Params::COUNT, (int) Inputs::COUNT, (int) Outputs::COUNT, (int) Lights::COUNT)
{}

void Distortion::step() {
    float input = inputs[(int) Inputs::SIGNAL].value;
    float high_cv = inputs[(int) Inputs::HIGH].value * params[(int) Params::HIGH_CV].value;
    float high_value = params[(int) Params::HIGH].value + high_cv;
    float low_cv = inputs[(int) Inputs::LOW].value * params[(int) Params::LOW_CV].value;
    float low_value = params[(int) Params::LOW].value + low_cv;
	
    //_debugOut << params[(int) Inputs::HIGH].value;
    outputs[(int) Outputs::SIGNAL].value = math::clamp(input, low_value, high_value);
}

struct DistortionWidget : ModuleWidget {
	DistortionWidget(Distortion *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Distortion.svg")));

		addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(ParamWidget::create<RoundBlackKnob>(Vec(50, 86), module, (int) Distortion::Params::HIGH, -10.f, 10.f, 10.f));
		addParam(ParamWidget::create<RoundBlackKnob>(Vec(50, 188), module, (int) Distortion::Params::HIGH_CV, 0.f, 1.f, 0.f));
        addInput(Port::create<PJ301MPort>(Vec(52, 149), Port::INPUT, module, (int) Distortion::Inputs::HIGH));
		
        addParam(ParamWidget::create<RoundBlackKnob>(Vec(10, 86), module, (int) Distortion::Params::LOW, -10.f, 10.f, -10.f));
        addParam(ParamWidget::create<RoundBlackKnob>(Vec(10, 188), module, (int) Distortion::Params::LOW_CV, 0.f, 1.f, 0.f));
        addInput(Port::create<PJ301MPort>(Vec(12, 149), Port::INPUT, module, (int) Distortion::Inputs::LOW));

        addInput(Port::create<PJ301MPort>(Vec(12, 330), Port::INPUT, module, (int) Distortion::Inputs::SIGNAL));
		addOutput(Port::create<PJ301MPort>(Vec(52, 330), Port::OUTPUT, module,(int)  Distortion::Outputs::SIGNAL));
	}
};

} // namespace rack_plugin_MicMusic

using namespace rack_plugin_MicMusic;

RACK_PLUGIN_MODEL_INIT(MicMusic, Distortion) {
   Model *distortionModule = Model::create<Distortion, DistortionWidget>("MicMusic", "Distortion", "Distortion - CuTeR", DISTORTION_TAG);
   return distortionModule;
}

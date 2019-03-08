#include "LabSeven.hpp"

#include "LabSeven_3340_VCO.h"
#include <time.h>

#include <fstream>
using namespace std;

//TODO:
//fine tune vco parameters to match my own synth

namespace rack_plugin_LabSeven {

struct LS3340VCO : Module
{
    enum ParamIds
    {
		PARAM_MOD,
		PARAM_RANGE,
		PARAM_PULSEWIDTH,
		PARAM_PWMSOURCE,
		PARAM_VOLSQUARE,
		PARAM_VOLSAW,
        PARAM_VOLTRIANGLE,
		PARAM_VOLSUBOSC,
		PARAM_SUBOSCRATIO,
		PARAM_VOLNOISE,
		NUM_PARAMS
	};
    enum InputIds
    {
		IN_PITCH,
		IN_MOD,
		IN_RANGE,
		IN_LFO,
		IN_ENV,
		IN_SUBOSCSELECT,
		NUM_INPUTS
	};
    enum OutputIds
    {
		OUT_SQUARE,
		OUT_SAW,
		OUT_SUB,
		OUT_TRIANGLE,
		OUT_MIX,
		OUT_NOISE,
		NUM_OUTPUTS
	};
    enum LightIds
    {
		NUM_LIGHTS
	};

    //VCO instance and VCO output frame
    LabSeven::LS3340::TLS3340VCO vco;
    LabSeven::LS3340::TLS3340VCOFrame nextFrame;

    LS3340VCO() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

    float sampleTimeCurrent = 0.0;
    float sampleRateCurrent = 0.0;

    double pitch,maxPitch,rangeFactor;
	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};


void LS3340VCO::step()
{
    //update external sample rate if neccessary
    if (sampleTimeCurrent != engineGetSampleTime())
    {
        sampleTimeCurrent = engineGetSampleTime();
        sampleRateCurrent = 1.0/sampleTimeCurrent;
        vco.setSamplerateExternal(sampleRateCurrent);
		
		maxPitch = sampleRateCurrent*0.45;
    	if (maxPitch > 40000) maxPitch = 40000; //high value so that suboscillator can go up to 10kHz
    }

	//get pitch and pitch mod input
    pitch = inputs[IN_PITCH].value;
    pitch +=  pow(2,2.25*0.2*inputs[IN_MOD].value * params[PARAM_MOD].value);

    //set rangeFactor
	rangeFactor = params[PARAM_RANGE].value;
	switch ((int)rangeFactor)
	{
		case 0: rangeFactor = 0.5; break;
		case 1: rangeFactor = 1.0; break;
		case 2: rangeFactor = 2.0; break;
		case 3: rangeFactor = 4.0; break;
		default: rangeFactor = 1.0;
	}

    //range modulation
    if (inputs[IN_RANGE].active)
    {
        int rangeSelect = (int)round(inputs[IN_RANGE].value*0.6);
        switch (rangeSelect)
        {
            case -3: rangeFactor /= 8.0; break;
            case -2: rangeFactor /= 4.0; break;
            case -1: rangeFactor /= 2.0; break;
            case  0: break; //no change
            case  1: rangeFactor *= 2.0; break;
            case  2: rangeFactor *= 4.0; break;
            case  3: rangeFactor *= 8.0; break;
        }
        if (rangeFactor > 16.0) rangeFactor = 16.0;
    }
	
    //set pitch
    //TODO: Clean up this paragraph!!!
	pitch = 261.626f * pow(2.0, pitch) * rangeFactor;
    pitch = clamp(pitch, 0.01f, maxPitch);
    //simulate the jitter observed in the hardware synth
    //use values > 0.02 for dirtier sound
    pitch *= 1.0+0.02*((double) rand() / (RAND_MAX)-0.5);
    vco.setFrequency(pitch);

	
	//update suboscillator
    switch((int)inputs[IN_SUBOSCSELECT].value)
    {
        case 1: vco.setSuboscillatorMode(0); break;
        case 2: vco.setSuboscillatorMode(1); break;
        case 3: vco.setSuboscillatorMode(2); break;
        default: vco.setSuboscillatorMode((unsigned short)params[PARAM_SUBOSCRATIO].value);
    }

	//pulse width modulation
    switch ((int)params[PARAM_PWMSOURCE].value)
	{
        //LFO PWM requires values between -0.4 and 0.4; SH does PWM between 10% and 90% pulse width
        case 2:  vco.setPwmCoefficient(-2.0*params[PARAM_PULSEWIDTH].value*0.4*0.2*inputs[IN_LFO].value); break; //bipolar, -5V - +5V
        case 1:  vco.setPwmCoefficient(-0.8*params[PARAM_PULSEWIDTH].value); break;
        case 0:  vco.setPwmCoefficient(-2.0*params[PARAM_PULSEWIDTH].value*0.4*0.1*inputs[IN_ENV].value); break; //unipolar, 0V - +10v
        default: vco.setPwmCoefficient(-0.8*params[PARAM_PULSEWIDTH].value);
	}
	
    //get next frame and put it out
    double scaling = 8.0;

    //TODO: PROPER (FREQUENCY DEPENDENT) AMPLITUDE SCALING FOR SAW AND TRIANGLE
    //TODO: PWM FOR TRIANGLE

    //calculate next frame

    if (this->sampleRateCurrent != 192000)
    {
        //TODO: Add a 'standard/high' quality switch to GUI
        //and choose interpolation method accordingly
        if (true)
        {
            vco.getNextFrameAtExternalSampleRateSinc(&nextFrame);
        }
        else
        {
            //currently next neighbour interpolation, not used!
            //TODO: Add quality switch (low/medium/high) to select
            //nn (has its own sound), cubic or sinc interpolation
            vco.getNextFrameAtExternalSampleRateCubic(&nextFrame);
        }
    }
    else //no interpolation required if internal sample rate == external sample rate == 192kHz
    {
        vco.getNextBlock(&nextFrame,1);
    }

    //TODO: Activate/deactivate interpolation if outs are not active
    outputs[OUT_SQUARE].value     = scaling * nextFrame.square;
    outputs[OUT_SAW].value        = scaling * nextFrame.sawtooth;
    outputs[OUT_SUB].value        = scaling * nextFrame.subosc;
    outputs[OUT_TRIANGLE].value   = scaling * nextFrame.triangle;
    outputs[OUT_NOISE].value      =          6.0* nextFrame.noise;
    outputs[OUT_MIX].value        = 0.4*(outputs[OUT_SQUARE].value   * params[PARAM_VOLSQUARE].value +
                                         outputs[OUT_SAW].value      * params[PARAM_VOLSAW].value +
                                         outputs[OUT_SUB].value      * params[PARAM_VOLSUBOSC].value +
                                         outputs[OUT_TRIANGLE].value * params[PARAM_VOLTRIANGLE].value +
                                         outputs[OUT_NOISE].value    * params[PARAM_VOLNOISE].value);
}


struct LS3340VCOWidget : ModuleWidget {
    LS3340VCOWidget(LS3340VCO *module) : ModuleWidget(module)
	{
        srand(time(0));

		//BACKGROUND
        setPanel(SVG::load(assetPlugin(plugin, "res/LabSeven_3340_VCO.svg")));

		//SCREWS
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		
		//INPUTS
        addInput(Port::create<LabSeven_Port>(Vec(114, 380-24-231+2), Port::INPUT, module, LS3340VCO::IN_PITCH));
        addInput(Port::create<LabSeven_Port>(Vec( 75, 380-24-231+2), Port::INPUT, module, LS3340VCO::IN_MOD));
        addInput(Port::create<LabSeven_Port>(Vec(114, 380-24-276+2), Port::INPUT, module, LS3340VCO::IN_RANGE));
        addInput(Port::create<LabSeven_Port>(Vec(219, 380-24-284+2), Port::INPUT, module, LS3340VCO::IN_LFO));
        addInput(Port::create<LabSeven_Port>(Vec(219, 380-24-214+2), Port::INPUT, module, LS3340VCO::IN_ENV));
        addInput(Port::create<LabSeven_Port>(Vec(153, 380-24- 32+2), Port::INPUT, module, LS3340VCO::IN_SUBOSCSELECT));
		
        //VCO SECTION
        addParam(ParamWidget::create<LabSeven_3340_FaderRedLargeRed>(Vec(28-4, 380-24-272), module, LS3340VCO::PARAM_MOD, 0.0f, 1.0f, 0.0f));
        addParam(ParamWidget::create<LabSeven_3340_KnobLargeRange>(Vec(69, 380-36-266), module, LS3340VCO::PARAM_RANGE, 0.0, 3.0, 1.0));
        addParam(ParamWidget::create<LabSeven_3340_FaderRedLargeRed>(Vec(164-4, 380-24-272), module, LS3340VCO::PARAM_PULSEWIDTH, 0.0f, 0.5f, 0.0f));
        addParam(ParamWidget::create<LabSeven_3340_FaderRedLargeYellow3Stage>(Vec(201-4, 380-40-234), module, LS3340VCO::PARAM_PWMSOURCE, 0.0f, 2.0f, 1.0f));
		
        //SOURCE MIXER SECTION
        addParam(ParamWidget::create<LabSeven_3340_FaderRedLargeGreen>(Vec( 28-4, 380-20-4-125), module, LS3340VCO::PARAM_VOLSQUARE, 0.0f, 1.0f, 0.0f));
        addParam(ParamWidget::create<LabSeven_3340_FaderRedLargeGreen>(Vec( 59-4, 380-20-4-125), module, LS3340VCO::PARAM_VOLSAW, 0.0f, 1.0f, 0.0f));
        addParam(ParamWidget::create<LabSeven_3340_FaderRedLargeGreen>(Vec( 90-4, 380-20-4-125), module, LS3340VCO::PARAM_VOLTRIANGLE, 0.0f, 1.0f, 0.0f));
        addParam(ParamWidget::create<LabSeven_3340_FaderRedLargeGreen>(Vec(121-4, 380-20-4-125), module, LS3340VCO::PARAM_VOLSUBOSC, 0.0f, 1.0f, 0.0f));
        addParam(ParamWidget::create<LabSeven_3340_FaderRedLargeYellow3Stage>(Vec(158-4-1, 380-40-88), module, LS3340VCO::PARAM_SUBOSCRATIO, 2.0f, 0.0f, 2.0f));
        addParam(ParamWidget::create<LabSeven_3340_FaderRedLargeGreen>(Vec(213-4, 380-20-4-125), module, LS3340VCO::PARAM_VOLNOISE, 0.0f, 1.0f, 0.0f));
		
		//OUTPUTS
        addOutput(Port::create<LabSeven_Port>(Vec( 24, 380-30-24), Port::OUTPUT, module, LS3340VCO::OUT_SQUARE));
        addOutput(Port::create<LabSeven_Port>(Vec( 55, 380-30-24), Port::OUTPUT, module, LS3340VCO::OUT_SAW));
        addOutput(Port::create<LabSeven_Port>(Vec(117, 380-30-24), Port::OUTPUT, module, LS3340VCO::OUT_SUB));
        addOutput(Port::create<LabSeven_Port>(Vec( 86, 380-30-24), Port::OUTPUT, module, LS3340VCO::OUT_TRIANGLE));
        addOutput(Port::create<LabSeven_Port>(Vec(181, 380-30-24), Port::OUTPUT, module, LS3340VCO::OUT_MIX));
        addOutput(Port::create<LabSeven_Port>(Vec(208, 380-30-24), Port::OUTPUT, module, LS3340VCO::OUT_NOISE));
	}
};

} // namespace rack_plugin_LabSeven

using namespace rack_plugin_LabSeven;

RACK_PLUGIN_MODEL_INIT(LabSeven, LS3340VCO) {
   Model *modelLS3340VCO = Model::create<LS3340VCO, LS3340VCOWidget>("LabSeven", "LS3340VCO", "LS3340-VCO", OSCILLATOR_TAG);
   return modelLS3340VCO;
}

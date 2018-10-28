
#include "dBiz.hpp"
#include "dsp/functions.hpp"
#include "dsp/decimator.hpp"
#include "dsp/filter.hpp"

namespace rack_plugin_dBiz {

extern float sawTable[2048];

template <int OVERSAMPLE, int QUALITY>
struct subBank
{

	float phase = 0.0;
	float freq;
	float pitch;

	Decimator<OVERSAMPLE, QUALITY> sawDecimator;

	// For analog detuning effect
	float pitchSlew = 0.0f;
	int pitchSlewIndex = 0;

	float sawBuffer[OVERSAMPLE] = {};

	//void setPitch(float pitchKnob, float pitchCv)
	void setPitch(float pitchKnob, float pitchCv)
	{
		// Compute frequency
		pitch = pitchKnob;
		const float pitchSlewAmount = 3.0f;
		pitch += pitchSlew * pitchSlewAmount;
		pitch += pitchCv;
		// Note C3
		freq = 261.626f * powf(2.0, pitch / 12.0);
		// Accumulate the phase
	}

void process(float deltaTime) {
			// Adjust pitch slew
			if (++pitchSlewIndex > 32) {
				const float pitchSlewTau = 100.0f; // Time constant for leaky integrator in seconds
				pitchSlew += (randomNormal() - pitchSlew / pitchSlewTau) * engineGetSampleTime();
				pitchSlewIndex = 0;
			}
		// Advance phase
		float deltaPhase = clamp(freq * deltaTime, 1e-6, 0.5f);



		for (int i = 0; i < OVERSAMPLE; i++) {
			sawBuffer[i] = 1.66f * interpolateLinear(sawTable, phase * 2047.f);
			// Advance phase
			phase += deltaPhase / OVERSAMPLE;
			phase = eucmod(phase, 1.0f);
		}
	}

	float saw() {
		return sawDecimator.process(sawBuffer);

}
};

struct SuHa : Module {
	enum ParamIds
	{
		SUM_VOL_PARAM,
		VCO_PARAM,
		SUB1_PARAM = VCO_PARAM + 2,
		SUB2_PARAM = SUB1_PARAM + 2,
		VCO_VOL_PARAM = SUB2_PARAM + 2,
		SUB1_VOL_PARAM = VCO_VOL_PARAM + 2,
		SUB2_VOL_PARAM = SUB1_VOL_PARAM + 2,
		NUM_PARAMS = SUB2_VOL_PARAM + 2
	};
	enum InputIds
	{
		VCO_INPUT,
		SUB1_INPUT = VCO_INPUT + 2,
		SUB2_INPUT = SUB1_INPUT + 2,
		NUM_INPUTS = SUB2_INPUT + 2
	};
	enum OutputIds
	{
		SUM_OUTPUT,
		VCO_OUTPUT,
		SUB1_OUTPUT = VCO_OUTPUT + 2,
		SUB2_OUTPUT = SUB1_OUTPUT + 2,
		NUM_OUTPUTS = SUB2_OUTPUT + 2
	};
	enum LightIds {
		NUM_LIGHTS
	};

	subBank <16,16> VCO[2]={};
	subBank <16,16> SUB1[2]={};
	subBank <16,16> SUB2[2]={};


	SuHa() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;


};


void SuHa::step() {



int s1[2]={};
int s2[2] = {};
float sum=0.0f;

for (int i=0;i<2;i++)
{
s1[i] = round(params[SUB1_PARAM+i].value + clamp(inputs[SUB1_INPUT+i].value, -15.0f, 15.0f));
if (s1[i]>15) s1[i]=15;
if (s1[i]<=1) s1[i]=1;

s2[i] = round(params[SUB2_PARAM+i].value + clamp(inputs[SUB2_INPUT+i].value, -15.0f, 15.0f));
if (s2[i]>15) s2[i]=15;
if (s2[i]<=1) s2[i]=1;


VCO[i].setPitch(params[VCO_PARAM+i].value,12*inputs[VCO_INPUT+i].value);
SUB1[i].freq=VCO[i].freq/s1[i];
SUB2[i].freq=VCO[i].freq/s2[i];

VCO[i].process(engineGetSampleTime());
SUB1[i].process(engineGetSampleTime());
SUB2[i].process(engineGetSampleTime());

outputs[VCO_OUTPUT + i].value =  2.0f * VCO[i].saw()*params[VCO_VOL_PARAM+i].value;
outputs[SUB1_OUTPUT + i].value = 2.0f * SUB1[i].saw()*params[SUB1_VOL_PARAM+i].value;
outputs[SUB2_OUTPUT + i].value = 2.0f * SUB2[i].saw()*params[SUB2_VOL_PARAM+i].value;

}

for (int i = 0; i < 2; i++)
{
	sum += clamp(outputs[VCO_OUTPUT + i].value + outputs[SUB1_OUTPUT + i].value + outputs[SUB2_OUTPUT + i].value,-5.0f,5.0f);
}


outputs[SUM_OUTPUT].value=sum*params[SUM_VOL_PARAM].value;


}


struct SuHaWidget : ModuleWidget {
	SuHaWidget(SuHa *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/SuHa.svg")));

		int KS=50;
		int JS = 37;
		float Side=7.5;

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


		///////////////////////////////////////////////////////////////////////////////////

		for (int i = 0; i < 2; i++)
		{

			addParam(ParamWidget::create<DKnob>(Vec(Side + 6, 87 + i * KS), module, SuHa::VCO_PARAM + i, -54.0, 54.0, 0.0));
			addParam(ParamWidget::create<DKnob>(Vec(Side + 6 + KS, 87 +i*KS), module, SuHa::SUB1_PARAM +i, 1.0, 15.0, 1.0));
			addParam(ParamWidget::create<DKnob>(Vec(Side + 6 + 2 * KS, 87 +i*KS), module, SuHa::SUB2_PARAM +i, 1.0, 15.0, 1.0));


			addParam(ParamWidget::create<Trimpot>(Vec(Side + 15, 25 + i*30), module, SuHa::VCO_VOL_PARAM +i, 0.0, 1.0, 0.0));
			addParam(ParamWidget::create<Trimpot>(Vec(Side + 15 + KS, 25 + i*30), module, SuHa::SUB1_VOL_PARAM +i, 0.0, 1.0, 0.0));
			addParam(ParamWidget::create<Trimpot>(Vec(Side + 15 + 2 * KS, 25 + i*30), module, SuHa::SUB2_VOL_PARAM +i, 0.0, 1.0, 0.0));
			

			addInput(Port::create<PJ301MVAPort>(Vec(Side + 11, 215+i*JS), Port::INPUT, module, SuHa::VCO_INPUT +i));
			addInput(Port::create<PJ301MVAPort>(Vec(Side + 11 + KS, 215+i*JS), Port::INPUT, module, SuHa::SUB1_INPUT +i));
			addInput(Port::create<PJ301MVAPort>(Vec(Side + 11 + 2 * KS, 215+i*JS), Port::INPUT, module, SuHa::SUB2_INPUT +i));


			addOutput(Port::create<PJ301MVAPort>(Vec(Side + 11, 215 + 2 * JS+i*JS), Port::OUTPUT, module, SuHa::VCO_OUTPUT +i));
			addOutput(Port::create<PJ301MVAPort>(Vec(Side + 11 + KS, 215 + 2 * JS+i*JS), Port::OUTPUT, module, SuHa::SUB1_OUTPUT +i));
			addOutput(Port::create<PJ301MVAPort>(Vec(Side + 11 + 2 * KS, 215 + 2 * JS+i*JS), Port::OUTPUT, module, SuHa::SUB2_OUTPUT +i));

		}

			addParam(ParamWidget::create<SDKnob>(Vec(Side + 40, 180), module, SuHa::SUM_VOL_PARAM, 0.0, 1.0, 0.0));
			addOutput(Port::create<PJ301MVAPort>(Vec(Side + 80, 185), Port::OUTPUT, module, SuHa::SUM_OUTPUT));

			

			//////////////////////////////////////////////////////////////////////////////////////////////////////////

			

		
}
};

} // namespace rack_plugin_dBiz

using namespace rack_plugin_dBiz;

RACK_PLUGIN_MODEL_INIT(dBiz, SuHa) {
   // Specify the Module and ModuleWidget subclass, human-readable
   // author name for categorization per plugin, module slug (should never
   // change), human-readable module name, and any number of tags
   // (found in `include/tags.hpp`) separated by commas.
   Model *modelSuHa = Model::create<SuHa, SuHaWidget>("dBiz", "SuHa", "SuHa", OSCILLATOR_TAG);
   return modelSuHa;
}

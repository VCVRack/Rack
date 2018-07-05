#include "FrozenWasteland.hpp"
#include "dsp/decimator.hpp"
#include "dsp/digital.hpp"
#include "StateVariableFilter.h"

using namespace std;

#define BANDS 5

namespace rack_plugin_FrozenWasteland {

struct VoxInhumana : Module {
	typedef float T;
	
	enum ParamIds {
		VOWEL_1_PARAM,
		VOWEL_2_PARAM,
		VOWEL_BALANCE_PARAM,
		VOICE_TYPE_PARAM,
		FC_MAIN_CUTOFF_PARAM,
		FREQ_1_CUTOFF_PARAM,
		FREQ_2_CUTOFF_PARAM,
		FREQ_3_CUTOFF_PARAM,
		FREQ_4_CUTOFF_PARAM,
		FREQ_5_CUTOFF_PARAM,
		AMP_1_PARAM,
		AMP_2_PARAM,
		AMP_3_PARAM,
		AMP_4_PARAM,
		AMP_5_PARAM,
		VOWEL_1_ATTENUVERTER_PARAM,
		VOWEL_2_ATTENUVERTER_PARAM,
		VOWEL_BALANCE_ATTENUVERTER_PARAM,
		VOICE_TYPE_ATTENUVERTER_PARAM,
		FC_MAIN_ATTENUVERTER_PARAM,
		FREQ_1_CV_ATTENUVERTER_PARAM,
		FREQ_2_CV_ATTENUVERTER_PARAM,
		FREQ_3_CV_ATTENUVERTER_PARAM,
		FREQ_4_CV_ATTENUVERTER_PARAM,
		FREQ_5_CV_ATTENUVERTER_PARAM,
		AMP_1_CV_ATTENUVERTER_PARAM,
		AMP_2_CV_ATTENUVERTER_PARAM,
		AMP_3_CV_ATTENUVERTER_PARAM,
		AMP_4_CV_ATTENUVERTER_PARAM,
		AMP_5_CV_ATTENUVERTER_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		SIGNAL_IN,
		VOWEL_1_CV_IN,
		VOWEL_2_CV_IN,
		VOWEL_BALANCE_CV_IN,
		VOICE_TYPE_CV_IN,
		FC_MAIN_CV_IN,
		FREQ_1_CUTOFF_INPUT,		
		FREQ_2_CUTOFF_INPUT,		
		FREQ_3_CUTOFF_INPUT,		
		FREQ_4_CUTOFF_INPUT,		
		FREQ_5_CUTOFF_INPUT,
		AMP_1_INPUT,		
		AMP_2_INPUT,		
		AMP_3_INPUT,		
		AMP_4_INPUT,		
		AMP_5_INPUT,		
		NUM_INPUTS
	};
	enum OutputIds {
		VOX_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		VOWEL_1_LIGHT,
		VOWEL_2_LIGHT,
		NUM_LIGHTS
	};
	
	StateVariableFilterState<T> filterStates[BANDS];
    StateVariableFilterParams<T> filterParams[BANDS];
	
	float freq[BANDS] = {0};
	float lastFreq[BANDS] = {0};

	float Q[BANDS] = {0};
	float lastQ[BANDS] = {0};

	float peak[BANDS] = {0};


	int vowel1 = 0;
	int vowel2 = 0;
	float vowelBalance = 0;
	int voiceType = 0;
	float fcShift = 0;

	float lerp(float v0, float v1, float t) {
	  return (1 - t) * v0 + t * v1;
	}


	// First Index is Voice Type (soprano,alto,counter-tenor,tenor,bass)
	// Second Index is Vowel (a,e,i,o,u) // should find more
	// Third if filter/formant index
	// Fourth: Cutoff,Q,Peak db)
	const float formantParameters[5][5][BANDS][3] = {
		//Bass
		{
			{{600,10,0},{1040,15,-7},{2250,21,-9},{2450,20,-9},{2750,21,-20}}, //a
			{{400,10,0},{1620,20,-12},{2400,24,-9},{2800,23,-12},{3100,26,-18}}, //e
			{{250,4,0},{1750,19,-30},{2600,26,-16},{3050,25,-22},{3340,28,-28}}, //i
			{{400,10,0},{750,9,-11},{2400,24,-21},{2600,22,-20},{2900,24,-40}}, //o
			{{350,9,0},{600,8,-20},{2400,24,-32},{2675,22,-28},{2950,25,-36}}, //u
		},
		//Tenor - in progress
		{
			{{650,8,0},{1080,12,-6},{2650,22,-7},{2900,22,-8},{3250,23,-22}}, //a
			{{400,6,0},{1700,21,-14},{2600,26,-12},{3200,27,-14},{3580,30,-20}}, //e
			{{290,7,0},{1870,21,-15},{2800,28,-18},{3250,27,-20},{3540,30,-30}}, //i
			{{400,10,0},{800,10,-10},{2600,26,-12},{2800,23,-12},{3000,25,-26}}, //o
			{{350,9,0},{600,10,-20},{2700,27,-17},{2900,24,-14},{3300,28,-26}}, //u
		},
		//Counter-Tenor - in progress
		{
			{{660,8,0},{1120,12,-6},{2750,23,-23},{3000,23,-24},{3350,24,-38}}, //a
			{{440,6,0},{1800,22,-14},{2700,27,-18},{3000,25,-20},{3300,28,-20}}, //e
			{{270,7,0},{1850,21,-24},{2900,29,-24},{3350,28,-36},{3590,30,-36}}, //i
			{{430,11,0},{820,10,-10},{2700,27,-26},{3000,25,-22},{3300,28,-34}}, //o
			{{370,9,0},{630,11,-20},{2750,28,-23},{3000,25,-30},{3400,28,-34}}, //u
		},
		//Alto
		{
			{{800,10,0},{1150,13,-4},{2800,27,-20},{3500,27,-36},{4950,35,-60}}, //a
			{{400,7,0},{1600,20,-24},{2700,23,-30},{3300,22,-35},{4950,25,-60}}, //e
			{{350,7,0},{1700,17,-20},{2700,23,-30},{3700,25,-36},{4950,25,-60}}, //i
			{{450,6,0},{800,10,-9},{2830,28,-16},{3500,27,-28},{4950,37,-55}}, //o
			{{325,7,0},{700,12,-12},{2530,15,-30},{3500,19,-40},{4950,25,-64}}, //u
		},
		//Soprano
		{
			{{800,10,0},{1150,12,-6},{2900,24,-32},{3900,30,-20},{4950,35,-50}}, //a
			{{350,6,0},{2000,20,-20},{2800,23,-15},{3600,24,-40},{4950,25,-56}}, //e
			{{270,5,0},{2140,24,-12},{2950,30,-26},{3900,33,-26},{4950,41,-44}}, //i
			{{450,6,0},{800,10,-11},{2830,28,-22},{3800,29,-22},{4950,37,-50}}, //o
			{{325,7,0},{700,12,-16},{2700,16,-35},{3800,21,-40},{4950,25,-60}}, //u
		}				
	};

	VoxInhumana() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {

		for (int i = 0; i < BANDS; ++i) {
			filterParams[i].setMode(StateVariableFilterParams<T>::Mode::BandPass);
			filterParams[i].setQ(5); 	
	        filterParams[i].setFreq(T(.1));
	    }
	}

	void reset() override;
	void step() override;
};

void VoxInhumana::reset() {
	params[FC_MAIN_CUTOFF_PARAM].value = 1.0f;
	params[VOWEL_BALANCE_PARAM].value = 0.0f;
	for(int i = 0;i<BANDS;i++) {
		params[FREQ_1_CUTOFF_PARAM+i].value = 0.0f;
		params[AMP_1_PARAM + i].value = 1.0f;
	}
}

void VoxInhumana::step() {
	
	float signalIn = inputs[SIGNAL_IN].value/5.0f;
	

	vowel1 = (int)clamp(params[VOWEL_1_PARAM].value + (inputs[VOWEL_1_CV_IN].value * params[VOWEL_1_ATTENUVERTER_PARAM].value),0.0f,4.0f);
	vowel2 = (int)clamp(params[VOWEL_2_PARAM].value + (inputs[VOWEL_2_CV_IN].value * params[VOWEL_2_ATTENUVERTER_PARAM].value),0.0f,4.0f);
	vowelBalance = clamp(params[VOWEL_BALANCE_PARAM].value + (inputs[VOWEL_BALANCE_CV_IN].value * params[VOWEL_BALANCE_ATTENUVERTER_PARAM].value /10.0f),0.0f,1.0f);
	voiceType = (int)clamp(params[VOICE_TYPE_PARAM].value + (inputs[VOICE_TYPE_CV_IN].value * params[VOICE_TYPE_ATTENUVERTER_PARAM].value),0.0f,4.0f);
	fcShift = clamp(params[FC_MAIN_CUTOFF_PARAM].value + (inputs[FC_MAIN_CV_IN].value * params[FC_MAIN_ATTENUVERTER_PARAM].value/10.0f) ,0.0f,2.0f);

	lights[VOWEL_1_LIGHT].value = 1.0-vowelBalance;
	lights[VOWEL_2_LIGHT].value = vowelBalance;
	
	for (int i=0; i<BANDS;i++) {
		float cutoffExp = params[FREQ_1_CUTOFF_PARAM+i].value + inputs[FREQ_1_CUTOFF_INPUT+i].value * params[FREQ_1_CV_ATTENUVERTER_PARAM+i].value; 
		cutoffExp = clamp(cutoffExp, -1.0f, 1.0f);
		freq[i] = lerp(formantParameters[voiceType][vowel1][i][0],formantParameters[voiceType][vowel2][i][0],vowelBalance); 
		//Apply individual formant CV
		freq[i] = freq[i] + (freq[i] / 2 * cutoffExp); //Formant CV can alter formant by +/- 50%
		//Apply global Fc shift
		freq[i] = freq[i] * fcShift; //Global can double or really lower freq
		
		Q[i] = lerp(formantParameters[voiceType][vowel1][i][1],formantParameters[voiceType][vowel2][i][1],vowelBalance);
		peak[i] = lerp(formantParameters[voiceType][vowel1][i][2],formantParameters[voiceType][vowel2][i][2],vowelBalance);		


		if(freq[i] != lastFreq[i]) {
			float Fc = freq[i] / engineGetSampleRate();
			filterParams[i].setFreq(T(Fc));
			lastFreq[i] = freq[i];
		}
		if(Q[i] != lastQ[i]) {
			filterParams[i].setQ(Q[i]); 
			lastQ[i] = Q[i];
		}		
	}

	float out = 0.0f;	
	for(int i=0;i<BANDS;i++) {
		float filterOut = StateVariableFilter<T>::run(signalIn, filterStates[i], filterParams[i]);;
		float attenuation = powf(10,peak[i] / 20.0f);
		float manualAttenuation = params[AMP_1_PARAM+i].value + inputs[AMP_1_INPUT+i].value * params[AMP_1_CV_ATTENUVERTER_PARAM+i].value; 
		attenuation = clamp(attenuation * manualAttenuation, 0.0f, 1.0f);
		out += filterOut * attenuation * 5.0f;
	}


	outputs[VOX_OUTPUT].value = out / 5.0f;
	
}


struct VoxInhumanaBandDisplay : TransparentWidget {
	VoxInhumana *module;
	int frame = 0;
	std::shared_ptr<Font> font;

	VoxInhumanaBandDisplay() {
		font = Font::load(assetPlugin(plugin, "res/fonts/DejaVuSansMono.ttf"));
	}

	void drawVowel(NVGcontext *vg, Vec pos, int vowel) {
		nvgFontSize(vg, 22);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -2);

		const char* vowelNames[5] = {"A","E","I","O","U"};

		nvgFillColor(vg, nvgRGBA(0x00, 0xaf, 0x00, 0xff));
		char text[128];
		snprintf(text, sizeof(text), "%s", vowelNames[vowel]);
		nvgText(vg, pos.x + 8, pos.y, text, NULL);
	}

	void drawVoiceType(NVGcontext *vg, Vec pos, int voiceType) {
		nvgFontSize(vg, 14);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -2);

		const char* voiceNames[5] = {"Bass","Tenor","Counter-Tenor","Alto","Soprano"};

		nvgFillColor(vg, nvgRGBA(0x00, 0x8f, 0xff, 0xff));
		char text[128];
		snprintf(text, sizeof(text), "%s", voiceNames[voiceType]);
		nvgText(vg, pos.x + 8, pos.y, text, NULL);
	}

	void draw(NVGcontext *vg) override {
		drawVowel(vg, Vec(24.0, box.size.y - 99),module->vowel1);
		drawVowel(vg, Vec(156.0, box.size.y - 99),module->vowel2);
		drawVoiceType(vg, Vec(20.0, box.size.y - 27),module->voiceType);		
	}
};

struct VoxInhumanaWidget : ModuleWidget {
	VoxInhumanaWidget(VoxInhumana *module);
};

VoxInhumanaWidget::VoxInhumanaWidget(VoxInhumana *module) : ModuleWidget(module) {
	box.size = Vec(15*14, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/VoxInhumana.svg")));
		addChild(panel);
	}
	
	{
		VoxInhumanaBandDisplay *offsetDisplay = new VoxInhumanaBandDisplay();
		offsetDisplay->module = module;
		offsetDisplay->box.pos = Vec(15, 10);
		offsetDisplay->box.size = Vec(box.size.x, 140);
		addChild(offsetDisplay);
	}


	addParam(ParamWidget::create<RoundBlackKnob>(Vec(10, 30), module, VoxInhumana::VOWEL_1_PARAM, 0, 4.6, 0));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(140, 30), module, VoxInhumana::VOWEL_2_PARAM, 0, 4.6, 0));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(89, 30), module, VoxInhumana::VOWEL_BALANCE_PARAM, 0.0, 1.0, 0));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(7, 103), module, VoxInhumana::VOICE_TYPE_PARAM, 0, 4.6, 0));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(140, 103), module, VoxInhumana::FC_MAIN_CUTOFF_PARAM, 0.0, 2.0, 1));

	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(40, 62), module, VoxInhumana::VOWEL_1_ATTENUVERTER_PARAM, -1.0, 1.0, 0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(170, 62), module, VoxInhumana::VOWEL_2_ATTENUVERTER_PARAM, -1.0, 1.0, 0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(107, 62), module, VoxInhumana::VOWEL_BALANCE_ATTENUVERTER_PARAM, -1.0, 1.0, 0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(31, 134), module, VoxInhumana::VOICE_TYPE_ATTENUVERTER_PARAM, -1.0, 1.0, 0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(169, 125), module, VoxInhumana::FC_MAIN_ATTENUVERTER_PARAM, -1.0, 1.0, 0));
		

	addParam(ParamWidget::create<RoundBlackKnob>(Vec(15, 160), module, VoxInhumana::FREQ_1_CUTOFF_PARAM, -1.0, 1.0, 0));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(15, 195), module, VoxInhumana::FREQ_2_CUTOFF_PARAM, -1.0, 1.0, 0));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(15, 230), module, VoxInhumana::FREQ_3_CUTOFF_PARAM, -1.0, 1.0, 0));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(15, 265), module, VoxInhumana::FREQ_4_CUTOFF_PARAM, -1.0, 1.0, 0));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(15, 300), module, VoxInhumana::FREQ_5_CUTOFF_PARAM, -1.0, 1.0, 0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(80, 162), module, VoxInhumana::FREQ_1_CV_ATTENUVERTER_PARAM, -1.0, 1.0, 0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(80, 197), module, VoxInhumana::FREQ_2_CV_ATTENUVERTER_PARAM, -1.0, 1.0, 0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(80, 232), module, VoxInhumana::FREQ_3_CV_ATTENUVERTER_PARAM, -1.0, 1.0, 0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(80, 267), module, VoxInhumana::FREQ_4_CV_ATTENUVERTER_PARAM, -1.0, 1.0, 0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(80, 302), module, VoxInhumana::FREQ_5_CV_ATTENUVERTER_PARAM, -1.0, 1.0, 0));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(110, 160), module, VoxInhumana::AMP_1_PARAM, 0.0, 2.0, 1));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(110, 195), module, VoxInhumana::AMP_2_PARAM, 0.0, 2.0, 1));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(110, 230), module, VoxInhumana::AMP_3_PARAM, 0.0, 2.0, 1));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(110, 265), module, VoxInhumana::AMP_4_PARAM, 0.0, 2.0, 1));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(110, 300), module, VoxInhumana::AMP_5_PARAM, 0.0, 2.0, 1));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(175, 162), module, VoxInhumana::AMP_1_CV_ATTENUVERTER_PARAM, -1.0, 1.0, 0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(175, 197), module, VoxInhumana::AMP_2_CV_ATTENUVERTER_PARAM, -1.0, 1.0, 0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(175, 232), module, VoxInhumana::AMP_3_CV_ATTENUVERTER_PARAM, -1.0, 1.0, 0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(175, 267), module, VoxInhumana::AMP_4_CV_ATTENUVERTER_PARAM, -1.0, 1.0, 0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(175, 302), module, VoxInhumana::AMP_5_CV_ATTENUVERTER_PARAM, -1.0, 1.0, 0));

	addInput(Port::create<PJ301MPort>(Vec(13, 62), Port::INPUT, module, VoxInhumana::VOWEL_1_CV_IN));
	addInput(Port::create<PJ301MPort>(Vec(143, 62), Port::INPUT, module, VoxInhumana::VOWEL_2_CV_IN));
	addInput(Port::create<PJ301MPort>(Vec(80, 62), Port::INPUT, module, VoxInhumana::VOWEL_BALANCE_CV_IN));
	addInput(Port::create<PJ301MPort>(Vec(5, 134), Port::INPUT, module, VoxInhumana::VOICE_TYPE_CV_IN));
	addInput(Port::create<PJ301MPort>(Vec(170, 97), Port::INPUT, module, VoxInhumana::FC_MAIN_CV_IN));


	addInput(Port::create<PJ301MPort>(Vec(50, 162), Port::INPUT, module, VoxInhumana::FREQ_1_CUTOFF_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(50, 197), Port::INPUT, module, VoxInhumana::FREQ_2_CUTOFF_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(50, 232), Port::INPUT, module, VoxInhumana::FREQ_3_CUTOFF_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(50, 267), Port::INPUT, module, VoxInhumana::FREQ_4_CUTOFF_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(50, 302), Port::INPUT, module, VoxInhumana::FREQ_5_CUTOFF_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(145, 162), Port::INPUT, module, VoxInhumana::AMP_1_INPUT)	);
	addInput(Port::create<PJ301MPort>(Vec(145, 197), Port::INPUT, module, VoxInhumana::AMP_2_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(145, 232), Port::INPUT, module, VoxInhumana::AMP_3_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(145, 267), Port::INPUT, module, VoxInhumana::AMP_4_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(145, 302), Port::INPUT, module, VoxInhumana::AMP_5_INPUT));

	addInput(Port::create<PJ301MPort>(Vec(56, 338), Port::INPUT, module, VoxInhumana::SIGNAL_IN));

	addOutput(Port::create<PJ301MPort>(Vec(130, 338), Port::OUTPUT, module, VoxInhumana::VOX_OUTPUT));

	addChild(ModuleLightWidget::create<LargeLight<GreenLight>>(Vec(72, 37), module, VoxInhumana::VOWEL_1_LIGHT));
	addChild(ModuleLightWidget::create<LargeLight<GreenLight>>(Vec(120, 37), module, VoxInhumana::VOWEL_2_LIGHT));


	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH-12, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH + 12, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH-12, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH + 12, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

} // namespace rack_plugin_FrozenWasteland

using namespace rack_plugin_FrozenWasteland;

RACK_PLUGIN_MODEL_INIT(FrozenWasteland, VoxInhumana) {
   Model *modelVoxInhumana = Model::create<VoxInhumana, VoxInhumanaWidget>("Frozen Wasteland", "VoxInhumana", "Vox Inhumana", FILTER_TAG);
   return modelVoxInhumana;
}

#include "mental.hpp"

#include "dsp/digital.hpp"

namespace rack_plugin_mental {

struct LowFrequencyOscillator {
	float phase = 0.0;
	float pw = 0.5;
	float freq = 1.0;
	bool offset = false;
	bool invert = false;
	SchmittTrigger resetTrigger;
	LowFrequencyOscillator() {
	}
  void setFreq(float freq_to_set)
  {
    freq = freq_to_set;
  }
	void setPitch(float pitch) {
		pitch = fminf(pitch, 8.0);
		freq = powf(2.0, pitch);
	}
  void setPhase(float phase_to_set)
  {
    phase = phase_to_set;
  }
  
	void setPulseWidth(float pw_) {
		const float pwMin = 0.01;
		pw = clamp(pw_, pwMin, 1.0f - pwMin);
	}
	void setReset(float reset) {
		if (resetTrigger.process(reset)) {
			phase = 0.0;
		}
	}
	void step(float dt) {
		float deltaPhase = fminf(freq * dt, 0.5);
		phase += deltaPhase;
		if (phase >= 1.0)
			phase -= 1.0;
	}
	float sin() {
		if (offset)
			return 1.0 - cosf(2*M_PI * phase) * (invert ? -1.0 : 1.0);
		else
			return sinf(2*M_PI * phase) * (invert ? -1.0 : 1.0);
	}
	float tri(float x) {
		return 4.0 * fabsf(x - roundf(x));
	}
	float tri() {
		if (offset)
			return tri(invert ? phase - 0.5 : phase);
		else
			return -1.0 + tri(invert ? phase - 0.25 : phase - 0.75);
	}
	float saw(float x) {
		return 2.0 * (x - roundf(x));
	}
	float saw() {
		if (offset)
			return invert ? 2.0 * (1.0 - phase) : 2.0 * phase;
		else
			return saw(phase) * (invert ? -1.0 : 1.0);
	}
	float sqr() {
		float sqr = (phase < pw) ^ invert ? 1.0 : -1.0;
		return offset ? sqr + 1.0 : sqr;
	}
	float light() {
		return sinf(2*M_PI * phase);
	}
};

struct MentalQuadLFO : Module {
	enum ParamIds {
    MODE_BUTTON_PARAM,
		FREQ_PARAM,
		NUM_PARAMS = FREQ_PARAM + 4
	};
	enum InputIds {
		FREQ_INPUT,
		RESET_INPUT = FREQ_INPUT + 4,
		NUM_INPUTS = RESET_INPUT + 4
	};
	enum OutputIds {
		SIN_OUTPUT,
		TRI_OUTPUT = SIN_OUTPUT + 4,
		SAW_OUTPUT = TRI_OUTPUT + 4,
		SQR_OUTPUT = SAW_OUTPUT + 4,
		NUM_OUTPUTS = SQR_OUTPUT + 4
	};
	enum LightIds {
    PHASE_POS_LIGHT,
		PHASE_NEG_LIGHT = PHASE_POS_LIGHT + 4,
		MODE_LIGHTS = PHASE_NEG_LIGHT + 4 ,
		NUM_LIGHTS = MODE_LIGHTS + 5
	};
 
  LowFrequencyOscillator oscillator[4];
  
  SchmittTrigger mode_button_trigger;
  int mode = 0;
  
	MentalQuadLFO() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
  
  json_t *toJson() override
  {
		json_t *rootJ = json_object();
    
    // save mode
		json_t *modeJ = json_integer((int)mode);
						
		json_object_set_new(rootJ, "mode", modeJ);    
    return rootJ;
  }
  
  void fromJson(json_t *rootJ) override
  {
    // read mode
		json_t *modeJ = json_object_get(rootJ, "mode");
		if (modeJ)
    {
			mode = json_integer_value(modeJ);		
		}  
  }
};

void MentalQuadLFO::step()
{
  if (mode_button_trigger.process(params[MODE_BUTTON_PARAM].value))
  {
    mode ++;
    for (int i = 0 ; i < 4 ; i ++)
    {
      oscillator[i].setReset(5.00);
    } 
    if (mode > 4) mode = 0;
    for (int i = 0 ; i < 5 ; i ++)
    {
      lights[MODE_LIGHTS + i].value = 0.0;
    }
  }
  
  lights[MODE_LIGHTS + mode].value = 1.0;
  if (mode == 0)
  {
    for (int i = 0 ; i < 4 ; i++)
    {
      oscillator[i].setPitch((params[FREQ_PARAM + i].value * 10 - 5) + inputs[FREQ_INPUT + i].value);
      oscillator[i].setReset(inputs[RESET_INPUT + i].value);
    }
  } else
  if (mode == 1)
  {
    for (int i = 0 ; i < 4 ; i++)
    {
      oscillator[i].setPitch((params[FREQ_PARAM].value * 10 - 5) + inputs[FREQ_INPUT].value);
      oscillator[i].setReset(inputs[RESET_INPUT].value);
      oscillator[i].setPhase(oscillator[0].phase + i * 0.25);
    }
  } else
  if (mode == 2)
  {
    for (int i = 0 ; i < 4 ; i++)
    {
      oscillator[i].setPitch((params[FREQ_PARAM].value * 10 - 5) + inputs[FREQ_INPUT].value);
      oscillator[i].setReset(inputs[RESET_INPUT].value);
      if (i > 0) oscillator[i].setPhase(oscillator[0].phase + (params[FREQ_PARAM + i].value));
    }
  } else
  if (mode == 3)
  {
    oscillator[0].setPitch((params[FREQ_PARAM].value * 10 - 5) + inputs[FREQ_INPUT].value);
    oscillator[1].setFreq(oscillator[0].freq / (std::round(params[FREQ_PARAM + 1].value * 11 + 1)));
    oscillator[2].setFreq(oscillator[0].freq / (std::round(params[FREQ_PARAM + 2].value * 11 + 1)));
    oscillator[3].setFreq(oscillator[0].freq / (std::round(params[FREQ_PARAM + 3].value * 11 + 1)));
    oscillator[0].setReset(inputs[RESET_INPUT].value); 
    oscillator[1].setReset(inputs[RESET_INPUT].value);
    oscillator[2].setReset(inputs[RESET_INPUT].value);
    oscillator[3].setReset(inputs[RESET_INPUT].value);  
  }
  if (mode == 4)
  {
    oscillator[0].setPitch((params[FREQ_PARAM].value * 10 - 5) + inputs[FREQ_INPUT].value);
    oscillator[1].setFreq(oscillator[0].freq * (std::round(params[FREQ_PARAM + 1].value * 11 + 1)));
    oscillator[2].setFreq(oscillator[0].freq * (std::round(params[FREQ_PARAM + 2].value * 11 + 1)));
    oscillator[3].setFreq(oscillator[0].freq * (std::round(params[FREQ_PARAM + 3].value * 11 + 1)));
    oscillator[0].setReset(inputs[RESET_INPUT].value); 
    oscillator[1].setReset(inputs[RESET_INPUT].value);
    oscillator[2].setReset(inputs[RESET_INPUT].value);
    oscillator[3].setReset(inputs[RESET_INPUT].value);
    if (oscillator[0].phase == 0.0)
    {
      oscillator[1].setReset(5.0);
      oscillator[2].setReset(5.0);
      oscillator[3].setReset(5.0);
    }    
  }
  
  for (int i = 0 ; i < 4 ; i++)
  {
	  
    oscillator[i].step(1.0 / engineGetSampleRate());

	  outputs[SIN_OUTPUT + i].value = 5.0 * oscillator[i].sin();
	  outputs[TRI_OUTPUT + i].value = 5.0 * oscillator[i].tri();
	  outputs[SAW_OUTPUT + i].value = 5.0 * oscillator[i].saw();
	  outputs[SQR_OUTPUT + i].value = 5.0 * oscillator[i].sqr();

	  lights[PHASE_POS_LIGHT + i].setBrightnessSmooth(fmaxf(0.0, oscillator[i].light()));
	  lights[PHASE_NEG_LIGHT + i].setBrightnessSmooth(fmaxf(0.0, -oscillator[i].light()));
  } 
}

////////////////////////////////////////////////////////////////////////////////
struct MentalQuadLFOWidget : ModuleWidget {
	MentalQuadLFOWidget(MentalQuadLFO *module);
};

MentalQuadLFOWidget::MentalQuadLFOWidget(MentalQuadLFO *module) : ModuleWidget(module)
{

  setPanel(SVG::load(assetPlugin(plugin, "res/MentalQuadLFO.svg")));

  int x_offset = 10.10;
  for (int i = 0 ; i < 4 ; i++)
  {
	  addParam(ParamWidget::create<BefacoSlidePot>(mm2px(Vec(2.792 + i * x_offset, 3.937)), module,MentalQuadLFO::FREQ_PARAM + i, 0.0, 1.0, 0.0));

	  addInput(Port::create<CVInPort>(mm2px(Vec(1.003 + i * x_offset, 61.915)), Port::INPUT, module, MentalQuadLFO::FREQ_INPUT + i));
	  addInput(Port::create<GateInPort>(mm2px(Vec(1.003 + i * x_offset, 72.858)), Port::INPUT, module, MentalQuadLFO::RESET_INPUT + i));

	  addOutput(Port::create<OutPort>(mm2px(Vec(1.003 + i * x_offset, 83.759)), Port::OUTPUT, module, MentalQuadLFO::SIN_OUTPUT + i));
	  addOutput(Port::create<OutPort>(mm2px(Vec(1.003 + i * x_offset, 94.173)), Port::OUTPUT, module, MentalQuadLFO::TRI_OUTPUT + i));
	  addOutput(Port::create<OutPort>(mm2px(Vec(1.003 + i * x_offset, 105.169)), Port::OUTPUT, module, MentalQuadLFO::SAW_OUTPUT + i));
	  addOutput(Port::create<OutPort>(mm2px(Vec(1.003 + i * x_offset, 114.583)), Port::OUTPUT, module, MentalQuadLFO::SQR_OUTPUT + i));
  	
    addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(13 + i * 30, 125), module, MentalQuadLFO::PHASE_POS_LIGHT + i));
  }
  for (int i = 0 ; i < 5 ; i++)
  {
    addChild(ModuleLightWidget::create<MedLight<BlueLED>>(mm2px(Vec(2.905 + i * 8 , 50.035)), module, MentalQuadLFO::MODE_LIGHTS + i));
  }
  addParam(ParamWidget::create<LEDButton>(Vec(50, 160), module, MentalQuadLFO::MODE_BUTTON_PARAM, 0.0, 1.0, 0.0));
  
}

} // namespace rack_plugin_mental

using namespace rack_plugin_mental;

RACK_PLUGIN_MODEL_INIT(mental, MentalQuadLFO) {
   Model *modelMentalQuadLFO = Model::create<MentalQuadLFO, MentalQuadLFOWidget>("mental", "MentalQuadLFO", "Quad LFO", LFO_TAG, QUAD_TAG, CLOCK_TAG);
   return modelMentalQuadLFO;
}

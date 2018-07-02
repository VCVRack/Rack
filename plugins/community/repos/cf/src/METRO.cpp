#include "cf.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_cf {

struct METRO : Module {
	enum ParamIds {
		BPM_PARAM,
		RST_PARAM,
		BEAT_PARAM,
		ON_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		ON_INPUT,
		BPM_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		MES_OUTPUT,
		BEAT_OUTPUT,
		START_OUTPUT,
		NUM_OUTPUTS
	};
    enum LightIds {
		ON_LIGHT,
		MES_LIGHT,
		BEAT_LIGHT,
		NUM_LIGHTS
	};

int max_METRO = 120 ;
int beatl = 0 ;
int mesl = 0 ;
int beats = 0 ;
int mess = 0 ;
int strt = 0 ;
int note ;
float toc_phase = 0.f;
uint32_t toc = 0u;
SchmittTrigger onTrigger;
SchmittTrigger oninTrigger;
SchmittTrigger rstTrigger;
bool ON_STATE = false;
float or_gain ;
int or_affi ;

	METRO() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {onReset();}
	void step() override;
void onReset() override {
			ON_STATE = true;
			
			}

json_t *toJson() override {
		json_t *rootJ = json_object();

		// on
		json_object_set_new(rootJ, "onstate", json_integer(ON_STATE));
		return rootJ;
		}

void fromJson(json_t *rootJ) override {

		// on
		json_t *onstateJ = json_object_get(rootJ, "onstate");
		if (onstateJ)
			ON_STATE = json_integer_value(onstateJ);
	
	}
};


void METRO::step() {

	if (!inputs[BPM_INPUT].active) {
		max_METRO = floor(params[BPM_PARAM].value);
		or_affi = 0;
	} else {
		max_METRO = round(clamp(inputs[BPM_INPUT].value *30,0.0f,300.0f));
		or_affi = 1;
		or_gain = max_METRO/30.0;
	}

	float bpm = max_METRO ;
	bool toced = false;

	if (onTrigger.process(params[ON_PARAM].value)+oninTrigger.process(inputs[ON_INPUT].value))
			{if (ON_STATE == 0) {ON_STATE = 1; strt = 5;} else ON_STATE = 0;}

	lights[ON_LIGHT].value = ON_STATE ;

	
	if (rstTrigger.process(params[RST_PARAM].value))
		{toc = 47u;toc_phase = 1.f; strt = 5;}
	

	if (ON_STATE) {
		toc_phase += ((bpm / 60.f) / engineGetSampleRate()) * 12.f;
		
		if(toc_phase >= 1.f) {
			toced = true;
			toc = (toc+1u) % 48u ;
			toc_phase -= 1.f;
			}

		if(toced) {
			if (toc % 12u ? 0 : 1) beatl = 4000;
			if (toc % 48u ? 0 : 1) mesl = 5000;
			if (toc % 12u ? 0 : 1) beats = 200;
			if (toc % 48u ? 0 : 1) mess = 200;
			note =5;
			}

		if (beatl>0) {
			lights[BEAT_LIGHT].value = true; 
			beatl = beatl -1;
		} else {
			lights[BEAT_LIGHT].value = false;
			}

		if (mesl>0) {
			lights[MES_LIGHT].value = true; 
			mesl = mesl -1;
		} else {
			lights[MES_LIGHT].value = false;
			}


		if (beats>0) {
			beats = beats -1;
			outputs[BEAT_OUTPUT].value = 2.5 * (beats- 150*round(beats/150))/150;
		} else {
			outputs[BEAT_OUTPUT].value = 0.0;
			}

		if (mess>0) { 
			mess = mess -1;
			outputs[BEAT_OUTPUT].value = 5.0 * (mess- 150*round(mess/150))/150;
			}

	} else {toc = 47u;toc_phase = 1.f;outputs[OUT_OUTPUT].value = 0.f;}
    

	if (strt > 0) {outputs[START_OUTPUT].value = 10.f;strt=strt-1;} else outputs[START_OUTPUT].value = 0.f;
	if (note > 0) {outputs[OUT_OUTPUT].value = 10.f;note=note-1;} else outputs[OUT_OUTPUT].value = 0.f;
}

struct NumDisplayWidget : TransparentWidget {

  int *value;
  std::shared_ptr<Font> font;

  NumDisplayWidget() {
    font = Font::load(assetPlugin(plugin, "res/Segment7Standard.ttf"));
  };

  void draw(NVGcontext *vg) {
    // Background
    NVGcolor backgroundColor = nvgRGB(0x44, 0x44, 0x44);
    NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 4.0);
    nvgFillColor(vg, backgroundColor);
    nvgFill(vg);
    nvgStrokeWidth(vg, 1.0);
    nvgStrokeColor(vg, borderColor);
    nvgStroke(vg);

    nvgFontSize(vg, 18);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 2.5);

    std::string to_display = std::to_string(*value);


    while(to_display.length()<3) to_display = ' ' + to_display;

    Vec textPos = Vec(6.0f, 17.0f);

    NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "~~~", NULL);

    textColor = nvgRGB(0xda, 0xe9, 0x29);

    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "\\\\\\", NULL);


    textColor = nvgRGB(0x28, 0xb0, 0xf3);
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, to_display.c_str(), NULL);
  }
};

struct MOTORPOTDisplay : TransparentWidget {

	float d;
	float *gainX ;
	int *affich;

	MOTORPOTDisplay() {
		
	}
	
	void draw(NVGcontext *vg) {
		if (*affich==1) {
		float xx = d*sin(-(*gainX*0.17+0.15)*M_PI) ;
		float yy = d*cos((*gainX*0.17+0.15)*M_PI) ;

		
			nvgBeginPath(vg);
			nvgCircle(vg, 0,0, d);
			nvgFillColor(vg, nvgRGBA(0x00, 0x00, 0x00, 0xff));
			nvgFill(vg);	
		
			nvgStrokeWidth(vg,1.2);
			nvgStrokeColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0xff));
			{
				nvgBeginPath(vg);
				nvgMoveTo(vg, 0,0);
				nvgLineTo(vg, xx,yy);
				nvgClosePath(vg);
			}
			nvgStroke(vg);
		}

	}
};


struct METROWidget : ModuleWidget {
	METROWidget(METRO *module);
};

METROWidget::METROWidget(METRO *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/METRO.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(27, 107), module, METRO::BPM_PARAM, 0.0f, 301.0f, 120.1f));
	addInput(Port::create<PJ301MPort>(Vec(11, 141), Port::INPUT, module, METRO::BPM_INPUT));
	{
		MOTORPOTDisplay *display = new MOTORPOTDisplay();
		display->box.pos = Vec(46, 126);
		display->d = 19.1;
		display->gainX = &module->or_gain;
		display->affich = &module->or_affi;
		addChild(display);
	}

     	addParam(ParamWidget::create<LEDButton>(Vec(38, 167), module, METRO::ON_PARAM, 0.0f, 1.0f, 0.0f));
	addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(42.4, 171.4), module, METRO::ON_LIGHT));
	addInput(Port::create<PJ301MPort>(Vec(11, 171), Port::INPUT, module, METRO::ON_INPUT));

     	addParam(ParamWidget::create<LEDButton>(Vec(38, 197), module, METRO::RST_PARAM, 0.0f, 1.0f, 0.0f));
	addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(42.4, 201.4), module, METRO::MES_LIGHT));

     	addParam(ParamWidget::create<LEDButton>(Vec(38, 227), module, METRO::BEAT_PARAM, 0.0f, 1.0f, 0.0f));
	addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(42.4, 231.4), module, METRO::BEAT_LIGHT));
	addOutput(Port::create<PJ301MPort>(Vec(54, 265), Port::OUTPUT, module, METRO::BEAT_OUTPUT));

	addOutput(Port::create<PJ301MPort>(Vec(11, 321), Port::OUTPUT, module, METRO::START_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(54, 321), Port::OUTPUT, module, METRO::OUT_OUTPUT));

	NumDisplayWidget *display = new NumDisplayWidget();
	display->box.pos = Vec(20,56);
	display->box.size = Vec(50, 20);
	display->value = &module->max_METRO;
	addChild(display);

	
}

} // namespace rack_plugin_cf

using namespace rack_plugin_cf;

RACK_PLUGIN_MODEL_INIT(cf, METRO) {
   Model *modelMETRO = Model::create<METRO, METROWidget>("cf", "METRO", "Metro", CLOCK_TAG);
   return modelMETRO;
}


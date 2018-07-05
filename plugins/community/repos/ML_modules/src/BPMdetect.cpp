#include "ML_modules.hpp"
#include "dsp/digital.hpp"
#include "util/math.hpp"

#include <iostream>
#include <sstream>
#include <iomanip>

namespace rack_plugin_ML_modules {

struct BPMdetect : Module {
	enum ParamIds {
		SMOOTH_PARAM,
		MULT2_PARAM,
		MULT3_PARAM,
		SWING2_PARAM,
		SWING3_PARAM,
		DELAY1_PARAM,
		DELAY2_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		GATE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		LFO_OUTPUT,
		SEQ_OUTPUT,
		DELAY_OUTPUT,
		TRIG1_OUTPUT,
		TRIG2_OUTPUT,
		TRIG3_OUTPUT,
		NUM_OUTPUTS
	};
	enum LighIds {
		NUM_LIGHTS
	};

	BPMdetect() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) { misses = 0; onSampleRateChange();};

	void step() override;

	int misses = 0;
	int count2 = 0;
	int count3 = 0;

	float timer = 0.0;
	float timer1 = 0.0;
	float timer2 = 0.0;
	float timer3 = 0.0;
	float seconds = 0.0;
	float deltaT;
	float BPM=0.0;
	float lfo_volts=0.0;
	float delay_volts=0.0;

	bool fine = false;

	inline bool checkBeat(float timer, int mult) {
		return ( ((timer - mult*seconds) * (timer - mult*seconds) / (seconds*seconds) < 0.2 ) && misses < 4);
	}

	float gSampleRate;
	void reset() override {onSampleRateChange();};
	void onSampleRateChange() override {gSampleRate = engineGetSampleRate(); deltaT = 1.0/gSampleRate;}

	SchmittTrigger gateTrigger;
	PulseGenerator outPulse1, outPulse2, outPulse3;
};



void BPMdetect::step() {


	float mult2 = roundf(params[MULT2_PARAM].value);
	float mult3 = roundf(params[MULT3_PARAM].value);

	float factor2 = ( fine ? 1.0f + 0.25f * (params[SWING2_PARAM].value - 1.0f): params[SWING2_PARAM].value ) / mult2;
	float factor3 = ( fine ? 1.0f + 0.25f * (params[SWING3_PARAM].value - 1.0f): params[SWING3_PARAM].value ) / mult3;


	if( inputs[GATE_INPUT].active) {

		if( timer1 > seconds ) {
			outPulse1.trigger(0.01);
			timer1 = 0.0;
		}

		if( (timer2 > seconds*factor2) /* && (count2 < mult2) */ ) {
//			if(nearf(factor2,1.0)) std::cerr << timer2 << "\n";
			outPulse2.trigger(0.01);
			timer2 = 0.0;
			// count2++;
		}

		if( (timer3 > seconds*factor3) /* && (count3<mult3) */ ) {
			outPulse3.trigger(0.01);
			timer3 = 0.0 ;
			// count3++;
		}

		if( gateTrigger.process(inputs[GATE_INPUT].value) ) {


			if(timer>0) {
				float new_seconds;


				bool found=false;

				for(int mult=1;  !found && mult < 20; mult++ )  {
					if(checkBeat(timer, mult)) {
						new_seconds = timer/mult;
						if(mult==1) misses=0;
						else        misses++;
						found = true;
					};
				};

				if( !found ) {
//					std::cerr << "default. misses = " << misses << "\n";
					new_seconds = timer;
					misses=0;
				}


				float a = params[SMOOTH_PARAM].value;
				seconds = ( (1.0-a)*seconds + a*new_seconds);
				BPM=60.0/seconds;

				lfo_volts = 1.0 - log2(seconds) ;

				float num   = roundf(params[DELAY1_PARAM].value);
				float denom = roundf(params[DELAY2_PARAM].value);

				delay_volts = 10.0*(3.0+log10(seconds * num/denom))/4.0;

				timer -= seconds;
				timer1 = 0.0;
				timer2 = 0.0;
				timer3 = 0.0;
				count2 = 1;
				count3 = 1;
				outPulse1.trigger(0.01);
				outPulse2.trigger(0.01);
				outPulse3.trigger(0.01);

			}

		};

	};

	timer += deltaT;
	timer1 += deltaT;
	timer2 += deltaT;
	timer3 += deltaT;

	outputs[TRIG1_OUTPUT].value = outPulse1.process(deltaT) ? 10.0 : 0.0;
	outputs[TRIG2_OUTPUT].value = outPulse2.process(deltaT) ? 10.0 : 0.0;
	outputs[TRIG3_OUTPUT].value = outPulse3.process(deltaT) ? 10.0 : 0.0;


	outputs[LFO_OUTPUT].value = lfo_volts;
	outputs[SEQ_OUTPUT].value = lfo_volts-3.0;
	outputs[DELAY_OUTPUT].value = delay_volts;

};

struct NumberDisplayWidget2 : TransparentWidget {

  float  *value;
  std::shared_ptr<Font> font;

  NumberDisplayWidget2() {
    font = Font::load(assetPlugin(plugin, "res/Segment7Standard.ttf"));
  };

  void draw(NVGcontext *vg) {
    // Background
    NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
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

    char display_string[10];

    sprintf(display_string,"%6.1f",*value);

    Vec textPos = Vec(6.0f, 17.0f);

    NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "~~~~~", NULL);

    textColor = nvgRGB(0xda, 0xe9, 0x29);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "\\\\\\\\\\", NULL);

    textColor = nvgRGB(0xf0, 0x00, 0x00);
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, display_string, NULL);
  }
};


struct FineMenuItem : MenuItem {

        BPMdetect *module;
        bool mfine;

        void onAction(EventAction &e) override {
                module->fine = mfine;
        };

        void step() override {
                rightText = (module->fine == mfine)? "✔" : "";
        };

};

struct NormalMenuItem : MenuItem {

        BPMdetect *module;
        bool mfine;

        void onAction(EventAction &e) override {
                module->fine = mfine;
        };

        void step() override {
                rightText = (module->fine != mfine)? "✔" : "";
        };

};

struct BPMdetectWidget : ModuleWidget {
	BPMdetectWidget(BPMdetect *module);
	json_t *toJsonData() ;
	void fromJsonData(json_t *root) ;
	Menu *createContextMenu() override;
};


Menu *BPMdetectWidget::createContextMenu() {

        Menu *menu = ModuleWidget::createContextMenu();

        MenuLabel *spacerLabel = new MenuLabel();
        menu->addChild(spacerLabel);

        BPMdetect *myModule = dynamic_cast<BPMdetect*>(module);
        assert(myModule);

        MenuLabel *modeLabel2 = new MenuLabel();
        modeLabel2->text = "Swing Range";
        menu->addChild(modeLabel2);

       

        FineMenuItem *fineMenuItem = new FineMenuItem();
        fineMenuItem->text = "Fine";
        fineMenuItem->module = myModule;
        fineMenuItem->mfine = true;
        menu->addChild(fineMenuItem);

        NormalMenuItem *normalMenuItem = new NormalMenuItem();
        normalMenuItem->text = "Legacy";
        normalMenuItem->module = myModule;
        normalMenuItem->mfine = false;
        menu->addChild(normalMenuItem);


        return menu;
};

BPMdetectWidget::BPMdetectWidget(BPMdetect *module) : ModuleWidget(module) {
	box.size = Vec(15*10, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin,"res/BPMdetect.svg")));
		addChild(panel);
	}

	const float column1 = 15;
	const float column2 = 61;
	const float column3 = 110;

	const float row1 = 84;
	const float row2 = 140;
	const float row3 = row2 + 60;
	const float row4 = row3 + 58;
	const float row5 = 316;


   addChild(Widget::create<MLScrew>(Vec(15, 0)));
	addChild(Widget::create<MLScrew>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<MLScrew>(Vec(15, 365)));
	addChild(Widget::create<MLScrew>(Vec(box.size.x-30, 365)));


	addInput(Port::create<MLPort>(Vec(column1+5,  row1+2), Port::INPUT, module, BPMdetect::GATE_INPUT));
   addParam(ParamWidget::create<SmallBlueMLKnob>(Vec(column2,   row1), module, BPMdetect::SMOOTH_PARAM, 0.0, 1.0, 0.5));
	addOutput(Port::create<MLPort>(Vec(column3-5,row1+2), Port::OUTPUT, module, BPMdetect::TRIG1_OUTPUT));

   addParam(ParamWidget::create<SmallBlueSnapMLKnob>(Vec(column1,  row2),    module, BPMdetect::MULT2_PARAM, 1.0, 8.0, 2.0));
   addParam(ParamWidget::create<SmallBlueMLKnob>(Vec(column2,  row2),    module, BPMdetect::SWING2_PARAM, 0.0, 2.0, 1.0));
	addOutput(Port::create<MLPort>(Vec(column3, row2+2), Port::OUTPUT, module, BPMdetect::TRIG2_OUTPUT));


   addParam(ParamWidget::create<SmallBlueSnapMLKnob>(Vec(column1,  row3),    module, BPMdetect::MULT3_PARAM, 1.0, 8.0, 3.0));
   addParam(ParamWidget::create<SmallBlueMLKnob>(Vec(column2,  row3),    module, BPMdetect::SWING3_PARAM, 0.0, 2.0, 1.0));
	addOutput(Port::create<MLPort>(Vec(column3, row3+2), Port::OUTPUT, module, BPMdetect::TRIG3_OUTPUT));

	addOutput(Port::create<MLPort>(Vec(column1, row4), Port::OUTPUT, module, BPMdetect::LFO_OUTPUT));
	addOutput(Port::create<MLPort>(Vec(column3, row4), Port::OUTPUT, module, BPMdetect::SEQ_OUTPUT));

   addParam(ParamWidget::create<SmallBlueSnapMLKnob>(Vec(column1,  row5), module, BPMdetect::DELAY1_PARAM, 1.0, 8.0, 1.0));
   addParam(ParamWidget::create<SmallBlueSnapMLKnob>(Vec(column2,  row5), module, BPMdetect::DELAY2_PARAM, 1.0, 8.0, 1.0));
	addOutput(Port::create<MLPort>(Vec(column3, row5), Port::OUTPUT, module, BPMdetect::DELAY_OUTPUT));

	NumberDisplayWidget2 *display = new NumberDisplayWidget2();
	display->box.pos = Vec(25,40);
	display->box.size = Vec(100, 20);
	display->value = &module->BPM;
	addChild(display);
}

} // namespace rack_plugin_ML_modules

using namespace rack_plugin_ML_modules;

RACK_PLUGIN_MODEL_INIT(ML_modules, BPMdetect) {
   Model *modelBPMdetect = Model::create<BPMdetect, BPMdetectWidget>("ML modules", "BPMdetect", "BPM Tools", UTILITY_TAG, CLOCK_TAG);
   return modelBPMdetect;
}

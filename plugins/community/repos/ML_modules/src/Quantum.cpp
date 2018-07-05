#include "ML_modules.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_ML_modules {

struct Quantum : Module {
	enum ParamIds {
		SEMI_1_PARAM,
		SEMI_2_PARAM,
		SEMI_3_PARAM,
		SEMI_4_PARAM,
		SEMI_5_PARAM,
		SEMI_6_PARAM,
		SEMI_7_PARAM,
		SEMI_8_PARAM,
		SEMI_9_PARAM,
		SEMI_10_PARAM,
		SEMI_11_PARAM,
		SEMI_12_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN_INPUT,
		TRANSPOSE_INPUT,
		NOTE_INPUT,
		SET_INPUT,
		RESET_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		TRIGGER_OUTPUT,
		GATE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		SEMI_1_LIGHT,
		SEMI_2_LIGHT,
		SEMI_3_LIGHT,
		SEMI_4_LIGHT,
		SEMI_5_LIGHT,
		SEMI_6_LIGHT,
		SEMI_7_LIGHT,
		SEMI_8_LIGHT,
		SEMI_9_LIGHT,
		SEMI_10_LIGHT,
		SEMI_11_LIGHT,
		SEMI_12_LIGHT,
		NUM_LIGHTS
	};

	enum Mode {
		LAST,
		CLOSEST_UP,
		CLOSEST_DOWN,
		UP,
		DOWN
	};

	Mode mode = LAST;

	Quantum() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) { reset(); };

	void step() override;
	PulseGenerator pulse;

	int last_octave=0, last_semi=0;

	bool semiState[12] = {};
	SchmittTrigger semiTriggers[12], setTrigger, resetTrigger;
	float semiLight[12] = {};

    void reset() override {
        for (int i = 0; i < 12; i++) {
            semiState[i] = false;
			semiLight[i] = 0.0;
        }
		last_octave = 0;
		last_semi   = 0;
	}

	void randomize() override {
		for (int i = 0; i<12; i++) {
			semiState[i] = (randomUniform() > 0.5);
			semiLight[i] = semiState[i]?1.0:0.0;
		};
		last_octave = 0;
		last_semi   = 0;

	}

    json_t *toJson() override {
        json_t *rootJ = json_object();

        json_t *scaleJ = json_array();
            for (int i = 0; i < 12; i++) {
                    json_t *semiJ = json_integer( (int) semiState[i]);
                    json_array_append_new(scaleJ, semiJ);
            }
            json_object_set_new(rootJ, "scale", scaleJ);
            json_object_set_new(rootJ, "mode", json_integer(mode));

        return rootJ;
    }

    void fromJson(json_t *rootJ) override {
        json_t *scaleJ = json_object_get(rootJ, "scale");
        for (int i = 0; i < 12; i++) {
            json_t *semiJ = json_array_get(scaleJ, i);
            semiState[i] = !!json_integer_value(semiJ);
			semiLight[i] = semiState[i]?1.0:0.0;
        }
        json_t *modeJ = json_object_get(rootJ, "mode");
        if(modeJ) mode = (Mode) json_integer_value(modeJ);
    }

	int modulo(int a, int b) {

		int r = a % b;
		return r < 0 ? r + b : r;

	}


};


void Quantum::step() {

	for(int i=0; i<12; i++) {

		if (semiTriggers[i].process(params[Quantum::SEMI_1_PARAM + i].value)) {
                        semiState[i] = !semiState[i];
                }
		lights[i].value = semiState[i]?1.0f:0.0f;
	}

	float gate = 0.f, trigger=0.f;
	float quantized = 0.f;

	float v=inputs[IN_INPUT].value;
	float t=inputs[TRANSPOSE_INPUT].normalize(0.0);

//	int octave   = floor(v);
//	int octave_t = floor(t);

	gate = mode==LAST? 0.0: 10.0;


	int semi_full   = round( 12.0f*v );
	int semi_t = round( 12.0f*t );

	int octave   = semi_full/12;

	int semi =semi_full % 12;

	if (semi<0) {
		semi+=12;
		octave-=1;
	}

	// transpose to shifted scale:

	int tmp_semi = modulo (semi-semi_t, 12) ;



   	if( inputs[RESET_INPUT].active ) {
		if( resetTrigger.process(inputs[RESET_INPUT].value) ) reset();
        };


	if( inputs[SET_INPUT].active ) {
		if( setTrigger.process(inputs[SET_INPUT].value ) ) {

			float n=inputs[NOTE_INPUT].normalize(0.0);
			int semi_n = round( 12.0*(n - 1.0*round(n)) ) - semi_t;
			if(semi_n<0) semi_n+=12;

			semiState[semi_n] = !semiState[semi_n];
			semiLight[semi_n] = semiState[semi_n]?1.0:0.0;
		}
	};


	if( semiState[tmp_semi] )
	{
		bool changed = !( (octave==last_octave) && (semi==last_semi));
		gate = 10.0f;
		quantized = 1.0f*octave + semi/12.0f;
		if(changed) pulse.trigger(0.001);
		last_octave = octave;
		last_semi   = semi;

	} else {

		if(mode==LAST) {
			quantized = 1.0f*last_octave + last_semi/12.0f;
		} else {

			int i_up   = 0;
			int i_down = 0;

			while( !semiState[ modulo(tmp_semi+i_up,  12) ] && i_up   < 12 ) i_up++;
		 	while( !semiState[ modulo(tmp_semi-i_down,12) ] && i_down < 12 ) i_down++;
				
			switch(mode) {	
				case UP: 			if (i_up   < 12) semi += i_up;
									else {semi = last_semi; octave = last_octave; gate = 0.0f;}
									break;

				case DOWN: 			if (i_down < 12) semi -= i_down; 
									else {semi = last_semi; octave = last_octave; gate = 0.0f;}
									break;

				case CLOSEST_UP:   	if (i_up<12 && i_down<12) semi = (i_up > i_down) ? (semi - i_down) : (semi + i_up); 
									else {semi = last_semi; octave = last_octave; gate = 0.0f;}
									break;

				case CLOSEST_DOWN: 	if (i_up<12 && i_down<12) semi = (i_down > i_up) ? (semi + i_up) : (semi - i_down); 
									else {semi = last_semi; octave = last_octave; gate = 0.0f;}
									break;

				case LAST:	
				default:		break;
			};
				
			if( semi > 11 ) {
				semi   -= 12;
				octave += 1;
			}

			if( semi < 0 ) {
				semi   += 12;
				octave -= 1;
			}


			bool changed = !( (octave==last_octave)&&(semi==last_semi));
			quantized = 1.0f*octave + semi/12.0f;
			if(changed) pulse.trigger(0.001f);
			last_octave = octave;
			last_semi   = semi;
		};
	};

	float gSampleRate = engineGetSampleRate();

	trigger = pulse.process(1.0/gSampleRate) ? 10.0f : 0.0f;


	outputs[OUT_OUTPUT].value = quantized;
	outputs[GATE_OUTPUT].value = gate - trigger;
	outputs[TRIGGER_OUTPUT].value = trigger;

}

struct QuantumModeItem : MenuItem {

        Quantum *quantum;
        Quantum::Mode mode;


        void onAction(EventAction &e) override {
                quantum->mode = mode;
        };

        void step() override {
                rightText = (quantum->mode == mode)? "✔" : "";
        };

};


struct QuantumWidget : ModuleWidget {
	QuantumWidget(Quantum *module);
	json_t *toJsonData() ;
	void fromJsonData(json_t *root) ;
	Menu *createContextMenu() override;
};

Menu *QuantumWidget::createContextMenu() {

        Menu *menu = ModuleWidget::createContextMenu();

        MenuLabel *spacerLabel = new MenuLabel();
        menu->addChild(spacerLabel);

        Quantum *quantum = dynamic_cast<Quantum*>(module);
        assert(quantum);

        MenuLabel *modeLabel = new MenuLabel();
        modeLabel->text = "Mode";
        menu->addChild(modeLabel);

        QuantumModeItem *last_Item = new QuantumModeItem();
        last_Item->text = "Last";
        last_Item->quantum = quantum;
        last_Item->mode = Quantum::LAST;
        menu->addChild(last_Item);

        QuantumModeItem *up_Item = new QuantumModeItem();
        up_Item->text = "Up";
        up_Item->quantum = quantum;
        up_Item->mode = Quantum::UP;
        menu->addChild(up_Item);

        QuantumModeItem *down_Item = new QuantumModeItem();
        down_Item->text = "Down";
        down_Item->quantum = quantum;
        down_Item->mode = Quantum::DOWN;
        menu->addChild(down_Item);

        QuantumModeItem *cl_up_Item = new QuantumModeItem();
        cl_up_Item->text = "Closest, up";
        cl_up_Item->quantum = quantum;
        cl_up_Item->mode = Quantum::CLOSEST_UP;
        menu->addChild(cl_up_Item);

        QuantumModeItem *cl_dn_Item = new QuantumModeItem();
        cl_dn_Item->text = "Closest, Down";
        cl_dn_Item->quantum = quantum;
        cl_dn_Item->mode = Quantum::CLOSEST_DOWN;
        menu->addChild(cl_dn_Item);

	return menu;
};

QuantumWidget::QuantumWidget(Quantum *module) : ModuleWidget(module) {
	box.size = Vec(15*8, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin,"res/Quantum.svg")));
		addChild(panel);
	}

	addChild(Widget::create<MLScrew>(Vec(15, 0)));
	addChild(Widget::create<MLScrew>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<MLScrew>(Vec(15, 365)));
	addChild(Widget::create<MLScrew>(Vec(box.size.x-30, 365)));

	addInput( Port::create<MLPort>(Vec(19, 42), Port::INPUT, module, Quantum::IN_INPUT));
	addOutput(Port::create<MLPort>(Vec(75, 42), Port::OUTPUT, module, Quantum::OUT_OUTPUT));

	addInput( Port::create<MLPort>(Vec(75, 90), Port::INPUT, module, Quantum::TRANSPOSE_INPUT));
	addOutput(Port::create<MLPort>(Vec(75, 140), Port::OUTPUT, module, Quantum::GATE_OUTPUT));
	addOutput(Port::create<MLPort>(Vec(75, 180), Port::OUTPUT, module, Quantum::TRIGGER_OUTPUT));

	addInput(Port::create<MLPort>(Vec(75, 226), Port::INPUT, module, Quantum::NOTE_INPUT));
	addInput(Port::create<MLPort>(Vec(75, 266), Port::INPUT, module, Quantum::SET_INPUT));
	addInput(Port::create<MLPort>(Vec(75, 312), Port::INPUT, module, Quantum::RESET_INPUT));

	static const float offset_x = 24;
	static const float offset_y = 333;

	for(int i=0; i<12; i++) {
		addParam(ParamWidget::create<ML_SmallLEDButton>(Vec(offset_x, -22*i+offset_y), module, Quantum::SEMI_1_PARAM + i, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MLSmallLight<GreenLight>>(Vec(offset_x+4, -22*i+4+offset_y), module, Quantum::SEMI_1_LIGHT+i));
	}

}

} // namespace rack_plugin_ML_modules

using namespace rack_plugin_ML_modules;

RACK_PLUGIN_MODEL_INIT(ML_modules, Quantum) {
   Model *modelQuantum = Model::create<Quantum, QuantumWidget>("ML modules", "Quantum", "Quantum", QUANTIZER_TAG);
   return modelQuantum;
}

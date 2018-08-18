#include "LFSR.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_alto777_LFSR {

/* at x1 works good except switching noise */
/* at x2 noise gone, but we get the tail end effect, a wee bit of f0 */
/* at 4. total rewrite ARGH! works good except I still get the tail effect */

//# define FADE_STEPS	20000	/* whoa! */
#define FADE_STEPS	2000

struct cheapFX : Module {
	enum ParamIds {
		ENUMS(FREQUENCY_PARAM, 2),
		ENUMS(SHAPE_PARAM, 2),

		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(FREQUENCY_CV_INPUT, 2),
		ENUMS(SHAPE_CV_INPUT, 2),
		ENUMS(TRIGGER_INPUT, 2),

		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(TRIANGLE_OUTPUT, 2),
		ENUMS(RECTANGLE_OUTPUT, 2),
		ENUMS(GATE_OUTPUT, 2),

		NUM_OUTPUTS
	};
	enum LightIds {

		NUM_LIGHTS
	};
	
	enum fadeFSMStates {
		OFF, START, COMING, ON, STOP, GOING
	};

	enum tweedFSMStates {
		NOT, INIT, TWEEDLE, FINI, BUSY
	};

	float phase[2] = {0.0f, 0.0f};
//	float previousGate[2] = {0.0f, 0.0f};
	SchmittTrigger eventTrigger[2];
	bool isGated[2] = {0, 0};
	bool phaseRolloverFlag[2] = {0, 0};
	bool retriggerFlag[2] = {0, 0};

	int tCount[2] = {0, 0};
	
	int  fadeCount[2];	/* for soft gate process */
	fadeFSMStates fadeState[2] = {OFF, OFF};
	tweedFSMStates tweedState[2] = {NOT, NOT};
	
	bool runnable = 0;

/* overkill for simple intialization, yet my best way of sandwiching it on in there */
	json_t *toJson() override {
		json_t *rootJ = json_object();
		if (!runnable) {
			for (int ii = 0; ii < 2; ii++) {
				phase[ii] = 0.0f;
// 			previousGate[ii] = 0.0f;
				isGated[ii] =0;
				tCount[ii] = 0;
				fadeState[ii] = OFF;
				tweedState[ii] = NOT;
				phaseRolloverFlag[ii] = 0;
				retriggerFlag[ii] = 0;
			}
			runnable = 1;
		}
		return (rootJ);
	}

	void fromJson(json_t *rootJ) override {
		for (int ii = 0; ii < 2; ii++) {
		
		}
	}
/*		json_t *rootJ = json_object();
		json_t *mstate = json_array();

		for (int i = 0; i < 2; i++)
			json_array_insert_new(mstate, i, json_integer((int) mState[i]));

		json_object_set_new(rootJ, "mstate", mstate);

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *mstate = json_object_get(rootJ, "mstate");

		if (mstate) {
			for (int i = 0; i < 2; i++) {
				json_t *wha = json_array_get(mstate, i);
				if (wha)
					mState[i] = json_integer_value(wha);
			}
		}
	}
*/
//	void accumulatePhaseAndOutput(int, float);
	cheapFX() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

struct myBoltA : SVGScrew {
	myBoltA() {
		sw->setSVG(SVG::load(assetPlugin(plugin, "res/myBoltA.svg")));
		box.size = sw->box.size;
	}
};

struct myBoltB : SVGScrew {
	myBoltB() {
		sw->setSVG(SVG::load(assetPlugin(plugin, "res/myBoltB.svg")));
		box.size = sw->box.size;
	}
};

struct myBoltC : SVGScrew {
	myBoltC() {
		sw->setSVG(SVG::load(assetPlugin(plugin, "res/myBoltC.svg")));
		box.size = sw->box.size;
	}
};

struct myBoltD : SVGScrew {
	myBoltD() {
		sw->setSVG(SVG::load(assetPlugin(plugin, "res/myBoltD.svg")));
		box.size = sw->box.size;
	}
};



//void  cheapFX::accumulatePhaseAndOutput(int ii, float deltaT) {}
//	accumulatePhaseAndOutput(ii);

void cheapFX::step() {
/* first - a square wave at frequency see Template.cpp */
	float deltaTime = engineGetSampleTime();

  for (int ii = 0; ii < 2; ii++) {	/* [ii] */
// ram pup or down the gate output for softer transitions  
/* inline because I am not confidant about scope and stuff. Sad. */
	switch (fadeState[ii]) {
	case OFF :			/* we dead, kill it again */
		outputs[GATE_OUTPUT + ii].value = 0.0f;
		break;

	case ON :			/* we full on */
		outputs[GATE_OUTPUT + ii].value = 10.0f;
		break;

	case START :		/* externally kicked */
		fadeCount[ii] = 0;
		fadeState[ii] = COMING;
		/* break not, get right to it */
	case COMING :
		if (fadeCount[ii] < FADE_STEPS) {
			++fadeCount[ii];
			outputs[GATE_OUTPUT + ii].value = (10.0f * fadeCount[ii]) / FADE_STEPS;
		}
		else fadeState[ii] = ON;
		break;

	case STOP :			/* also externally kicked */
		fadeState[ii] = GOING;
		/* break not, get right to it */
	case GOING :
		if (fadeCount[ii] > 0) {
			--fadeCount[ii];
			outputs[GATE_OUTPUT + ii].value = (10.0f * fadeCount[ii]) / FADE_STEPS;
		}
		else fadeState[ii] = OFF;
		break;
	}	
/**/

	switch (tweedState[ii]) {
	case INIT :
	/* when we start a tweedle cycle */
		phase[ii] = 0.0f;		/* restart the tweedle waveform */
		phaseRolloverFlag[ii] = 0;
		retriggerFlag[ii] = 0;
		fadeState[ii] = START;	/* send in the clowns */
		/* break not, get going right now */
		tweedState[ii] = TWEEDLE;

	case TWEEDLE :
	/* this carries out one full tweedle, maybe retriggers/stays on */
		if (eventTrigger[ii].process(inputs[TRIGGER_INPUT + ii].value))
			retriggerFlag[ii] = 1;	/* any trigger during buys another cycle */
	
		if (phaseRolloverFlag[ii]) {
			if (retriggerFlag[ii] || eventTrigger[ii].isHigh()) {
				retriggerFlag[ii] = 0;
				phaseRolloverFlag[ii] = 0;
				/* state remains the same */
			}
			else {	/* we've finished a tweedle and should stop it */
				tweedState[ii] = FINI;
				break;
			}
		}
		/* break not, NOT is just what we need to do anyway */
		/* but there's a slight chance to see a trigger, so if around that by state YUCK! */
	case NOT : 
		if (tweedState[ii] == NOT) {
			if (eventTrigger[ii].process(inputs[TRIGGER_INPUT + ii].value)) {
				tweedState[ii] = INIT;	/* and get right to it */
				break;
			}
		}
		if (1) {	/* cannot jump from switch statement to this case label */
		float pitch = params[FREQUENCY_PARAM + ii].value;
		pitch += inputs[FREQUENCY_CV_INPUT + ii].value;
		pitch = clamp(pitch, -4.0f, 4.0f);

		float freq = 2.04395f * powf(2.0f, pitch);		// LFO, right?

//--> don't advance phase in the middle of a fade out on the gate
// ARGH! too late, the phase done rolled over, that's what kicked us
//		if ((fadeState[ii] == ON) || (fadeState[ii] == OFF))

			phase[ii] += freq * deltaTime;

		if (phase[ii] >= 1.0f) {
			phaseRolloverFlag[ii] = 1;
			phase[ii] -= 1.0f;
		}
	/* YIKES !!! */
		if ((tweedState[ii] == TWEEDLE) || (tweedState[ii] == NOT)) {
			float x = params[SHAPE_PARAM + ii].value + inputs[SHAPE_CV_INPUT + ii].value * 0.1f;
			float t = phase[ii];		/* hey! */
			float y;

/* yuck. skip one sample update at phase rollover. FIGURE THIS AWAY! */
			if (!phaseRolloverFlag[ii])
				outputs[RECTANGLE_OUTPUT + ii].value = (phase[ii] > x) ? 0.0f : 10.0f;

			if (x >= 1.0f) y = t;
			else if(x <= 0.0f) y = 1.0f - t;
			else if (t < x) y = t / x;
			else y = (1.0f - t) / (1.0f - x);

			if (!phaseRolloverFlag[ii])
				outputs[TRIANGLE_OUTPUT + ii].value = 10.0f * y;
		}
		}	/* if (1) */
		break;

	case FINI :
			fadeState[ii] = STOP;
			tweedState[ii] = BUSY;
		break;

	case BUSY :
			if (fadeState[ii] == OFF) tweedState[ii] = NOT;
		break;
	}

/**/
  }
}

struct cheapFXWidget : ModuleWidget {
  cheapFXWidget(cheapFX *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/cheapFX.svg")));

	addChild(Widget::create<myBoltA>(Vec(0, 0)));
	addChild(Widget::create<myBoltB>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<myBoltD>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<myBoltC>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


	addParam(ParamWidget::create<RoundLargeBlackKnob>(mm2px(Vec(2.4f, 12.845f)), module, cheapFX::FREQUENCY_PARAM + 0, -3.0, 3.0, 0.0));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(mm2px(Vec(20.461f, 12.845f)), module, cheapFX::SHAPE_PARAM + 0, 0.0, 1.0, 0.5));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(mm2px(Vec(2.4f, 72.641f)), module, cheapFX::FREQUENCY_PARAM + 1, -3.0, 3.0, 0.0));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(mm2px(Vec(20.461f, 72.641f)), module, cheapFX::SHAPE_PARAM + 1, 0.0, 1.0, 0.5));

	addInput(Port::create<PJ301MPort>(mm2px(Vec(4.572f, 28.358f)), Port::INPUT, module, cheapFX::FREQUENCY_CV_INPUT + 0));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(22.449f, 28.359f)), Port::INPUT, module, cheapFX::SHAPE_CV_INPUT + 0));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(4.572f, 53.229f)), Port::INPUT, module, cheapFX::TRIGGER_INPUT + 0));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(4.572f, 88.154f)), Port::INPUT, module, cheapFX::FREQUENCY_CV_INPUT + 1));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(22.449f, 88.155f)), Port::INPUT, module, cheapFX::SHAPE_CV_INPUT + 1));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(4.572f, 113.024f)), Port::INPUT, module, cheapFX::TRIGGER_INPUT + 1));

	addOutput(Port::create<PJ301MPort>(mm2px(Vec(4.572f, 40.11f)), Port::OUTPUT, module, cheapFX::TRIANGLE_OUTPUT + 0));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(22.633f, 40.111f)), Port::OUTPUT, module, cheapFX::RECTANGLE_OUTPUT + 0));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(22.633f, 53.229f)), Port::OUTPUT, module, cheapFX::GATE_OUTPUT + 0));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(4.572f, 99.906f)), Port::OUTPUT, module, cheapFX::TRIANGLE_OUTPUT + 1));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(22.633f, 99.907f)), Port::OUTPUT, module, cheapFX::RECTANGLE_OUTPUT + 1));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(22.633f, 113.024f)), Port::OUTPUT, module, cheapFX::GATE_OUTPUT + 1));
  }
};

} // namespace rack_plugin_alto777_LFSR

using namespace rack_plugin_alto777_LFSR;

RACK_PLUGIN_MODEL_INIT(alto777_LFSR, cheapFX) {
   Model *modelcheapFX = Model::create<cheapFX, cheapFXWidget>("alto777_LFSR", "cheapFX", "Cheap F/X", LFO_TAG);
   return modelcheapFX;
}

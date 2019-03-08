#include "Bidoo.hpp"
#include "BidooComponents.hpp"
#include "dsp/digital.hpp"

using namespace std;

namespace rack_plugin_Bidoo {

struct MU : Module {
	enum ParamIds {
		BPM_PARAM,
		BPMFINE_PARAM,
		STEPLENGTH_PARAM,
		STEPLENGTHFINE_PARAM,
		NOTELENGTH_PARAM,
		STEPPROBA_PARAM,
		ALTEOSTEPPROBA_PARAM,
		NUMTRIGS_PARAM,
		DISTTRIGS_PARAM,
		CV_PARAM,
		START_PARAM,
		CVSTACK_PARAM,
		TRIGSTACK_PARAM,
		MUTE_PARAM,
		OFFSET_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		ACTIVATE_INPUT,
		INHIBIT_INPUT,
		GATEBRIDGE_INPUT,
		CVBRIDGE_INPUT,
		BPM_INPUT,
		NOTELENGTH_INPUT,
		CV_INPUT,
		NUMTRIGS_INPUT,
		DISTTRIGS_INPUT,
		OFFSET_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		EOSTEP_OUTPUT,
		ALTEOSTEP_OUTPUT,
		GATEBRIDGE_OUTPUT,
		CVBRIDGE_OUTPUT,
		BPM_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NORMEOS_LIGHT,
		ALTEOS_LIGHT,
		CVSTACK_LIGHT,
		TRIGSTACK_LIGHT,
		START_LIGHT,
		MUTE_LIGHT,
		GATE_LIGHT,
		NUM_LIGHTS = GATE_LIGHT + 3
	};
	PulseGenerator eosPulse;
	SchmittTrigger activateTrigger;
	SchmittTrigger inhibateTrigger;
	SchmittTrigger cvModeTrigger;
	SchmittTrigger trigModeTrigger;
	SchmittTrigger muteTrigger;
	SchmittTrigger startTrigger;
	bool isActive = false;
	int ticks = 0;
	int initTicks = 0;
	int gateTicks = 0;
	float bpm = 0.1f;
	int numTrigs = 1;
	int distRetrig = 0;
	bool play = false;
	bool alt = false;
	int count = 0;
	float displayLength = 0.0f;
	float displayNoteLength = 0.0f;
	float displayStepProba = 0.0f;
	float displayAltProba = 0.0f;
	float displayNumTrigs = 0.0f;
	float displayDistTrigs = 0.0f;
	float displayCV = 0.0f;
	float displayOffset = 0.0f;
	bool cvStack = false;
	bool trigStack = false;
	bool mute = false;

	MU() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}

	void step() override;
};

void MU::step() {
	const float invLightLambda = 13.333333333333333333333f;
	float invESR = 1 / engineGetSampleRate();

	bpm = inputs[BPM_INPUT].active ? rescale(clamp(inputs[BPM_INPUT].value,0.0f,10.0f),0.0f,10.0f,1.0f,800.99f) : (round(params[BPM_PARAM].value)+round(100*params[BPMFINE_PARAM].value)/100);
	displayLength = round(params[STEPLENGTH_PARAM].value) + round(params[STEPLENGTHFINE_PARAM].value*100) * 0.01f;

	bool wasActive = isActive;
	if (startTrigger.process(params[START_PARAM].value))
	{
		lights[START_LIGHT].value = 10.0f;
		isActive = true;
	}
	if (activateTrigger.process(inputs[ACTIVATE_INPUT].value))
	{
		isActive = true;
	}

	if (!wasActive && isActive) {
		initTicks = (displayLength/100) * engineGetSampleRate() * 60 / bpm;
		ticks = initTicks;
		count = 0;
		play = (randomUniform() + params[STEPPROBA_PARAM].value)>=1;
		alt = (randomUniform() + params[ALTEOSTEPPROBA_PARAM].value)>1;
		lights[GATE_LIGHT].value = 0.0f;
		lights[GATE_LIGHT+1].value = 0.0f;
		lights[GATE_LIGHT+2].value = 10.0f;
	}

	if (cvModeTrigger.process(params[CVSTACK_PARAM].value))
	{
		cvStack = !cvStack;
	}

	if (trigModeTrigger.process(params[TRIGSTACK_PARAM].value))
	{
		trigStack = !trigStack;
	}

	if (muteTrigger.process(params[MUTE_PARAM].value))
	{
		mute = !mute;
	}

	numTrigs = clamp(params[NUMTRIGS_PARAM].value + rescale(clamp(inputs[NUMTRIGS_INPUT].value,-10.0f,10.0f),-10.0f,10.0f,-64.0f,64.0f),0.0f,64.0f);
	distRetrig = clamp(params[DISTTRIGS_PARAM].value + rescale(clamp(inputs[DISTTRIGS_INPUT].value,-10.0f,10.0f),-10.0f,10.0f,-1.0f,1.0f), 0.0f,1.0f) * initTicks;
	gateTicks = initTicks * clamp(params[NOTELENGTH_PARAM].value + rescale(clamp(inputs[NOTELENGTH_INPUT].value,-10.0f,10.0f),-10.0f,10.0f,-1.0f,1.0f),0.0f,1.0f);
	displayNoteLength = clamp(params[NOTELENGTH_PARAM].value + rescale(clamp(inputs[NOTELENGTH_INPUT].value,-10.0f,10.0f),-10.0f,10.0f,-1.0f,1.0f),0.0f,1.0f) * 100;
	displayStepProba = params[STEPPROBA_PARAM].value * 100;
	displayAltProba = params[ALTEOSTEPPROBA_PARAM].value * 100;
	displayDistTrigs = params[DISTTRIGS_PARAM].value * 100;
	displayNumTrigs = numTrigs;
	displayOffset = clamp(params[OFFSET_PARAM].value + rescale(clamp(inputs[OFFSET_INPUT].value, 0.0f,10.0f), 0.0f, 10.0f, 0.0f,1.0f), 0.0f, 1.0f) * 100;

	if (inhibateTrigger.process(inputs[INHIBIT_INPUT].value))
	{
		isActive = false;
	}

	if (isActive && (ticks >= 0)) {
		if (ticks <= 0) {
			isActive = false;
			eosPulse.trigger(10 / engineGetSampleRate());
			outputs[GATEBRIDGE_OUTPUT].value = 0.0f;
			outputs[CVBRIDGE_OUTPUT].value = 0.0f;
		}
		else {
			int offset = displayOffset * initTicks * 0.01f;
			int mult = ((distRetrig > 0) && (count>offset))  ? ((count-offset) / distRetrig) : 0;
			if (play && (mult < numTrigs) && (count >= (offset + mult * distRetrig)) && (count <= (offset + (mult * distRetrig) + gateTicks))) {
				outputs[GATEBRIDGE_OUTPUT].value = mute ? 0.0f : 10.0f;
				lights[GATE_LIGHT].value = mute ? 0.0f : 10.0f;
				lights[GATE_LIGHT+1].value = 0.0f;
				lights[GATE_LIGHT+2].value = !mute ? 0.0f : 10.0f;
			}
			else {
				outputs[GATEBRIDGE_OUTPUT].value = (trigStack && !mute && (inputs[GATEBRIDGE_INPUT].value > 0.0f)) ? clamp(inputs[GATEBRIDGE_INPUT].value,0.0f,10.0f) : 0.0f;
				lights[GATE_LIGHT].value = 0.0f;
				lights[GATE_LIGHT+1].value = 0.0f;
				lights[GATE_LIGHT+2].value = 10.0f;
			}
			outputs[CVBRIDGE_OUTPUT].value = clamp(params[CV_PARAM].value + clamp(inputs[CV_INPUT].value,-10.0f,10.0f) + (cvStack ? inputs[CVBRIDGE_INPUT].value : 0.0f),0.0f,10.0f);
		}
		ticks--;
		count++;
	}
	else {
		outputs[GATEBRIDGE_OUTPUT].value = clamp(inputs[GATEBRIDGE_INPUT].value,0.0f,10.0f);
		outputs[CVBRIDGE_OUTPUT].value = inputs[CVBRIDGE_INPUT].value;
	}

	bool pulse = eosPulse.process(1 / engineGetSampleRate());

	outputs[EOSTEP_OUTPUT].value = !alt ? (pulse ? 10.0f : 0.0f) : 0.0f;
	outputs[ALTEOSTEP_OUTPUT].value = alt ? (pulse ? 10.0f : 0.0f) : 0.0f;
	outputs[BPM_OUTPUT].value = rescale(bpm,1.0f,800.99f,0.0f,10.0f);

	lights[NORMEOS_LIGHT].value = (!alt && pulse) ? 10.0f : (lights[NORMEOS_LIGHT].value - lights[NORMEOS_LIGHT].value * invLightLambda * invESR);
	lights[ALTEOS_LIGHT].value = (alt && pulse) ? 10.0f : (lights[ALTEOS_LIGHT].value - lights[ALTEOS_LIGHT].value * invLightLambda * invESR);
	lights[START_LIGHT].value = lights[START_LIGHT].value - lights[START_LIGHT].value * invLightLambda * invESR;
	if (outputs[GATEBRIDGE_OUTPUT].value == 0.0f) lights[GATE_LIGHT].value -= 4 * lights[GATE_LIGHT].value * invLightLambda * invESR;
	if (!isActive) lights[GATE_LIGHT+2].value -= 4 * lights[GATE_LIGHT+2].value * invLightLambda * invESR;
	if (!isActive) lights[GATE_LIGHT].value -= 4 * lights[GATE_LIGHT].value * invLightLambda * invESR;
	lights[MUTE_LIGHT].value = mute ? 10.0f : 0.0f;
	lights[CVSTACK_LIGHT].value = cvStack ? 10.0f : 0.0f;
	lights[TRIGSTACK_LIGHT].value = trigStack ? 10.0f : 0.0f;
}


struct LabelMICROWidget : TransparentWidget {
  float *value = NULL;
	const char *format = NULL;
	const char *header = "Have fun !!!";
  std::shared_ptr<Font> font;

  LabelMICROWidget() {
    font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
  };

  void draw(NVGcontext *vg) override
  {
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -2.0f);
		nvgFillColor(vg, YELLOW_BIDOO);
		nvgTextAlign(vg, NVG_ALIGN_LEFT);
		if (header) {
			nvgFontSize(vg, 12.0f);
			nvgText(vg, 0.0f, 0.0f, header, NULL);
		}
		if (value && format) {
			char display[128];
			snprintf(display, sizeof(display), format, *value);
			nvgFontSize(vg, 16.0f);
			nvgText(vg, 0.0f, 15.0f, display, NULL);
		}
  }
};

struct BidooBlueTrimpotWithDisplay : BidooBlueTrimpot {
	LabelMICROWidget *lblDisplay = NULL;
	float *valueForDisplay = NULL;
	const char *format = NULL;
	const char *header = NULL;

	void onMouseEnter(EventMouseEnter &e) override {
			if (lblDisplay && valueForDisplay && format) {
				lblDisplay->value = valueForDisplay;
				lblDisplay->format = format;
			}
			if (lblDisplay && header) lblDisplay->header = header;
			BidooBlueTrimpot::onMouseEnter(e);
		}
};

struct TinyPJ301MPortWithDisplay : TinyPJ301MPort {
	LabelMICROWidget *lblDisplay = NULL;
	float *valueForDisplay = NULL;
	const char *format = NULL;
	const char *header = NULL;

	void onMouseEnter(EventMouseEnter &e) override {
		if (lblDisplay && valueForDisplay && format) {
			lblDisplay->value = valueForDisplay;
			lblDisplay->format = format;
		}
		if (lblDisplay && header) lblDisplay->header = header;
			TinyPJ301MPort::onMouseEnter(e);
		}
};

struct MUWidget : ModuleWidget {
	MUWidget(MU *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/MU.svg")));

		LabelMICROWidget *display = new LabelMICROWidget();
		display->box.pos = Vec(4,37);
		addChild(display);

		addChild(ModuleLightWidget::create<SmallLight<RedGreenBlueLight>>(Vec(40, 15), module, MU::GATE_LIGHT));

		addParam(ParamWidget::create<LEDButton>(Vec(5, 5), module, MU::START_PARAM, 0.0f, 10.0f,  0.0f));
		addChild(ModuleLightWidget::create<SmallLight<BlueLight>>(Vec(11, 11), module, MU::START_LIGHT));

		addParam(ParamWidget::create<LEDButton>(Vec(52, 5), module, MU::MUTE_PARAM, 0.0f, 10.0f,  0.0f));
		addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(58, 11), module, MU::MUTE_LIGHT));

		static const float portX0[2] = {14.0f, 41.0f};
		static const float portY0[5] = {60.0f, 83.0f, 106.0f, 145.0f, 166.0f};

		static const float portX1[2] = {15.0f, 45.0f};
		static const float portY1[8] = {191.0f, 215.0f, 239.0f, 263.0f, 287.0f, 311.0f, 335.0f, 359.0f};

		BidooBlueTrimpotWithDisplay* bpm = ParamWidget::create<BidooBlueTrimpotWithDisplay>(Vec(portX0[0], portY0[0]), module, MU::BPM_PARAM, 1.0f, 800.0f, 117.0f);
		bpm->lblDisplay = display;
		bpm->valueForDisplay = &module->bpm;
		bpm->format = "%2.2f";
		bpm->header = "BPM";
		addParam(bpm);
		BidooBlueTrimpotWithDisplay* bpmFine =  ParamWidget::create<BidooBlueTrimpotWithDisplay>(Vec(portX0[0], portY0[1]), module, MU::BPMFINE_PARAM, 0.0f, 0.99f, 0.0f);
		bpmFine->lblDisplay = display;
		bpmFine->valueForDisplay = &module->bpm;
		bpmFine->format = "%2.2f";
		bpmFine->header = "BPM";
		addParam(bpmFine);

		BidooBlueTrimpotWithDisplay* stepLength =  ParamWidget::create<BidooBlueTrimpotWithDisplay>(Vec(portX0[1], portY0[0]), module, MU::STEPLENGTH_PARAM, 0.0f, 1600.0f, 100.0f);
		stepLength->lblDisplay = display;
		stepLength->valueForDisplay = &module->displayLength;
		stepLength->format = "%2.2f %%";
		stepLength->header = "Step len.";
		addParam(stepLength);
		BidooBlueTrimpotWithDisplay* stepLengthFine =  ParamWidget::create<BidooBlueTrimpotWithDisplay>(Vec(portX0[1], portY0[1]), module, MU::STEPLENGTHFINE_PARAM, -0.5f, 0.5f, 0.0f);
		stepLengthFine->lblDisplay = display;
		stepLengthFine->valueForDisplay = &module->displayLength;
		stepLengthFine->format = "%2.2f %%";
		stepLengthFine->header = "Step len.";
		addParam(stepLengthFine);

		BidooBlueTrimpotWithDisplay* noteLength =  ParamWidget::create<BidooBlueTrimpotWithDisplay>(Vec(portX0[0], portY0[2]), module, MU::NOTELENGTH_PARAM, 0.0f, 1.0f, 1.0f);
		noteLength->lblDisplay = display;
		noteLength->valueForDisplay = &module->displayNoteLength;
		noteLength->format = "%2.2f %%";
		noteLength->header = "Trigs len.";
		addParam(noteLength);

		BidooBlueTrimpotWithDisplay* offset =  ParamWidget::create<BidooBlueTrimpotWithDisplay>(Vec(portX0[1], portY0[2]), module, MU::OFFSET_PARAM, 0.0f, 1.0f, 0.0f);
		offset->lblDisplay = display;
		offset->valueForDisplay = &module->displayOffset;
		offset->format = "%2.2f %%";
		offset->header = "Trigs offset";
		addParam(offset);

		BidooBlueTrimpotWithDisplay* cv =  ParamWidget::create<BidooBlueTrimpotWithDisplay>(Vec(portX0[0]+14, portY0[2]+19), module, MU::CV_PARAM, 0.0f, 10.0f, 0.0f);
		cv->lblDisplay = display;
		cv->valueForDisplay = &module->params[MU::CV_PARAM].value;
		cv->format = "%2.2f V";
		cv->header = "CV";
		addParam(cv);

		BidooBlueTrimpotWithDisplay* stepProb =  ParamWidget::create<BidooBlueTrimpotWithDisplay>(Vec(portX0[0], portY0[3]), module, MU::STEPPROBA_PARAM, 0.0f, 1.0f, 1.0f);
		stepProb->lblDisplay = display;
		stepProb->valueForDisplay = &module->displayStepProba;
		stepProb->format = "%2.2f %%";
		stepProb->header = "Step prob.";
		addParam(stepProb);

		BidooBlueTrimpotWithDisplay* altOutProb =  ParamWidget::create<BidooBlueTrimpotWithDisplay>(Vec(portX0[1], portY0[3]), module, MU::ALTEOSTEPPROBA_PARAM, 0.0f, 1.0f, 0.0f);
		altOutProb->lblDisplay = display;
		altOutProb->valueForDisplay = &module->displayAltProba;
		altOutProb->format = "%2.2f %%";
		altOutProb->header = "Alt out prob.";
		addParam(altOutProb);

		BidooBlueTrimpotWithDisplay* numTrig =  ParamWidget::create<BidooBlueTrimpotWithDisplay>(Vec(portX0[0], portY0[4]), module, MU::NUMTRIGS_PARAM, 1.0f, 64.0f, 1.0f);
		numTrig->lblDisplay = display;
		numTrig->valueForDisplay = &module->displayNumTrigs;
		numTrig->format = "%2.0f";
		numTrig->header = "Trigs count";
		addParam(numTrig);

		BidooBlueTrimpotWithDisplay* distTrig =  ParamWidget::create<BidooBlueTrimpotWithDisplay>(Vec(portX0[1], portY0[4]), module, MU::DISTTRIGS_PARAM, 0.0f, 1.0f, 1.0f);
		distTrig->lblDisplay = display;
		distTrig->valueForDisplay = &module->displayDistTrigs;
		distTrig->format = "%2.2f %%";
		distTrig->header = "Trigs Dist.";
		addParam(distTrig);

		TinyPJ301MPortWithDisplay* bpmIn = Port::create<TinyPJ301MPortWithDisplay>(Vec(portX1[0], portY1[0]), Port::INPUT, module, MU::BPM_INPUT);
		bpmIn->lblDisplay = display;
		bpmIn->valueForDisplay = &module->inputs[MU::BPM_INPUT].value;
		bpmIn->format = "%2.2f V";
		bpmIn->header = "BPM";
		addInput(bpmIn);
		TinyPJ301MPortWithDisplay* bpmOut = Port::create<TinyPJ301MPortWithDisplay>(Vec(portX1[1], portY1[0]), Port::OUTPUT, module, MU::BPM_OUTPUT);
		bpmOut->lblDisplay = display;
		bpmOut->valueForDisplay = &module->outputs[MU::BPM_OUTPUT].value;
		bpmOut->format = "%2.2f V";
		bpmOut->header = "BPM";
		addOutput(bpmOut);

		TinyPJ301MPortWithDisplay* activateIn = Port::create<TinyPJ301MPortWithDisplay>(Vec(portX1[0], portY1[1]), Port::INPUT, module, MU::ACTIVATE_INPUT);
		activateIn->lblDisplay = display;
		activateIn->valueForDisplay = &module->inputs[MU::ACTIVATE_INPUT].value;
		activateIn->format = "%2.2f V";
		activateIn->header = "Step start";
		addInput(activateIn);
		TinyPJ301MPortWithDisplay* activateOUT = Port::create<TinyPJ301MPortWithDisplay>(Vec(portX1[1], portY1[1]), Port::OUTPUT, module, MU::EOSTEP_OUTPUT);
		activateOUT->lblDisplay = display;
		activateOUT->valueForDisplay = &module->outputs[MU::BPM_OUTPUT].value;
		activateOUT->format = "%2.2f V";
		activateOUT->header = "Step end";
		addOutput(activateOUT);

		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(Vec(portX1[1]-11, portY1[1]+5), module, MU::NORMEOS_LIGHT));

		TinyPJ301MPortWithDisplay* activateAltOUT = Port::create<TinyPJ301MPortWithDisplay>(Vec(portX1[1], portY1[2]), Port::OUTPUT, module, MU::ALTEOSTEP_OUTPUT);
		activateAltOUT->lblDisplay = display;
		activateAltOUT->valueForDisplay = &module->outputs[MU::ALTEOSTEP_OUTPUT].value;
		activateAltOUT->format = "%2.2f V";
		activateAltOUT->header = "Alt step end";
		addOutput(activateAltOUT);

		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(Vec(portX1[1]-11, portY1[2]+5), module, MU::ALTEOS_LIGHT));

		TinyPJ301MPortWithDisplay* inhibitIn = Port::create<TinyPJ301MPortWithDisplay>(Vec(portX1[0], portY1[2]), Port::INPUT, module, MU::INHIBIT_INPUT);
		inhibitIn->lblDisplay = display;
		inhibitIn->valueForDisplay = &module->inputs[MU::INHIBIT_INPUT].value;
		inhibitIn->format = "%2.2f V";
		inhibitIn->header = "Inhibit step";
		addInput(inhibitIn);

		TinyPJ301MPortWithDisplay* noteLengthIn = Port::create<TinyPJ301MPortWithDisplay>(Vec(portX1[0], portY1[3]), Port::INPUT, module, MU::NOTELENGTH_INPUT);
		noteLengthIn->lblDisplay = display;
		noteLengthIn->valueForDisplay = &module->inputs[MU::NOTELENGTH_INPUT].value;
		noteLengthIn->format = "%2.2f V";
		noteLengthIn->header = "Trigs len.";
		addInput(noteLengthIn);

		TinyPJ301MPortWithDisplay* offsetIn = Port::create<TinyPJ301MPortWithDisplay>(Vec(portX1[1], portY1[3]), Port::INPUT, module, MU::OFFSET_INPUT);
		offsetIn->lblDisplay = display;
		offsetIn->valueForDisplay = &module->inputs[MU::OFFSET_INPUT].value;
		offsetIn->format = "%2.2f V";
		offsetIn->header = "Offset mod";
		addInput(offsetIn);

		TinyPJ301MPortWithDisplay* cvIn = Port::create<TinyPJ301MPortWithDisplay>(Vec(portX1[0]+14, portY1[4]), Port::INPUT, module, MU::CV_INPUT);
		cvIn->lblDisplay = display;
		cvIn->valueForDisplay = &module->inputs[MU::CV_INPUT].value;
		cvIn->format = "%2.2f V";
		cvIn->header = "CV mod";
		addInput(cvIn);

		TinyPJ301MPortWithDisplay* numTrigsIn = Port::create<TinyPJ301MPortWithDisplay>(Vec(portX1[0], portY1[5]), Port::INPUT, module, MU::NUMTRIGS_INPUT);
		numTrigsIn->lblDisplay = display;
		numTrigsIn->valueForDisplay = &module->inputs[MU::NUMTRIGS_INPUT].value;
		numTrigsIn->format = "%2.2f V";
		numTrigsIn->header = "Trigs count";
		addInput(numTrigsIn);

		TinyPJ301MPortWithDisplay* distTrigsIn = Port::create<TinyPJ301MPortWithDisplay>(Vec(portX1[1], portY1[5]), Port::INPUT, module, MU::DISTTRIGS_INPUT);
		distTrigsIn->lblDisplay = display;
		distTrigsIn->valueForDisplay = &module->inputs[MU::DISTTRIGS_INPUT].value;
		distTrigsIn->format = "%2.2f V";
		distTrigsIn->header = "Trigs Dist.";
		addInput(distTrigsIn);

		TinyPJ301MPortWithDisplay* gateBridgeIn = Port::create<TinyPJ301MPortWithDisplay>(Vec(portX1[0], portY1[6]), Port::INPUT, module, MU::GATEBRIDGE_INPUT);
		gateBridgeIn->lblDisplay = display;
		gateBridgeIn->valueForDisplay = &module->inputs[MU::GATEBRIDGE_INPUT].value;
		gateBridgeIn->format = "%2.2f V";
		gateBridgeIn->header = "Gate";
		addInput(gateBridgeIn);

		TinyPJ301MPortWithDisplay* gateBridgeOut = Port::create<TinyPJ301MPortWithDisplay>(Vec(portX1[1], portY1[6]), Port::OUTPUT, module, MU::GATEBRIDGE_OUTPUT);
		gateBridgeOut->lblDisplay = display;
		gateBridgeOut->valueForDisplay = &module->outputs[MU::GATEBRIDGE_OUTPUT].value;
		gateBridgeOut->format = "%2.2f V";
		gateBridgeOut->header = "Gate";
		addOutput(gateBridgeOut);

		addParam(ParamWidget::create<MiniLEDButton>(Vec(portX1[1]-11, portY1[6]+5), module, MU::TRIGSTACK_PARAM, 0.0f, 10.0f,  0.0f));
		addChild(ModuleLightWidget::create<SmallLight<BlueLight>>(Vec(portX1[1]-11, portY1[6]+5), module, MU::TRIGSTACK_LIGHT));

		TinyPJ301MPortWithDisplay* cvBridgeIn = Port::create<TinyPJ301MPortWithDisplay>(Vec(portX1[0], portY1[7]), Port::INPUT, module, MU::CVBRIDGE_INPUT);
		cvBridgeIn->lblDisplay = display;
		cvBridgeIn->valueForDisplay = &module->inputs[MU::CVBRIDGE_INPUT].value;
		cvBridgeIn->format = "%2.2f V";
		cvBridgeIn->header = "CV";
		addInput(cvBridgeIn);

		TinyPJ301MPortWithDisplay* cvBridgeOut = Port::create<TinyPJ301MPortWithDisplay>(Vec(portX1[1], portY1[7]), Port::OUTPUT, module, MU::CVBRIDGE_OUTPUT);
		cvBridgeOut->lblDisplay = display;
		cvBridgeOut->valueForDisplay = &module->outputs[MU::CVBRIDGE_OUTPUT].value;
		cvBridgeOut->format = "%2.2f V";
		cvBridgeOut->header = "CV";
		addOutput(cvBridgeOut);

		addParam(ParamWidget::create<MiniLEDButton>(Vec(portX1[1]-11, portY1[7]+5), module, MU::CVSTACK_PARAM, 0.0f, 10.0f,  0.0f));
		addChild(ModuleLightWidget::create<SmallLight<BlueLight>>(Vec(portX1[1]-11, portY1[7]+5), module, MU::CVSTACK_LIGHT));
  }
};

} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, MU) {
   Model *modelMU= Model::create<MU, MUWidget>("Bidoo", "µ", "µ synced pulse generator", SEQUENCER_TAG);
   return modelMU;
}

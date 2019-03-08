#include "Bidoo.hpp"
#include "BidooComponents.hpp"
#include "dsp/ringbuffer.hpp"

using namespace std;

namespace rack_plugin_Bidoo {

struct BAR : Module {
	enum ParamIds {
		THRESHOLD_PARAM,
		RATIO_PARAM,
		ATTACK_PARAM,
		RELEASE_PARAM,
		KNEE_PARAM,
		MAKEUP_PARAM,
		MIX_PARAM,
		LOOKAHEAD_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN_L_INPUT,
		IN_R_INPUT,
		SC_L_INPUT,
		SC_R_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_L_OUTPUT,
		OUT_R_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};
	DoubleRingBuffer<float,16384> vu_L_Buffer, vu_R_Buffer;
	DoubleRingBuffer<float,512> rms_L_Buffer, rms_R_Buffer;
	float runningVU_L_Sum = 1e-6f, runningRMS_L_Sum = 1e-6f, rms_L = -96.3f, vu_L = -96.3f, peakL = -96.3f;
	float runningVU_R_Sum = 1e-6f, runningRMS_R_Sum = 1e-6f, rms_R = -96.3f, vu_R = -96.3f, peakR = -96.3f;
	float in_L_dBFS = 1e-6f;
	float in_R_dBFS = 1e-6f;
	float dist = 0.0f, gain = 1.0f, gaindB = 1.0f, ratio = 1.0f, threshold = 1.0f, knee = 0.0f;
	float attackTime = 0.0f, releaseTime = 0.0f, makeup = 1.0f, previousPostGain = 1.0f, mix = 1.0f;
	int indexVU = 0, indexRMS = 0, lookAheadWriteIndex=0;
	int maxIndexVU = 0, maxIndexRMS = 0, maxLookAheadWriteIndex=0;
	int lookAhead;
	float buffL[20000] = {0.0f}, buffR[20000] = {0.0f};
	BAR() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}

	void step() override;

};

void BAR::step() {
	if (indexVU>=16384) {
		runningVU_L_Sum -= *vu_L_Buffer.startData();
		runningVU_R_Sum -= *vu_R_Buffer.startData();
		vu_L_Buffer.startIncr(1);
		vu_R_Buffer.startIncr(1);
		indexVU--;
	}

	if (indexRMS>=512) {
		runningRMS_L_Sum -= *rms_L_Buffer.startData();
		runningRMS_R_Sum -= *rms_R_Buffer.startData();
		rms_L_Buffer.startIncr(1);
		rms_R_Buffer.startIncr(1);
		indexRMS--;
	}

	indexVU++;
	indexRMS++;

	buffL[lookAheadWriteIndex]=inputs[IN_L_INPUT].value;
	buffR[lookAheadWriteIndex]=inputs[IN_R_INPUT].value;

	if (!inputs[SC_L_INPUT].active && inputs[IN_L_INPUT].active)
		in_L_dBFS = max(20.0f*log10((abs(inputs[IN_L_INPUT].value)+1e-6f) * 0.2f), -96.3f);
	else if (inputs[SC_L_INPUT].active)
		in_L_dBFS = max(20.0f*log10((abs(inputs[SC_L_INPUT].value)+1e-6f) * 0.2f), -96.3f);
	else
		in_L_dBFS = -96.3f;

	if (!inputs[SC_R_INPUT].active && inputs[IN_R_INPUT].active)
		in_R_dBFS = max(20.0f*log10((abs(inputs[IN_R_INPUT].value)+1e-6f) * 0.2f), -96.3f);
	else if (inputs[SC_R_INPUT].active)
		in_R_dBFS = max(20.0f*log10((abs(inputs[SC_R_INPUT].value)+1e-6f) * 0.2f), -96.3f);
	else
		in_R_dBFS = -96.3f;

	float data_L = in_L_dBFS*in_L_dBFS;

	if (!vu_L_Buffer.full()) {
		vu_L_Buffer.push(data_L);
	}
	if (!rms_L_Buffer.full()) {
		rms_L_Buffer.push(data_L);
	}

	float data_R = in_R_dBFS*in_R_dBFS;
	if (!vu_R_Buffer.full()) {
		vu_R_Buffer.push(data_R);
	}
	if (!rms_R_Buffer.full()) {
		rms_R_Buffer.push(data_R);
	}

	runningVU_L_Sum += data_L;
	runningRMS_L_Sum += data_L;
	runningVU_R_Sum += data_R;
	runningRMS_R_Sum += data_R;
	rms_L = clamp(-1 * sqrtf(runningRMS_L_Sum/512), -96.3f,0.0f);
	vu_L = clamp(-1 * sqrtf(runningVU_L_Sum/16384), -96.3f,0.0f);
	rms_R = clamp(-1 * sqrtf(runningRMS_R_Sum/512), -96.3f,0.0f);
	vu_R = clamp(-1 * sqrtf(runningVU_R_Sum/16384), -96.3f,0.0f);
	threshold = params[THRESHOLD_PARAM].value;
	attackTime = params[ATTACK_PARAM].value;
	releaseTime = params[RELEASE_PARAM].value;
	ratio = params[RATIO_PARAM].value;
	knee = params[KNEE_PARAM].value;
	makeup = params[MAKEUP_PARAM].value;

	if (in_L_dBFS>peakL)
		peakL=in_L_dBFS;
	else
		peakL -= 50.0f/engineGetSampleRate();

	if (in_R_dBFS>peakR)
		peakR=in_R_dBFS;
	else
		peakR -= 50.0f/engineGetSampleRate();

	float slope = 1.0f/ratio-1.0f;
	float maxIn = max(in_L_dBFS,in_R_dBFS);
	float dist = maxIn-threshold;
	float gcurve = 0.0f;

	if (dist<-1.0f*knee/2.0f)
		gcurve = maxIn;
	else if ((dist > -1.0f * knee * 0.5f) && (dist < knee * 0.5f)) {
		gcurve = maxIn + slope * pow(dist + knee *0.5f, 2.0f) / (2.0f * knee);
	} else {
		gcurve = maxIn + slope * dist;
	}

	float preGain = gcurve - maxIn;
	float postGain = 0.0f;
	float cAtt = exp(-1.0f/(attackTime*engineGetSampleRate() * 0.001f));
	float cRel = exp(-1.0f/(releaseTime*engineGetSampleRate() * 0.001f));

	if (preGain>previousPostGain) {
		postGain = cAtt * previousPostGain + (1.0f-cAtt) * preGain;
	} else {
		postGain = cRel * previousPostGain + (1.0f-cRel) * preGain;
	}

	previousPostGain = postGain;
	gaindB = makeup + postGain;
	gain = pow(10.0f, gaindB/20.0f);

	mix = params[MIX_PARAM].value;
	lookAhead = params[LOOKAHEAD_PARAM].value;

	int nbSamples = clamp(floor(lookAhead * attackTime * engineGetSampleRate() * 0.000001f),0.0f,19999.0f);
	int readIndex;
	if (lookAheadWriteIndex-nbSamples>=0)
	  readIndex = (lookAheadWriteIndex-nbSamples)%20000;
	else {
		readIndex = 20000 - abs(lookAheadWriteIndex-nbSamples);
	}

	outputs[OUT_L_OUTPUT].value = buffL[readIndex] * (gain*mix + (1.0f - mix));
	outputs[OUT_R_OUTPUT].value = buffR[readIndex] * (gain*mix + (1.0f - mix));

	lookAheadWriteIndex = (lookAheadWriteIndex+1)%20000;
}

struct BARDisplay : TransparentWidget {
	BAR *module;
	std::shared_ptr<Font> font;
	BARDisplay() {
		font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
	}

void draw(NVGcontext *vg) override {
	float height = 150.0f;
	float width = 15.0f;
	float spacer = 3.0f;
	float vuL = rescale(module->vu_L,-97.0f,0.0f,0.0f,height);
	float rmsL = rescale(module->rms_L,-97.0f,0.0f,0.0f,height);
	float vuR = rescale(module->vu_R,-97.0f,0.0f,0.0f,height);
	float rmsR = rescale(module->rms_R,-97.0f,0.0f,0.0f,height);
	float threshold = rescale(module->threshold,0.0f,-97.0f,0.0f,height);
	float gain = rescale(1-(module->gaindB-module->makeup),-97.0f,0.0f,97.0f,0.0f);
	float makeup = rescale(module->makeup,0.0f,60.0f,0.0f,60.0f);
	float peakL = rescale(module->peakL,0.0f,-97.0f,0.0f,height);
	float peakR = rescale(module->peakR,0.0f,-97.0f,0.0f,height);
	float inL = rescale(module->in_L_dBFS,-97.0f,0.0f,0.0f,height);
	float inR = rescale(module->in_R_dBFS,-97.0f,0.0f,0.0f,height);
	nvgSave(vg);
	nvgStrokeWidth(vg, 0.0f);
	nvgBeginPath(vg);
	nvgFillColor(vg, BLUE_BIDOO);
	nvgRoundedRect(vg,0.0f,height-vuL,width,vuL,0.0f);
	nvgRoundedRect(vg,3.0f*(width+spacer),height-vuR,width,vuR,0.0f);
	nvgFill(vg);
	nvgClosePath(vg);
	nvgBeginPath(vg);
	nvgFillColor(vg, LIGHTBLUE_BIDOO);
	nvgRoundedRect(vg,width+spacer,height-rmsL,width,rmsL,0.0f);
	nvgRoundedRect(vg,2.0f*(width+spacer),height-rmsR,width,rmsR,0.0f);
	nvgFill(vg);
	nvgClosePath(vg);

	nvgFillColor(vg, RED_BIDOO);
	nvgBeginPath(vg);
	nvgRoundedRect(vg,width+spacer,peakL,width,2.0f,0.0f);
	nvgRoundedRect(vg,2.0f*(width+spacer),peakR,width,2.0f,0.0f);
	nvgFill(vg);
	nvgClosePath(vg);

	nvgFillColor(vg, ORANGE_BIDOO);
	nvgBeginPath(vg);
	if (inL>rmsL+3.0f)
		nvgRoundedRect(vg,width+spacer,height-inL+1.0f,width,inL-rmsL-2.0f,0.0f);
	if (inR>rmsR+3)
		nvgRoundedRect(vg,2.0f*(width+spacer),height-inR+1.0f,width,inR-rmsR-2.0f,0.0f);
	nvgFill(vg);
	nvgClosePath(vg);

	nvgStrokeWidth(vg, 0.5f);
	nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));
	nvgStrokeColor(vg, nvgRGBA(255, 255, 255, 255));
	nvgBeginPath(vg);
	nvgMoveTo(vg, width+spacer+5.0f, threshold);
	nvgLineTo(vg, 3.0f*width+2.0f*spacer-5.0f, threshold);
	//nvgRoundedRect(vg,22,threshold+50,22,2,0);
	{
		nvgMoveTo(vg, width+spacer, threshold-3.0f);
		nvgLineTo(vg, width+spacer, threshold+3.0f);
		nvgLineTo(vg, width+spacer+5.0f, threshold);
		nvgLineTo(vg, width+spacer, threshold-3.0f);
		nvgMoveTo(vg, 3.0f*width+2.0f*spacer, threshold-3.0f);
		nvgLineTo(vg, 3*width+2*spacer, threshold+3.0f);
		nvgLineTo(vg, 3.0f*width+2.0f*spacer-5.0f, threshold);
		nvgLineTo(vg, 3.0f*width+2.0f*spacer, threshold-3.0f);
	}
	nvgClosePath(vg);
	nvgStroke(vg);
	nvgFill(vg);

	float offset = 11.0f;
	nvgStrokeWidth(vg, 0.5f);
	nvgFillColor(vg, YELLOW_BIDOO);
	nvgStrokeColor(vg, YELLOW_BIDOO);
	nvgBeginPath(vg);
	nvgRoundedRect(vg,4.0f*(width+spacer)+offset,70.0f,width,-gain-makeup,0.0f);
	nvgMoveTo(vg, 5.0f*(width+spacer)+7.0f+offset, 70.0f-3.0f);
	nvgLineTo(vg, 5.0f*(width+spacer)+7.0f+offset, 70.0f+3.0f);
	nvgLineTo(vg, 5.0f*(width+spacer)+2.0f+offset, 70.0f);
	nvgLineTo(vg, 5.0f*(width+spacer)+7.0f+offset, 70.0f-3.0f);
	nvgClosePath(vg);
	nvgStroke(vg);
	nvgFill(vg);

	char tTresh[128],tRatio[128],tAtt[128],tRel[128],tKnee[128],tMakeUp[128],tMix[128],tLookAhead[128];
	snprintf(tTresh, sizeof(tTresh), "%2.1f", module->threshold);
	snprintf(tRatio, sizeof(tTresh), "%2.0f:1", module->ratio);
	snprintf(tAtt, sizeof(tTresh), "%1.0f/%1.0f", module->attackTime,module->releaseTime);
	snprintf(tRel, sizeof(tTresh), "%3.0f", module->releaseTime);
	snprintf(tKnee, sizeof(tTresh), "%2.1f", module->knee);
	snprintf(tMakeUp, sizeof(tTresh), "%2.1f", module->makeup);
	snprintf(tMix, sizeof(tTresh), "%1.0f/%1.0f", (1-module->mix)*100,module->mix*100);
	snprintf(tLookAhead, sizeof(tTresh), "%3i", module->lookAhead);
	nvgFontSize(vg, 14.0f);
	nvgFontFaceId(vg, font->handle);
	nvgTextLetterSpacing(vg, -2.0f);
	nvgFillColor(vg, YELLOW_BIDOO);
	nvgTextAlign(vg, NVG_ALIGN_CENTER);
	nvgText(vg, 8.0f, height+31.0f, tTresh, NULL);
	nvgText(vg, 50.0f, height+31.0f, tRatio, NULL);
	nvgText(vg, 96.0f, height+31.0f, tAtt, NULL);
	nvgText(vg, 8.0f, height+63.0f, tKnee, NULL);
	nvgText(vg, 40.0f, height+63.0f, tMakeUp, NULL);
	nvgText(vg, 75.0f, height+63.0f, tMix, NULL);
	nvgText(vg, 107.0f, height+63.0f, tLookAhead, NULL);
	nvgRestore(vg);
}
};


struct BARWidget : ModuleWidget {
	BARWidget(BAR *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/BAR.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		BARDisplay *display = new BARDisplay();
		display->module = module;
		display->box.pos = Vec(12.0f, 40.0f);
		display->box.size = Vec(110.0f, 70.0f);
		addChild(display);

		addParam(ParamWidget::create<BidooBlueTrimpot>(Vec(10.0f,265.0f), module, BAR::THRESHOLD_PARAM, -93.6f, 0.0f, 0.0f));
		addParam(ParamWidget::create<BidooBlueTrimpot>(Vec(42.0f,265.0f), module, BAR::RATIO_PARAM, 1.0f, 20.0f, 0.0f));
		addParam(ParamWidget::create<BidooBlueTrimpot>(Vec(74.0f,265.0f), module, BAR::ATTACK_PARAM, 1.0f, 100.0f, 10.0f));
		addParam(ParamWidget::create<BidooBlueTrimpot>(Vec(106.0f,265.0f), module, BAR::RELEASE_PARAM, 1.0f, 300.0f, 10.0f));
		addParam(ParamWidget::create<BidooBlueTrimpot>(Vec(10.0f,291.0f), module, BAR::KNEE_PARAM, 0.0f, 24.0f, 6.0f));
		addParam(ParamWidget::create<BidooBlueTrimpot>(Vec(42.0f,291.0f), module, BAR::MAKEUP_PARAM, 0.0f, 60.0f, 0.0f));
		addParam(ParamWidget::create<BidooBlueTrimpot>(Vec(74.0f,291.0f), module, BAR::MIX_PARAM, 0.0f, 1.0f, 1.0f));
		addParam(ParamWidget::create<BidooBlueTrimpot>(Vec(106.0f,291.0f), module, BAR::LOOKAHEAD_PARAM, 0.0f, 200.0f, 0.0f));
	 	//Changed ports opposite way around
		addInput(Port::create<TinyPJ301MPort>(Vec(24.0f, 319.0f), Port::INPUT, module, BAR::IN_L_INPUT));
		addInput(Port::create<TinyPJ301MPort>(Vec(24.0f, 339.0f), Port::INPUT, module, BAR::IN_R_INPUT));
		addInput(Port::create<TinyPJ301MPort>(Vec(66.0f, 319.0f), Port::INPUT, module, BAR::SC_L_INPUT));
		addInput(Port::create<TinyPJ301MPort>(Vec(66.0f, 339.0f), Port::INPUT, module, BAR::SC_R_INPUT));
		addOutput(Port::create<TinyPJ301MPort>(Vec(109.0f, 319.0f),Port::OUTPUT, module, BAR::OUT_L_OUTPUT));
		addOutput(Port::create<TinyPJ301MPort>(Vec(109.0f, 339.0f),Port::OUTPUT, module, BAR::OUT_R_OUTPUT));
	}
};

} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, BAR) {
   Model *modelBAR = Model::create<BAR, BARWidget>("Bidoo", "baR", "bAR compressor", DYNAMICS_TAG);
   return modelBAR;
}

#include "global_pre.hpp"
#include "Bidoo.hpp"
#include "BidooComponents.hpp"
#include "dsp/samplerate.hpp"
#include "dsp/decimator.hpp"
#include "dsp/filter.hpp"
#include "global_ui.hpp"

using namespace std;

namespace rack_plugin_Bidoo {

extern float sawTable[2048];
extern float triTable[2048];

struct CLACOS : Module {
	enum ParamIds {
		PITCH_PARAM,
    FINE_PARAM,
		DIST_X_PARAM,
    DIST_Y_PARAM = DIST_X_PARAM + 4,
		WAVEFORM_PARAM = DIST_Y_PARAM + 4,
		MODE_PARAM = WAVEFORM_PARAM + 4,
		SYNC_PARAM,
		FM_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		PITCH_INPUT,
    SYNC_INPUT,
		DIST_X_INPUT,
    DIST_Y_INPUT = DIST_X_INPUT + 4,
		WAVEFORM_INPUT = DIST_Y_PARAM + 4,
		FM_INPUT = WAVEFORM_INPUT + 4,
		NUM_INPUTS
	};
	enum OutputIds {
		MAIN_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};
	bool analog = false;
	bool soft = false;
	float lastSyncValue = 0.0f;
	float phase = 0.0f;
	float phaseDist = 0.0f;
	float phaseDistX[4] = {0.5f};
	float phaseDistY[4] = {0.5f};
	int waveFormIndex[4] = {0};
	float freq;
	float pitch;
	bool syncEnabled = false;
	bool syncDirection = false;
	int index = 0, prevIndex = 3;

	Decimator<16, 16> mainDecimator;
	RCFilter sqrFilter;
	RCFilter mainFilter;

	// For analog detuning effect
	float pitchSlew = 0.0f;
	int pitchSlewIndex = 0;

	float sinBuffer[16] = {0.0f};
	float triBuffer[16] = {0.0f};
	float sawBuffer[16] = {0.0f};
	float sqrBuffer[16] = {0.0f};
	float mainBuffer[16] = {0.0f};

	void setPitch(float pitchKnob, float pitchCv) {
		// Compute frequency
		pitch = pitchKnob;
		if (analog) {
			// Apply pitch slew
			const float pitchSlewAmount = 3.0f;
			pitch += pitchSlew * pitchSlewAmount;
		}
		else {
			// Quantize coarse knob if digital mode
			pitch = roundf(pitch);
		}
		pitch += pitchCv;
		// Note C3
		freq = 261.626f * powf(2.0f, pitch / 12.0f);
	}

	void process(float deltaTime, float syncValue) {
		if (analog) {
			// Adjust pitch slew
			if (++pitchSlewIndex > 32) {
				const float pitchSlewTau = 100.0f; // Time constant for leaky integrator in seconds
				pitchSlew += (randomNormal() - pitchSlew / pitchSlewTau) / engineGetSampleRate();
				pitchSlewIndex = 0.0f;
			}
		}

		// Advance phase
		float deltaPhase = clamp(freq * deltaTime, 1e-6f, 0.5f);

		// Detect sync
		int syncIndex = -1; // Index in the oversample loop where sync occurs [0, OVERSAMPLE)
		float syncCrossing = 0.0f; // Offset that sync occurs [0.0, 1.0)
		if (syncEnabled) {
			syncValue -= 0.01f;
			if (syncValue > 0.0f && lastSyncValue <= 0.0f) {
				float deltaSync = syncValue - lastSyncValue;
				syncCrossing = 1.0f - syncValue / deltaSync;
				syncCrossing *= 16;
				syncIndex = (int)syncCrossing;
				syncCrossing -= syncIndex;
			}
			lastSyncValue = syncValue;
		}

		if (syncDirection)
			deltaPhase *= -1.0f;

		sqrFilter.setCutoff(40.0f * deltaTime);
		mainFilter.setCutoff(22000.0f * deltaTime);

		for (int i = 0; i < 16; i++) {
			if (syncIndex == i) {
				if (soft) {
					syncDirection = !syncDirection;
					deltaPhase *= -1.0f;
				}
				else {
					phase = 0.0f;
					phaseDist = 0.0f;
				}
			}

			if (phase<0.25f)
				index = 0;
			else if ((phase>=0.25f) && (phase<0.50f))
				index = 1;
			else if ((phase>=0.50f) && (phase<0.75f))
				index = 2;
			else
				index = 3;

			if (analog) {
				// Quadratic approximation of sine, slightly richer harmonics
				if (phaseDist < 0.5f)
					sinBuffer[i] = 1.0f - 16.0f * powf(phaseDist - 0.25f, 2.0f);
				else
					sinBuffer[i] = -1.0f + 16.0f * powf(phaseDist - 0.75f, 2.0f);
				sinBuffer[i] *= 1.08f;
			}
			else {
				sinBuffer[i] = sinf(2.0f * M_PI * phaseDist);
			}
			if (analog) {
				triBuffer[i] = 1.25f * interpolateLinear(triTable, phaseDist * 2047.f);
			}
			else {
				if (phaseDist < 0.25f)
					triBuffer[i] = 4.0f * phaseDist;
				else if (phaseDist < 0.75f)
					triBuffer[i] = 2.0f - 4.0f * phaseDist;
				else
					triBuffer[i] = -4.0f + 4.0f * phaseDist;
			}
			if (analog) {
				sawBuffer[i] = 1.66f * interpolateLinear(sawTable, phaseDist * 2047.f);
			}
			else {
				if (phaseDist < 0.5f)
					sawBuffer[i] = 2.0f * phaseDist;
				else
					sawBuffer[i] = -2.0f + 2.0f * phaseDist;
			}
			sqrBuffer[i] = (phaseDist < 0.5f) ? 1.0f : -1.0f;
			if (analog) {
				// Simply filter here
				sqrFilter.process(sqrBuffer[i]);
				sqrBuffer[i] = 0.71f * sqrFilter.highpass();
			}

			waveFormIndex[index] = inputs[WAVEFORM_INPUT+index].active ? clamp((int)(rescale(inputs[WAVEFORM_INPUT+index].value,0.0f,10.0f,0.0f,3.0f)),0,3) : clamp((int)(params[WAVEFORM_PARAM+index].value),0,3);
			if (waveFormIndex[index] == 0)
				mainBuffer[i]=sinBuffer[i];
			else if (waveFormIndex[index] == 1)
				mainBuffer[i]=triBuffer[i];
			else if (waveFormIndex[index] == 2)
				mainBuffer[i]=sawBuffer[i];
			else if (waveFormIndex[index] == 3)
				mainBuffer[i]=sqrBuffer[i];

			mainFilter.process(mainBuffer[i]);
			mainBuffer[i]=mainFilter.lowpass();

			// Advance phase
			phase += deltaPhase / 16.0f;
			phase = eucmod(phase, 1.0f);
			if (phase<=0.25f)
				index = 0;
			else if ((phase>0.25f) && (phase<=0.5f))
				index = 1;
			else if ((phase>0.5f) && (phase<=0.75f))
				index = 2;
			else
				index = 3;

			if (prevIndex!=index){
				phaseDist = phase;
				prevIndex = index;
			}
			else {
				if (rescale(phase,index*0.25f,(index+1)*0.25f,0.0f,1.0f)<=(phaseDistX[index]))
					phaseDist = min(phaseDist + (deltaPhase / 16.0f) * phaseDistY[index]/phaseDistX[index], (index+1)*0.25f);
				else
					phaseDist = min(phaseDist + (deltaPhase / 16.0f) * (1-phaseDistY[index])/(1-phaseDistX[index]), (index+1)*0.25f);
				phaseDist = eucmod(phaseDist, 1.0f);
			}
		}
	}

	float main() {
		return mainDecimator.process(mainBuffer);
	}
	float light() {
		return sinf(2.0f*M_PI * phase);
	}

	CLACOS() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}

	void step() override;

	json_t *toJson() override {
		json_t *rootJ = json_object();
		for (int i=0; i<4; i++) {
			json_object_set_new(rootJ, ("phaseDistX" + to_string(i)).c_str(), json_real(phaseDistX[i]));
			json_object_set_new(rootJ, ("phaseDistY" + to_string(i)).c_str(), json_real(phaseDistY[i]));
		}
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		for (int i=0; i<4; i++) {
			json_t *phaseDistXJ = json_object_get(rootJ, ("phaseDistX" + to_string(i)).c_str());
			if (phaseDistXJ) {
				phaseDistX[i] = json_number_value(phaseDistXJ);
			}
			json_t *phaseDistYJ = json_object_get(rootJ, ("phaseDistY" + to_string(i)).c_str());
			if (phaseDistYJ) {
				phaseDistY[i] = json_number_value(phaseDistYJ);
			}
		}
	}

	void randomize() override {
		for (int i=0; i<4; i++) {
			if ((!inputs[CLACOS::DIST_X_INPUT+i].active) && (!inputs[CLACOS::DIST_X_INPUT+i].active)) {
				phaseDistX[i] = randomUniform();
				phaseDistY[i] = randomUniform();
			}
		}
	}

	void reset() override {
		for (int i=0; i<4; i++) {
			if ((!inputs[CLACOS::DIST_X_INPUT+i].active) && (!inputs[CLACOS::DIST_X_INPUT+i].active)) {
				phaseDistX[i] = 0.5;
				phaseDistY[i] = 0.5;
			}
		}
	}
};

void CLACOS::step() {
	analog = params[MODE_PARAM].value > 0.0f;
	soft = params[SYNC_PARAM].value <= 0.0f;

	for (int i=0; i<4; i++) {
	if (inputs[DIST_X_INPUT+i].active)
		phaseDistX[i] = rescale(clamp(inputs[DIST_X_INPUT+i].value,0.0f,10.0f),0.0f,10.0f,0.01f,0.99f);

	if (inputs[DIST_Y_INPUT+i].active)
		phaseDistY[i] = rescale(clamp(inputs[DIST_Y_INPUT+i].value,0.0f,10.0f),0.0f,10.0f,0.01f,0.99f);
	}

	float pitchFine = 3.0f * quadraticBipolar(params[FINE_PARAM].value);
	float pitchCv = 12.0f * inputs[PITCH_INPUT].value;
	if (inputs[FM_INPUT].active) {
		pitchCv += quadraticBipolar(params[FM_PARAM].value) * 12.0f * inputs[FM_INPUT].value;
	}
	setPitch(params[PITCH_PARAM].value, pitchFine + pitchCv);
	syncEnabled = inputs[SYNC_INPUT].active;

	process(1.0f / engineGetSampleRate(), inputs[SYNC_INPUT].value);

	// Set output
	outputs[MAIN_OUTPUT].value = 5.0f * main();
}

struct CLACOSDisplay : TransparentWidget {
	CLACOS *module;
	int frame = 0;
	string waveForm;
	int segmentNumber = 0;
	float initX = 0.0f;
	float initY = 0.0f;
	float dragX = 0.0f;
	float dragY = 0.0f;

CLACOSDisplay() {}

void onDragStart(EventDragStart &e) override {
	dragX = RACK_PLUGIN_UI_RACKWIDGET->lastMousePos.x;
	dragY = RACK_PLUGIN_UI_RACKWIDGET->lastMousePos.y;
}

void onDragMove(EventDragMove &e) override {
	if ((!module->inputs[CLACOS::DIST_X_INPUT + segmentNumber].active) && (!module->inputs[CLACOS::DIST_X_INPUT + segmentNumber].active)) {
		float newDragX = RACK_PLUGIN_UI_RACKWIDGET->lastMousePos.x;
		float newDragY = RACK_PLUGIN_UI_RACKWIDGET->lastMousePos.y;
		module->phaseDistX[segmentNumber] = rescale(clamp(initX+(newDragX-dragX),0.0f,70.0f), 0.0f, 70.0f, 0.01f,0.99f);
		module->phaseDistY[segmentNumber]  = rescale(clamp(initY-(newDragY-dragY),0.0f,70.0f), 0.0f, 70.0f, 0.01f,0.99f);
	}
}

void onMouseDown(EventMouseDown &e) override {
	if (e.button == 0) {
		e.consumed = true;
		e.target = this;
		initX = e.pos.x;
		initY = 70.0f - e.pos.y;
	}
}

void draw(NVGcontext *vg) override {
	if (++frame >= 4) {
		frame = 0;
		if (module->waveFormIndex[segmentNumber] == 0)
			waveForm="SIN";
		else if (module->waveFormIndex[segmentNumber] == 1)
			waveForm="TRI";
		else if (module->waveFormIndex[segmentNumber] == 2)
			waveForm="SAW";
		else if (module->waveFormIndex[segmentNumber] == 3)
			waveForm="SQR";
	}
	nvgFontSize(vg, 10.0f);
	nvgFillColor(vg, nvgRGBA(42, 87, 117, 255));
	nvgText(vg, 12.0f, 79.0f, waveForm.c_str(), NULL);

	// Draw ref lines
	nvgStrokeColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0x80));
	{
		nvgBeginPath(vg);
		nvgMoveTo(vg, 0.0f, 35.0f);
		nvgLineTo(vg, 70.0f, 35.0f);
		nvgMoveTo(vg, 35.0f, 0.0f);
		nvgLineTo(vg, 35.0f, 70.0f);
		nvgClosePath(vg);
	}
	nvgStroke(vg);

	// Draw phase distortion
	nvgStrokeColor(vg, nvgRGBA(42, 87, 117, 255));
	{
		nvgBeginPath(vg);
		nvgMoveTo(vg, 0.0f, 70.0f);
		nvgLineTo(vg, (int)(rescale(module->phaseDistX[segmentNumber], 0.0f,1.0f,0.0f,70.0f)) , 70.0f - (int)(rescale(module->phaseDistY[segmentNumber], 0.0f,1.0f,0.01f,70.0f)));
		nvgMoveTo(vg, (int)(rescale(module->phaseDistX[segmentNumber], 0.0f,1.0f,0.0f,70.0f)) , 70.0f - (int)(rescale(module->phaseDistY[segmentNumber], 0.0f,1.0f,0.01f,70.0f)));
		nvgLineTo(vg, 70.0f, 0.0f);
		nvgClosePath(vg);
	}
	nvgStroke(vg);
}
};


struct CLACOSWidget : ModuleWidget {
	CLACOSWidget(CLACOS *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/CLACOS.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		for (int i = 0; i < 4; i++)
		{
			{
				CLACOSDisplay *display = new CLACOSDisplay();
				display->module = module;
				display->segmentNumber = i;
				display->box.pos = Vec(3.0f + 74.0f * (i%2), 113.0f + 102.0f * round(i/2));
				display->box.size = Vec(70.0f, 70.0f);
				addChild(display);
				addParam(ParamWidget::create<BidooBlueTrimpot>(Vec(2.0f + 74.0f * (i%2), 194.0f + 102.0f * round(i/2)), module, CLACOS::WAVEFORM_PARAM + i, 0.0f, 3.0f, 0.0f));
				addInput(Port::create<TinyPJ301MPort>(Vec(22.0f + 74.0f * (i%2), 196.0f + 102.0f * round(i/2)), Port::INPUT, module, CLACOS::WAVEFORM_INPUT + i));
				addInput(Port::create<TinyPJ301MPort>(Vec(40.0f + 74.0f * (i%2), 196.0f + 102.0f * round(i/2)), Port::INPUT, module, CLACOS::DIST_X_INPUT + i));
				addInput(Port::create<TinyPJ301MPort>(Vec(57.0f + 74.0f * (i%2), 196.0f + 102.0f * round(i/2)), Port::INPUT, module, CLACOS::DIST_Y_INPUT + i));
			}
		}

		addParam(ParamWidget::create<CKSS>(Vec(15.0f, 80.0f), module, CLACOS::MODE_PARAM, 0.0f, 1.0f, 1.0f));
		addParam(ParamWidget::create<CKSS>(Vec(119.0f, 80.0f), module, CLACOS::SYNC_PARAM, 0.0f, 1.0f, 1.0f));

		addParam(ParamWidget::create<BidooLargeBlueKnob>(Vec(57.0f, 45.0f), module, CLACOS::PITCH_PARAM, -54.0f, 54.0f, 0.0f));
	  addParam(ParamWidget::create<BidooBlueTrimpot>(Vec(114.0f,45.0f), module, CLACOS::FINE_PARAM, -1.0f, 1.0f, 0.0f));
		addParam(ParamWidget::create<BidooBlueTrimpot>(Vec(18.0f,45.0f), module, CLACOS::FM_PARAM, 0.0f, 1.0f, 0.0f));
		addInput(Port::create<TinyPJ301MPort>(Vec(38.0f, 83.0f), Port::INPUT, module, CLACOS::FM_INPUT));

	  addInput(Port::create<PJ301MPort>(Vec(11.0f, 330.0f), Port::INPUT, module, CLACOS::PITCH_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(45.0f, 330.0f), Port::INPUT, module, CLACOS::SYNC_INPUT));
		addOutput(Port::create<PJ301MPort>(Vec(114.0f, 330.0f), Port::OUTPUT, module, CLACOS::MAIN_OUTPUT));
	}
};

} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, CLACOS) {
   Model *modelCLACOS = Model::create<CLACOS, CLACOSWidget>("Bidoo", "clACos", "clACos oscillator", OSCILLATOR_TAG);
   return modelCLACOS;
}

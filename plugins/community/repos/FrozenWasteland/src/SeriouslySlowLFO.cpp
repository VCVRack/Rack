#include "FrozenWasteland.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_FrozenWasteland {

struct SeriouslySlowLFO : Module {
	enum ParamIds {
		TIME_BASE_PARAM,
		DURATION_PARAM,
		FM_CV_ATTENUVERTER_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		FM_INPUT,
		RESET_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SIN_OUTPUT,
		TRI_OUTPUT,
		SAW_OUTPUT,
		SQR_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		MINUTES_LIGHT,
		HOURS_LIGHT,
		DAYS_LIGHT,
		WEEKS_LIGHT,
		MONTHS_LIGHT,
		NUM_LIGHTS
	};

struct LowFrequencyOscillator {
	double phase = 0.0;
	float pw = 0.5;
	float freq = 1.0;
	bool offset = false;
	bool invert = false;
	SchmittTrigger resetTrigger;
	LowFrequencyOscillator() {}
	void setPitch(float pitch) {
		pitch = fminf(pitch, 8.0);
		freq = powf(2.0, pitch);
	}
	void setFrequency(float frequency) {
		freq = frequency;
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
	void hardReset()
	{
		phase = 0.0;
	}

	void step(float dt) {
		//float deltaPhase = fminf(freq * dt, 0.5);
		double deltaPhase = freq * dt;
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
	float progress() {
		return phase;
	}
};




	LowFrequencyOscillator oscillator;
	SchmittTrigger sumTrigger;
	float duration = 0.0;
	int timeBase = 0;


	SeriouslySlowLFO() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

	json_t *toJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "timeBase", json_integer((int) timeBase));
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *sumJ = json_object_get(rootJ, "timeBase");
		if (sumJ)
			timeBase = json_integer_value(sumJ);
	}

	void reset() override {
		timeBase = 0;
	}

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};


void SeriouslySlowLFO::step() {

	if (sumTrigger.process(params[TIME_BASE_PARAM].value)) {
		timeBase = (timeBase + 1) % 5;
		oscillator.hardReset();
	}

	float numberOfSeconds = 0;
	switch(timeBase) {
		case 0 :
			numberOfSeconds = 60; // Minutes
			break;
		case 1 :
			numberOfSeconds = 3600; // Hours
			break;
		case 2 :
			numberOfSeconds = 86400; // Days
			break;
		case 3 :
			numberOfSeconds = 604800; // Weeks
			break;
		case 4 :
			numberOfSeconds = 259200; // Months
			break;
	}

	duration = params[DURATION_PARAM].value;
	if(inputs[FM_INPUT].active) {
		duration +=inputs[FM_INPUT].value * params[FM_CV_ATTENUVERTER_PARAM].value;
	}
	duration = clamp(duration,1.0f,100.0f);

	oscillator.setFrequency(1.0 / (duration * numberOfSeconds));
	oscillator.step(1.0 / engineGetSampleRate());
	if(inputs[RESET_INPUT].active) {
		oscillator.setReset(inputs[RESET_INPUT].value);
	}


	outputs[SIN_OUTPUT].value = 5.0 * oscillator.sin();
	outputs[TRI_OUTPUT].value = 5.0 * oscillator.tri();
	outputs[SAW_OUTPUT].value = 5.0 * oscillator.saw();
	outputs[SQR_OUTPUT].value =  5.0 * oscillator.sqr();

	for(int lightIndex = 0;lightIndex<5;lightIndex++)
	{
		lights[lightIndex].value = lightIndex != timeBase ? 0.0 : 1.0;
	}
}

struct SSLFOProgressDisplay : TransparentWidget {
	SeriouslySlowLFO *module;
	int frame = 0;
	std::shared_ptr<Font> font;



	SSLFOProgressDisplay() {
		font = Font::load(assetPlugin(plugin, "res/fonts/01 Digit.ttf"));
	}

	void drawProgress(NVGcontext *vg, float phase)
	{
		//float startArc = (-M_PI) / 2.0; we know this rotates 90 degrees to top
		//float endArc = (phase * M_PI) - startArc;
		const float rotate90 = (M_PI) / 2.0;
		float startArc = 0 - rotate90;
		float endArc = (phase * M_PI * 2) - rotate90;

		// Draw indicator
		nvgFillColor(vg, nvgRGBA(0xff, 0xff, 0x20, 0xff));
		{
			nvgBeginPath(vg);
			nvgArc(vg,109.8,194.5,35,startArc,endArc,NVG_CW);
			nvgLineTo(vg,109.8,194.5);
			nvgClosePath(vg);
		}
		nvgFill(vg);
	}

	void drawDuration(NVGcontext *vg, Vec pos, float duration) {
		nvgFontSize(vg, 28);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -2);

		nvgFillColor(vg, nvgRGBA(0x00, 0xff, 0x00, 0xff));
		char text[128];
		snprintf(text, sizeof(text), " % #6.1f", duration);
		nvgText(vg, pos.x + 22, pos.y, text, NULL);
	}

	void draw(NVGcontext *vg) override {

		drawProgress(vg,module->oscillator.progress());
		drawDuration(vg, Vec(0, box.size.y - 140), module->duration);
	}
};

struct SeriouslySlowLFOWidget : ModuleWidget {
	SeriouslySlowLFOWidget(SeriouslySlowLFO *module);
};

SeriouslySlowLFOWidget::SeriouslySlowLFOWidget(SeriouslySlowLFO *module) : ModuleWidget(module) {
	box.size = Vec(15*10, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/SeriouslySlowLFO.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH - 12, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH + 12, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH-12, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH + 12, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	{
		SSLFOProgressDisplay *display = new SSLFOProgressDisplay();
		display->module = module;
		display->box.pos = Vec(0, 0);
		display->box.size = Vec(box.size.x, 220);
		addChild(display);
	}

	addParam(ParamWidget::create<CKD6>(Vec(10, 240), module, SeriouslySlowLFO::TIME_BASE_PARAM, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(75, 90), module, SeriouslySlowLFO::DURATION_PARAM, 1.0, 100.0, 1.0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(41, 121), module, SeriouslySlowLFO::FM_CV_ATTENUVERTER_PARAM, -1.0, 1.0, 0.0));


	addInput(Port::create<PJ301MPort>(Vec(40, 93), Port::INPUT, module, SeriouslySlowLFO::FM_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(63, 272), Port::INPUT, module, SeriouslySlowLFO::RESET_INPUT));

	addOutput(Port::create<PJ301MPort>(Vec(11, 320), Port::OUTPUT, module, SeriouslySlowLFO::SIN_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(45, 320), Port::OUTPUT, module, SeriouslySlowLFO::TRI_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(80, 320), Port::OUTPUT, module, SeriouslySlowLFO::SAW_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(114, 320), Port::OUTPUT, module, SeriouslySlowLFO::SQR_OUTPUT));

	addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(10, 168), module, SeriouslySlowLFO::MINUTES_LIGHT));
	addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(10, 183), module, SeriouslySlowLFO::HOURS_LIGHT));
	addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(10, 198), module, SeriouslySlowLFO::DAYS_LIGHT));
	addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(10, 213), module, SeriouslySlowLFO::WEEKS_LIGHT));
	addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(10, 228), module, SeriouslySlowLFO::MONTHS_LIGHT));
}

} // namespace rack_plugin_FrozenWasteland

using namespace rack_plugin_FrozenWasteland;

RACK_PLUGIN_MODEL_INIT(FrozenWasteland, SeriouslySlowLFO) {
   Model *modelSeriouslySlowLFO = Model::create<SeriouslySlowLFO, SeriouslySlowLFOWidget>("Frozen Wasteland", "SeriouslySlowLFO", "Seriously Slow LFO", LFO_TAG);
   return modelSeriouslySlowLFO;
}

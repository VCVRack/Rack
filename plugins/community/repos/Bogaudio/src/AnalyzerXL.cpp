
#include "string.h"

#include "AnalyzerXL.hpp"

#define RANGE_KEY "range"
#define RANGE_DB_KEY "range_db"
#define SMOOTH_KEY "smooth"
#define QUALITY_KEY "quality"
#define QUALITY_GOOD_KEY "good"
#define QUALITY_HIGH_KEY "high"
#define QUALITY_ULTRA_KEY "ultra"
#define WINDOW_KEY "window"
#define WINDOW_NONE_KEY "none"
#define WINDOW_HAMMING_KEY "hamming"
#define WINDOW_KAISER_KEY "kaiser"

void AnalyzerXL::onReset() {
	_modulationStep = modulationSteps;
	_range = 0.0f;
	_smooth = 0.25f;
	_quality = AnalyzerCore::QUALITY_GOOD;
	_window = AnalyzerCore::WINDOW_KAISER;
	setCoreParams();
	_core.resetChannels();
}

void AnalyzerXL::onSampleRateChange() {
	_modulationStep = modulationSteps;
	setCoreParams();
	_core.resetChannels();
}

void AnalyzerXL::setCoreParams() {
	_rangeMinHz = 0.0f;
	_rangeMaxHz = 0.5f * engineGetSampleRate();
	if (_range < 0.0f) {
		_rangeMaxHz *= 1.0f + _range;
	}
	else if (_range > 0.0f) {
		_rangeMinHz = _range * _rangeMaxHz;
	}

	float smooth = _smooth / (_core.size() / (_core._overlap * engineGetSampleRate()));
	int averageN = std::max(1, (int)roundf(smooth));
	_core.setParams(averageN, _quality, _window);
}

json_t* AnalyzerXL::toJson() {
	json_t* root = json_object();
	json_object_set_new(root, RANGE_KEY, json_real(_range));
	json_object_set_new(root, RANGE_DB_KEY, json_real(_rangeDb));
	json_object_set_new(root, SMOOTH_KEY, json_real(_smooth));
	switch (_quality) {
		case AnalyzerCore::QUALITY_GOOD: {
			json_object_set_new(root, QUALITY_KEY, json_string(QUALITY_GOOD_KEY));
			break;
		}
		case AnalyzerCore::QUALITY_HIGH: {
			json_object_set_new(root, QUALITY_KEY, json_string(QUALITY_HIGH_KEY));
			break;
		}
		case AnalyzerCore::QUALITY_ULTRA: {
			json_object_set_new(root, QUALITY_KEY, json_string(QUALITY_ULTRA_KEY));
			break;
		}
	}
	switch (_window) {
		case AnalyzerCore::WINDOW_NONE: {
			json_object_set_new(root, WINDOW_KEY, json_string(WINDOW_NONE_KEY));
			break;
		}
		case AnalyzerCore::WINDOW_HAMMING: {
			json_object_set_new(root, WINDOW_KEY, json_string(WINDOW_HAMMING_KEY));
			break;
		}
		case AnalyzerCore::WINDOW_KAISER: {
			json_object_set_new(root, WINDOW_KEY, json_string(WINDOW_KAISER_KEY));
			break;
		}
	}
	return root;
}

void AnalyzerXL::fromJson(json_t* root) {
	json_t* jr = json_object_get(root, RANGE_KEY);
	if (jr) {
		_range = clamp(json_real_value(jr), -0.9f, 0.8f);
	}

	json_t* jrd = json_object_get(root, RANGE_DB_KEY);
	if (jrd) {
		_rangeDb = clamp(json_real_value(jrd), 80.0f, 140.0f);
	}

	json_t* js = json_object_get(root, SMOOTH_KEY);
	if (js) {
		_smooth = clamp(json_real_value(js), 0.0f, 0.5f);
	}

	json_t* jq = json_object_get(root, QUALITY_KEY);
	if (jq) {
		const char *s = json_string_value(jq);
		if (strcmp(s, QUALITY_GOOD_KEY) == 0) {
			_quality = AnalyzerCore::QUALITY_GOOD;
		}
		else if (strcmp(s, QUALITY_HIGH_KEY) == 0) {
			_quality = AnalyzerCore::QUALITY_HIGH;
		}
		else if (strcmp(s, QUALITY_ULTRA_KEY) == 0) {
			_quality = AnalyzerCore::QUALITY_ULTRA;
		}
	}

	json_t* jw = json_object_get(root, WINDOW_KEY);
	if (jw) {
		const char *s = json_string_value(jw);
		if (strcmp(s, WINDOW_NONE_KEY) == 0) {
			_window = AnalyzerCore::WINDOW_NONE;
		}
		else if (strcmp(s, WINDOW_HAMMING_KEY) == 0) {
			_window = AnalyzerCore::WINDOW_HAMMING;
		}
		else if (strcmp(s, WINDOW_KAISER_KEY) == 0) {
			_window = AnalyzerCore::WINDOW_KAISER;
		}
	}
}

void AnalyzerXL::step() {
	++_modulationStep;
	if (_modulationStep >= modulationSteps) {
		_modulationStep = 0;
		setCoreParams();
	}

	for (int i = 0; i < 8; ++i) {
		_core.stepChannel(i, inputs[SIGNALA_INPUT + i]);
	}
}

struct RangeMenuItem : MenuItem {
	AnalyzerXL* _module;
	const float _range;

	RangeMenuItem(AnalyzerXL* module, const char* label, float range)
	: _module(module)
	, _range(range)
	{
		this->text = label;
	}

	void onAction(EventAction &e) override {
		_module->_range = _range;
	}

	void step() override {
		rightText = _module->_range == _range ? "✔" : "";
	}
};

struct RangeDbMenuItem : MenuItem {
	AnalyzerXL* _module;
	const float _rangeDb;

	RangeDbMenuItem(AnalyzerXL* module, const char* label, float rangeDb)
	: _module(module)
	, _rangeDb(rangeDb)
	{
		this->text = label;
	}

	void onAction(EventAction &e) override {
		_module->_rangeDb = _rangeDb;
	}

	void step() override {
		rightText = _module->_rangeDb == _rangeDb ? "✔" : "";
	}
};

struct SmoothMenuItem : MenuItem {
	AnalyzerXL* _module;
	const float _smooth;

	SmoothMenuItem(AnalyzerXL* module, const char* label, float smooth)
	: _module(module)
	, _smooth(smooth)
	{
		this->text = label;
	}

	void onAction(EventAction &e) override {
		_module->_smooth = _smooth;
	}

	void step() override {
		rightText = _module->_smooth == _smooth ? "✔" : "";
	}
};

struct QualityMenuItem : MenuItem {
	AnalyzerXL* _module;
	const AnalyzerCore::Quality _quality;

	QualityMenuItem(AnalyzerXL* module, const char* label, AnalyzerCore::Quality quality)
	: _module(module)
	, _quality(quality)
	{
		this->text = label;
	}

	void onAction(EventAction &e) override {
		_module->_quality = _quality;
	}

	void step() override {
		rightText = _module->_quality == _quality ? "✔" : "";
	}
};

struct WindowMenuItem : MenuItem {
	AnalyzerXL* _module;
	const AnalyzerCore::Window _window;

	WindowMenuItem(AnalyzerXL* module, const char* label, AnalyzerCore::Window window)
	: _module(module)
	, _window(window)
	{
		this->text = label;
	}

	void onAction(EventAction &e) override {
		_module->_window = _window;
	}

	void step() override {
		rightText = _module->_window == _window ? "✔" : "";
	}
};

struct AnalyzerXLWidget : ModuleWidget {
	static constexpr int hp = 42;

	AnalyzerXLWidget(AnalyzerXL* module) : ModuleWidget(module) {
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);

		{
			SVGPanel *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(SVG::load(assetPlugin(plugin, "res/AnalyzerXL.svg")));
			addChild(panel);
		}

		{
			auto inset = Vec(30, 1);
			auto size = Vec(box.size.x - inset.x - 1, 378);
			auto display = new AnalyzerDisplay(module, size, false);
			display->box.pos = inset;
			display->box.size = size;
			addChild(display);
		}

		// generated by svg_widgets.rb
		auto signalaInputPosition = Vec(3.0, 13.0);
		auto signalbInputPosition = Vec(3.0, 47.0);
		auto signalcInputPosition = Vec(3.0, 81.0);
		auto signaldInputPosition = Vec(3.0, 115.0);
		auto signaleInputPosition = Vec(3.0, 149.0);
		auto signalfInputPosition = Vec(3.0, 183.0);
		auto signalgInputPosition = Vec(3.0, 217.0);
		auto signalhInputPosition = Vec(3.0, 251.0);
		// end generated by svg_widgets.rb

		addInput(Port::create<Port24>(signalaInputPosition, Port::INPUT, module, AnalyzerXL::SIGNALA_INPUT));
		addInput(Port::create<Port24>(signalbInputPosition, Port::INPUT, module, AnalyzerXL::SIGNALB_INPUT));
		addInput(Port::create<Port24>(signalcInputPosition, Port::INPUT, module, AnalyzerXL::SIGNALC_INPUT));
		addInput(Port::create<Port24>(signaldInputPosition, Port::INPUT, module, AnalyzerXL::SIGNALD_INPUT));
		addInput(Port::create<Port24>(signaleInputPosition, Port::INPUT, module, AnalyzerXL::SIGNALE_INPUT));
		addInput(Port::create<Port24>(signalfInputPosition, Port::INPUT, module, AnalyzerXL::SIGNALF_INPUT));
		addInput(Port::create<Port24>(signalgInputPosition, Port::INPUT, module, AnalyzerXL::SIGNALG_INPUT));
		addInput(Port::create<Port24>(signalhInputPosition, Port::INPUT, module, AnalyzerXL::SIGNALH_INPUT));
	}

	void appendContextMenu(Menu* menu) override {
		AnalyzerXL* a = dynamic_cast<AnalyzerXL*>(module);
		assert(a);

		menu->addChild(new MenuLabel());
		menu->addChild(new RangeMenuItem(a, "Frequency range: lower 25%", -0.75f));
		menu->addChild(new RangeMenuItem(a, "Frequency range: lower 50%", -0.5f));
		menu->addChild(new RangeMenuItem(a, "Frequency range: full", 0.0f));
		menu->addChild(new RangeMenuItem(a, "Frequency range: upper 50%", 0.5f));
		menu->addChild(new RangeMenuItem(a, "Frequency range: upper 25%", 0.75f));

		menu->addChild(new MenuLabel());
		menu->addChild(new RangeDbMenuItem(a, "Amplitude range: to -60dB", 80.0f));
		menu->addChild(new RangeDbMenuItem(a, "Amplitude range: to -120dB", 140.0f));

		menu->addChild(new MenuLabel());
		menu->addChild(new SmoothMenuItem(a, "Smooth: none", 0.0f));
		menu->addChild(new SmoothMenuItem(a, "Smooth: 10ms", 0.01f));
		menu->addChild(new SmoothMenuItem(a, "Smooth: 50ms", 0.05f));
		menu->addChild(new SmoothMenuItem(a, "Smooth: 100ms", 0.1f));
		menu->addChild(new SmoothMenuItem(a, "Smooth: 250ms", 0.25f));
		menu->addChild(new SmoothMenuItem(a, "Smooth: 500ms", 0.5f));

		menu->addChild(new MenuLabel());
		menu->addChild(new QualityMenuItem(a, "Quality: good", AnalyzerCore::QUALITY_GOOD));
		menu->addChild(new QualityMenuItem(a, "Quality: high", AnalyzerCore::QUALITY_HIGH));
		menu->addChild(new QualityMenuItem(a, "Quality: ultra", AnalyzerCore::QUALITY_ULTRA));

		menu->addChild(new MenuLabel());
		menu->addChild(new WindowMenuItem(a, "Window: Kaiser", AnalyzerCore::WINDOW_KAISER));
		menu->addChild(new WindowMenuItem(a, "Window: Hamming", AnalyzerCore::WINDOW_HAMMING));
		menu->addChild(new WindowMenuItem(a, "Window: none", AnalyzerCore::WINDOW_NONE));
	}
};

RACK_PLUGIN_MODEL_INIT(Bogaudio, AnalyzerXL) {
   Model *modelAnalyzerXL = createModel<AnalyzerXL, AnalyzerXLWidget>("Bogaudio-AnalyzerXL", "Analyzer-XL", "spectrum analyzer", VISUAL_TAG);
   return modelAnalyzerXL;
}

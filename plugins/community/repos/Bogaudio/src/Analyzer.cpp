#include "global_pre.hpp"
#include "Analyzer.hpp"
#include "dsp/signal.hpp"
#include "global_ui.hpp"

struct bogaudio::ChannelAnalyzer : SpectrumAnalyzer {
	int _binsN;
	float* _bins;
	AveragingBuffer<float>* _averagedBins;

	ChannelAnalyzer(
		SpectrumAnalyzer::Size size,
		SpectrumAnalyzer::Overlap overlap,
		SpectrumAnalyzer::WindowType windowType,
		float sampleRate,
		int averageN,
		int binSize
	)
	: SpectrumAnalyzer(size, overlap, windowType, sampleRate)
	, _binsN(size / binSize)
	, _bins(averageN == 1 ? new float[_binsN] {} : NULL)
	, _averagedBins(averageN == 1 ? NULL : new AveragingBuffer<float>(_binsN, averageN))
	{
		assert(averageN >= 1);
		assert(binSize >= 1);
	}
	virtual ~ChannelAnalyzer() {
		if (_bins) {
			delete[] _bins;
		}
		if (_averagedBins) {
			delete _averagedBins;
		}
	}

	const float* getBins() {
		if (_bins) {
			return _bins;
		}
		return _averagedBins->getAverages();
	}

	bool step(float sample) override;
	float getPeak();
};

bool ChannelAnalyzer::step(float sample) {
	if (SpectrumAnalyzer::step(sample)) {
		if (_bins) {
			getMagnitudes(_bins, _binsN);
		}
		else {
			float* frame = _averagedBins->getInputFrame();
			getMagnitudes(frame, _binsN);
			_averagedBins->commitInputFrame();
		}
		return true;
	}
	return false;
}

float ChannelAnalyzer::getPeak() {
	float max = 0.0;
	float sum = 0.0;
	int maxBin = 0;
	const float* bins = getBins();
	for (int bin = 0; bin < _binsN; ++bin) {
		if (bins[bin] > max) {
			max = bins[bin];
			maxBin = bin;
		}
		sum += bins[bin];
	}
	const int bandsPerBin = _size / _binsN;
	const float fWidth = (_sampleRate / 2.0f) / (float)(_size / bandsPerBin);
	return (maxBin + 0.5f)*fWidth;
}


void Analyzer::onReset() {
	resetChannels();
}

void Analyzer::onSampleRateChange() {
	resetChannels();
}

void Analyzer::resetChannels() {
	if (_channelA) {
		delete _channelA;
		_channelA = NULL;
	}
	if (_channelB) {
		delete _channelB;
		_channelB = NULL;
	}
	if (_channelC) {
		delete _channelC;
		_channelC = NULL;
	}
	if (_channelD) {
		delete _channelD;
		_channelD = NULL;
	}
}

SpectrumAnalyzer::Size Analyzer::size() {
	if (_quality == QUALITY_HIGH) {
		return SpectrumAnalyzer::SIZE_4096;
	}
	return SpectrumAnalyzer::SIZE_1024;
}

void Analyzer::step() {
	_range = params[RANGE_PARAM].value;

	const float maxTime = 0.5;
	float smooth = params[SMOOTH_PARAM].value * maxTime;
	smooth /= size() / (_overlap * engineGetSampleRate());
	int smoothN = std::max(1, (int)roundf(smooth));
	if (_averageN != smoothN) {
		_averageN = smoothN;
		resetChannels();
	}

	Quality quality = params[QUALITY_PARAM].value > 1.5 ? QUALITY_HIGH : QUALITY_GOOD;
	if (_quality != quality) {
		_quality = quality;
		resetChannels();
	}

	_running = params[POWER_PARAM].value == 1.0;
	stepChannel(_channelA, _running, inputs[SIGNALA_INPUT], outputs[SIGNALA_OUTPUT]);
	stepChannel(_channelB, _running, inputs[SIGNALB_INPUT], outputs[SIGNALB_OUTPUT]);
	stepChannel(_channelC, _running, inputs[SIGNALC_INPUT], outputs[SIGNALC_OUTPUT]);
	stepChannel(_channelD, _running, inputs[SIGNALD_INPUT], outputs[SIGNALD_OUTPUT]);

	lights[QUALITY_HIGH_LIGHT].value = _running && quality == QUALITY_HIGH;
	lights[QUALITY_GOOD_LIGHT].value = _running && quality == QUALITY_GOOD;
	lights[POWER_ON_LIGHT].value = _running;
}

void Analyzer::stepChannel(ChannelAnalyzer*& channelPointer, bool running, Input& input, Output& output) {
	if (running && input.active) {
		if (!channelPointer) {
			channelPointer = new ChannelAnalyzer(
				size(),
				_overlap,
				SpectrumAnalyzer::WINDOW_HAMMING,
				engineGetSampleRate(),
				_averageN,
				_binAverageN
			);
		}

		channelPointer->step(input.value);
		output.value = input.value;
	}
	else {
		if (channelPointer) {
			delete channelPointer;
			channelPointer = NULL;
		}

		output.value = input.value;
	}
}


struct AnalyzerDisplay : TransparentWidget {
	const int _insetAround = 2;
	const int _insetLeft = _insetAround + 12;
	const int _insetRight = _insetAround + 2;
	const int _insetTop = _insetAround + 13;
	const int _insetBottom = _insetAround + 9;

	const float _displayDB = 80.0;
	const float _positiveDisplayDB = 20.0;

	const float xAxisLogFactor = 1 / 3.321; // magic number.

	const NVGcolor _axisColor = nvgRGBA(0xff, 0xff, 0xff, 0x70);
	const NVGcolor _textColor = nvgRGBA(0xff, 0xff, 0xff, 0xc0);
	const NVGcolor _channelAColor = nvgRGBA(0x00, 0xff, 0x00, 0xd0);
	const NVGcolor _channelBColor = nvgRGBA(0xff, 0x00, 0xff, 0xd0);
	const NVGcolor _channelCColor = nvgRGBA(0xff, 0x80, 0x00, 0xd0);
	const NVGcolor _channelDColor = nvgRGBA(0x00, 0x80, 0xff, 0xd0);

	Analyzer* _module;
	const Vec _size;
	const Vec _graphSize;
	std::shared_ptr<Font> _font;

	AnalyzerDisplay(
		Analyzer* module,
		Vec size
	)
	: _module(module)
	, _size(size)
	, _graphSize(_size.x - _insetLeft - _insetRight, _size.y - _insetTop - _insetBottom)
	, _font(Font::load(assetPlugin(plugin, "res/fonts/inconsolata.ttf")))
	{
	}

	void draw(NVGcontext* vg) override;
	void drawBackground(NVGcontext* vg);
	void drawHeader(NVGcontext* vg);
	void drawYAxis(NVGcontext* vg, float strokeWidth);
	void drawXAxis(NVGcontext* vg, float strokeWidth);
	void drawXAxisLine(NVGcontext* vg, float hz, float maxHz);
	void drawGraph(NVGcontext* vg, const float* bins, int binsN, NVGcolor color, float strokeWidth);
	void drawText(NVGcontext* vg, const char* s, float x, float y, float rotation = 0.0, const NVGcolor* color = NULL);
	int binValueToHeight(float value);
};

void AnalyzerDisplay::draw(NVGcontext* vg) {
	drawBackground(vg);
	if (_module->_running) {
		float strokeWidth = std::max(1.0f, 3 - RACK_PLUGIN_UI_RACKSCENE->zoomWidget->zoom);
		nvgSave(vg);
		nvgScissor(vg, _insetAround, _insetAround, _size.x - _insetAround, _size.y - _insetAround);
		drawHeader(vg);
		drawYAxis(vg, strokeWidth);
		drawXAxis(vg, strokeWidth);

		if (_module->_channelA) {
			drawGraph(vg, _module->_channelA->getBins(), _module->_channelA->_binsN, _channelAColor, strokeWidth);
		}
		if (_module->_channelB) {
			drawGraph(vg, _module->_channelB->getBins(), _module->_channelB->_binsN, _channelBColor, strokeWidth);
		}
		if (_module->_channelC) {
			drawGraph(vg, _module->_channelC->getBins(), _module->_channelC->_binsN, _channelCColor, strokeWidth);
		}
		if (_module->_channelD) {
			drawGraph(vg, _module->_channelD->getBins(), _module->_channelD->_binsN, _channelDColor, strokeWidth);
		}
		nvgRestore(vg);
	}
}

void AnalyzerDisplay::drawBackground(NVGcontext* vg) {
	nvgSave(vg);
	nvgBeginPath(vg);
	nvgRect(vg, 0, 0, _size.x, _size.y);
	nvgFillColor(vg, nvgRGBA(0x00, 0x00, 0x00, 0xff));
	nvgFill(vg);
	nvgStrokeColor(vg, nvgRGBA(0xc0, 0xc0, 0xc0, 0xff));
	nvgStroke(vg);
	nvgRestore(vg);
}

void AnalyzerDisplay::drawHeader(NVGcontext* vg) {
	nvgSave(vg);

	const int textY = -4;
	const int charPx = 5;
	const int sLen = 100;
	char s[sLen];
	int x = _insetAround + 2;

	int n = snprintf(s, sLen, "Peaks (+/-%0.1f):", (engineGetSampleRate() / 2.0f) / (float)(_module->size() / _module->_binAverageN));
	drawText(vg, s, x, _insetTop + textY);
	x += n * charPx - 0;

	if (_module->_channelA) {
		snprintf(s, sLen, "A:%7.1f", _module->_channelA->getPeak());
		drawText(vg, s, x, _insetTop + textY, 0.0, &_channelAColor);
	}
	x += 9 * charPx + 3;

	if (_module->_channelB) {
		snprintf(s, sLen, "B:%7.1f", _module->_channelB->getPeak());
		drawText(vg, s, x, _insetTop + textY, 0.0, &_channelBColor);
	}
	x += 9 * charPx + 3;

	if (_module->_channelC) {
		snprintf(s, sLen, "C:%7.1f", _module->_channelC->getPeak());
		drawText(vg, s, x, _insetTop + textY, 0.0, &_channelCColor);
	}
	x += 9 * charPx + 3;

	if (_module->_channelD) {
		snprintf(s, sLen, "D:%7.1f", _module->_channelD->getPeak());
		drawText(vg, s, x, _insetTop + textY, 0.0, &_channelDColor);
	}

	nvgRestore(vg);
}

void AnalyzerDisplay::drawYAxis(NVGcontext* vg, float strokeWidth) {
	nvgSave(vg);
	nvgStrokeColor(vg, _axisColor);
	nvgStrokeWidth(vg, strokeWidth);
	const int lineX = _insetLeft - 2;
	const int textX = 9;
	const float textR = -M_PI/2.0;

	nvgBeginPath(vg);
	int lineY = _insetTop;
	nvgMoveTo(vg, lineX, lineY);
	nvgLineTo(vg, _size.x - _insetRight, lineY);
	nvgStroke(vg);

	nvgBeginPath(vg);
	lineY = _insetTop + (_graphSize.y - _graphSize.y*(_displayDB - _positiveDisplayDB + 12.0)/_displayDB);
	nvgMoveTo(vg, lineX, lineY);
	nvgLineTo(vg, _size.x - _insetRight, lineY);
	nvgStroke(vg);
	drawText(vg, "12", textX, lineY + 5.0, textR);

	nvgBeginPath(vg);
	lineY = _insetTop + (_graphSize.y - _graphSize.y*(_displayDB - _positiveDisplayDB)/_displayDB);
	nvgMoveTo(vg, lineX, lineY);
	nvgLineTo(vg, _size.x - _insetRight, lineY);
	nvgStrokeWidth(vg, strokeWidth * 1.5);
	nvgStroke(vg);
	nvgStrokeWidth(vg, strokeWidth);
	drawText(vg, "0", textX, lineY + 2.3, textR);

	nvgBeginPath(vg);
	lineY = _insetTop + (_graphSize.y - _graphSize.y*(_displayDB - _positiveDisplayDB - 12.0)/_displayDB);
	nvgMoveTo(vg, lineX, lineY);
	nvgLineTo(vg, _size.x - _insetRight, lineY);
	nvgStroke(vg);
	drawText(vg, "-12", textX, lineY + 10, textR);

	nvgBeginPath(vg);
	lineY = _insetTop + (_graphSize.y - _graphSize.y*(_displayDB - _positiveDisplayDB - 24.0)/_displayDB);
	nvgMoveTo(vg, lineX, lineY);
	nvgLineTo(vg, _size.x - _insetRight, lineY);
	nvgStroke(vg);
	drawText(vg, "-24", textX, lineY + 10, textR);

	nvgBeginPath(vg);
	lineY = _insetTop + (_graphSize.y - _graphSize.y*(_displayDB - _positiveDisplayDB - 48.0)/_displayDB);
	nvgMoveTo(vg, lineX, lineY);
	nvgLineTo(vg, _size.x - _insetRight, lineY);
	nvgStroke(vg);
	drawText(vg, "-48", textX, lineY + 10, textR);

	nvgBeginPath(vg);
	lineY = _insetTop + _graphSize.y + 1;
	nvgMoveTo(vg, lineX, lineY);
	nvgLineTo(vg, _size.x - _insetRight, lineY);
	nvgStroke(vg);

	nvgBeginPath(vg);
	nvgMoveTo(vg, lineX, _insetTop);
	nvgLineTo(vg, lineX, lineY);
	nvgStroke(vg);

	drawText(vg, "dB", textX, _size.y - _insetBottom, textR);

	nvgRestore(vg);
}

void AnalyzerDisplay::drawXAxis(NVGcontext* vg, float strokeWidth) {
	nvgSave(vg);
	nvgStrokeColor(vg, _axisColor);
	nvgStrokeWidth(vg, strokeWidth);

	const float maxHz = _module->_range * (engineGetSampleRate() / 2.0);
	float hz = 100.0;
	while (hz < maxHz && hz < 1001.0) {
		drawXAxisLine(vg, hz, maxHz);
		hz += 100.0;
	}
	hz = 2000.0;
	while (hz < maxHz && hz < 10001.0) {
		drawXAxisLine(vg, hz, maxHz);
		hz += 1000.0;
	}
	hz = 20000.0;
	while (hz < maxHz && hz < 100001.0) {
		drawXAxisLine(vg, hz, maxHz);
		hz += 10000.0;
	}

	drawText(vg, "Hz", _insetLeft, _size.y - 2);
	{
		float x = 100.0 / maxHz;
		x = powf(x, xAxisLogFactor);
		if (x < 1.0) {
			x *= _graphSize.x;
			drawText(vg, "100", _insetLeft + x - 8, _size.y - 2);
		}
	}
	{
		float x = 1000.0 / maxHz;
		x = powf(x, xAxisLogFactor);
		if (x < 1.0) {
			x *= _graphSize.x;
			drawText(vg, "1k", _insetLeft + x - 4, _size.y - 2);
		}
	}
	{
		float x = 10000.0 / maxHz;
		x = powf(x, xAxisLogFactor);
		if (x < 1.0) {
			x *= _graphSize.x;
			drawText(vg, "10k", _insetLeft + x - 7, _size.y - 2);
		}
	}

	nvgRestore(vg);
}

void AnalyzerDisplay::drawXAxisLine(NVGcontext* vg, float hz, float maxHz) {
	float x = hz / maxHz;
	x = powf(x, xAxisLogFactor);
	if (x < 1.0) {
		x *= _graphSize.x;
		nvgBeginPath(vg);
		nvgMoveTo(vg, _insetLeft + x, _insetTop);
		nvgLineTo(vg, _insetLeft + x, _insetTop + _graphSize.y);
		nvgStroke(vg);
	}
}

void AnalyzerDisplay::drawGraph(NVGcontext* vg, const float* bins, int binsN, NVGcolor color, float strokeWidth) {
	const int pointsN = roundf(_module->_range*(_module->size()/2));
	nvgSave(vg);
	nvgScissor(vg, _insetLeft, _insetTop, _graphSize.x, _graphSize.y);
	nvgStrokeColor(vg, color);
	nvgStrokeWidth(vg, strokeWidth);
	nvgBeginPath(vg);
	for (int i = 0; i < pointsN; ++i) {
		int height = binValueToHeight(bins[i]);
		if (i == 0) {
			nvgMoveTo(vg, _insetLeft, _insetTop + (_graphSize.y - height));
		}
		else {
			float x = _graphSize.x * powf(i / (float)pointsN, xAxisLogFactor);
			nvgLineTo(vg, _insetLeft + x, _insetTop + (_graphSize.y - height));
		}
	}
	nvgStroke(vg);
	nvgRestore(vg);
}

void AnalyzerDisplay::drawText(NVGcontext* vg, const char* s, float x, float y, float rotation, const NVGcolor* color) {
	nvgSave(vg);
	nvgTranslate(vg, x, y);
	nvgRotate(vg, rotation);
	nvgFontSize(vg, 10);
	nvgFontFaceId(vg, _font->handle);
	nvgFillColor(vg, color ? *color : _textColor);
	nvgText(vg, 0, 0, s, NULL);
	nvgRestore(vg);
}

int AnalyzerDisplay::binValueToHeight(float value) {
	const float minDB = -(_displayDB - _positiveDisplayDB);
	if (value < 0.00001f) {
		return 0;
	}
	value /= 10.0f; // arbitrarily use 5.0f as reference "maximum" baseline signal (e.g. raw output of an oscillator)...but signals are +/-5, so 10 total.
	value = powf(value, 0.5f); // undoing magnitude scaling of levels back from the FFT?
	value = amplitudeToDecibels(value);
	value = std::max(minDB, value);
	value = std::min(_positiveDisplayDB, value);
	value -= minDB;
	value /= _displayDB;
	return roundf(_graphSize.y * value);
}


struct OneTenKnob : Knob38 {
	OneTenKnob() : Knob38() {
		minAngle = -0.664*M_PI;
	}
};


struct AnalyzerWidget : ModuleWidget {
	static constexpr int hp = 20;

	AnalyzerWidget(Analyzer* module) : ModuleWidget(module) {
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);

		{
			auto panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(SVG::load(assetPlugin(plugin, "res/Analyzer.svg")));
			addChild(panel);
		}

		{
			auto inset = Vec(10, 25);
			auto size = Vec(box.size.x - 2*inset.x, 230);
			auto display = new AnalyzerDisplay(module, size);
			display->box.pos = inset;
			display->box.size = size;
			addChild(display);
		}

		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

		// generated by svg_widgets.rb
		auto rangeParamPosition = Vec(35.08, 271.08);
		auto smoothParamPosition = Vec(109.08, 271.08);
		auto qualityParamPosition = Vec(188.02, 300.02);
		auto powerParamPosition = Vec(261.02, 300.02);

		auto signalaInputPosition = Vec(13.5, 323.0);
		auto signalbInputPosition = Vec(86.5, 323.0);
		auto signalcInputPosition = Vec(160.5, 323.0);
		auto signaldInputPosition = Vec(233.5, 323.0);

		auto signalaOutputPosition = Vec(42.5, 323.0);
		auto signalbOutputPosition = Vec(115.5, 323.0);
		auto signalcOutputPosition = Vec(189.5, 323.0);
		auto signaldOutputPosition = Vec(262.5, 323.0);

		auto qualityHighLightPosition = Vec(179.0, 273.0);
		auto qualityGoodLightPosition = Vec(179.0, 288.0);
		auto powerOnLightPosition = Vec(252.0, 288.0);
		// end generated by svg_widgets.rb

		addParam(ParamWidget::create<OneTenKnob>(rangeParamPosition, module, Analyzer::RANGE_PARAM, 0.1, 1.0, 0.5));
		addParam(ParamWidget::create<Knob38>(smoothParamPosition, module, Analyzer::SMOOTH_PARAM, 0.0, 1.0, 0.5));
		addParam(ParamWidget::create<StatefulButton9>(qualityParamPosition, module, Analyzer::QUALITY_PARAM, 1.0, 2.0, 1.0));
		{
			auto w = ParamWidget::create<StatefulButton9>(powerParamPosition, module, Analyzer::POWER_PARAM, 0.0, 1.0, 1.0);
			w->randomizable = false;
			addParam(w);
		}

		addInput(Port::create<Port24>(signalaInputPosition, Port::INPUT, module, Analyzer::SIGNALA_INPUT));
		addInput(Port::create<Port24>(signalbInputPosition, Port::INPUT, module, Analyzer::SIGNALB_INPUT));
		addInput(Port::create<Port24>(signalcInputPosition, Port::INPUT, module, Analyzer::SIGNALC_INPUT));
		addInput(Port::create<Port24>(signaldInputPosition, Port::INPUT, module, Analyzer::SIGNALD_INPUT));

		addOutput(Port::create<Port24>(signalaOutputPosition, Port::OUTPUT, module, Analyzer::SIGNALA_OUTPUT));
		addOutput(Port::create<Port24>(signalbOutputPosition, Port::OUTPUT, module, Analyzer::SIGNALB_OUTPUT));
		addOutput(Port::create<Port24>(signalcOutputPosition, Port::OUTPUT, module, Analyzer::SIGNALC_OUTPUT));
		addOutput(Port::create<Port24>(signaldOutputPosition, Port::OUTPUT, module, Analyzer::SIGNALD_OUTPUT));

		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(qualityHighLightPosition, module, Analyzer::QUALITY_HIGH_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(qualityGoodLightPosition, module, Analyzer::QUALITY_GOOD_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(powerOnLightPosition, module, Analyzer::POWER_ON_LIGHT));
	}
};

RACK_PLUGIN_MODEL_INIT(Bogaudio, Analyzer) {
   Model *modelAnalyzer = createModel<Analyzer, AnalyzerWidget>("Bogaudio-Analyzer", "Analyzer",  "spectrum analyzer", VISUAL_TAG);
   return modelAnalyzer;
}

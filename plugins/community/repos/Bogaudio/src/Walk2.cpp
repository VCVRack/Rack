
#include "Walk2.hpp"

void Walk2::onReset() {
	_triggerX.reset();
	_triggerY.reset();
	_jumpTrigger.reset();
	_modulationStep = modulationSteps;
}

void Walk2::onSampleRateChange() {
	_modulationStep = modulationSteps;
	_historySteps = (historySeconds * engineGetSampleRate()) / historyPoints;
}

void Walk2::step() {
	lights[TRACK_X_LIGHT].value = params[TRACK_X_PARAM].value;
	lights[TRACK_Y_LIGHT].value = params[TRACK_Y_PARAM].value;

	++_modulationStep;
	if (_modulationStep >= modulationSteps) {
		_modulationStep = 0;

		float rateX = params[RATE_X_PARAM].value;
		if (inputs[RATE_X_INPUT].active) {
			rateX *= clamp(inputs[RATE_X_INPUT].value / 10.0f, 0.0f, 1.0f);
		}
		rateX *= rateX;
		rateX *= rateX;
		_walkX.setParams(engineGetSampleRate(), rateX);

		float rateY = params[RATE_Y_PARAM].value;
		if (inputs[RATE_Y_INPUT].active) {
			rateY *= clamp(inputs[RATE_Y_INPUT].value / 10.0f, 0.0f, 1.0f);
		}
		rateY *= rateY;
		rateY *= rateY;
		_walkY.setParams(engineGetSampleRate(), rateY);
	}

	if (_jumpTrigger.process(inputs[JUMP_INPUT].value)) {
		_walkX.jump();
		_walkY.jump();
	}

	float outX = _walkX.next();
	outX *= params[SCALE_X_PARAM].value;
	outX += params[OFFSET_X_PARAM].value * 5.0f;
	outputs[OUT_X_OUTPUT].value = outX;

	bool triggeredX = _triggerX.process(inputs[HOLD_X_INPUT].value);
	if (params[TRACK_X_PARAM].value > 0.5f ? _triggerX.isHigh() : triggeredX) {
		_holdX = outX;
	}
	outputs[HOLD_X_OUTPUT].value = _holdX;

	float outY = _walkY.next();
	outY *= params[SCALE_Y_PARAM].value;
	outY += params[OFFSET_Y_PARAM].value * 5.0f;
	outputs[OUT_Y_OUTPUT].value = outY;

	bool triggeredY = _triggerY.process(inputs[HOLD_Y_INPUT].value);
	if (params[TRACK_Y_PARAM].value > 0.5f ? _triggerY.isHigh() : triggeredY) {
		_holdY = outY;
	}
	outputs[HOLD_Y_OUTPUT].value = _holdY;

	outputs[DISTANCE_OUTPUT].value = sqrtf(outX*outX + outY*outY) * 0.707107f; // scaling constant is 10 / squrt(200)

	if (_historyStep == 0) {
		_outsX.push(outX);
		_holdsX.push(_holdX);
		_outsY.push(outY);
		_holdsY.push(_holdY);
	}
	++_historyStep;
	_historyStep %= _historySteps;
}

struct Walk2Display : TransparentWidget {
	const int _insetAround = 4;

	const NVGcolor _axisColor = nvgRGBA(0xff, 0xff, 0xff, 0x70);
	const NVGcolor _textColor = nvgRGBA(0xff, 0xff, 0xff, 0xc0);
	const NVGcolor _traceColor = nvgRGBA(0xff, 0x00, 0x00, 0xd0);
	const NVGcolor _holdColor = nvgRGBA(0x00, 0xff, 0x00, 0xd0);

	Walk2* _module;
	const Vec _size;
	const Vec _drawSize;
	int _midX, _midY;
	std::shared_ptr<Font> _font;

	Walk2Display(
		Walk2* module,
		Vec size
	)
	: _module(module)
	, _size(size)
	, _drawSize(_size.x - 2*_insetAround, _size.y - 2*_insetAround)
	, _midX(_size.x / 2)
	, _midY(_size.y / 2)
	, _font(Font::load(assetPlugin(plugin, "res/fonts/inconsolata.ttf")))
	{
	}

	void draw(NVGcontext* vg) override {
		drawBackground(vg);
		float strokeWidth = std::max(1.0f, 3 - RACK_PLUGIN_UI_RACKSCENE->zoomWidget->zoom);

		nvgSave(vg);
		nvgScissor(vg, _insetAround, _insetAround, _size.x - _insetAround, _size.y - _insetAround);
		drawAxes(vg, strokeWidth);
		drawTrace(vg, _traceColor, _module->_outsX, _module->_outsY);
		drawTrace(vg, _holdColor, _module->_holdsX, _module->_holdsY);
		nvgRestore(vg);
	}

	void drawBackground(NVGcontext* vg) {
		nvgSave(vg);
		nvgBeginPath(vg);
		nvgRect(vg, 0, 0, _size.x, _size.y);
		nvgFillColor(vg, nvgRGBA(0x00, 0x00, 0x00, 0xff));
		nvgFill(vg);
		nvgStrokeColor(vg, nvgRGBA(0xc0, 0xc0, 0xc0, 0xff));
		nvgStroke(vg);
		nvgRestore(vg);
	}

	void drawAxes(NVGcontext* vg, float strokeWidth) {
		nvgSave(vg);
		nvgStrokeColor(vg, _axisColor);
		nvgStrokeWidth(vg, strokeWidth);

		nvgBeginPath(vg);
		nvgMoveTo(vg, _insetAround, _midY);
		nvgLineTo(vg, _size.x - _insetAround, _midY);
		nvgStroke(vg);

		nvgBeginPath(vg);
		nvgMoveTo(vg, _midX, _insetAround);
		nvgLineTo(vg, _midX, _size.y - _insetAround);
		nvgStroke(vg);

		nvgRestore(vg);
	}

	void drawTrace(NVGcontext* vg, NVGcolor color, HistoryBuffer<float>& x, HistoryBuffer<float>& y) {
		nvgSave(vg);
		nvgGlobalCompositeOperation(vg, NVG_LIGHTER);

		// int n = _module->historyPoints;
		// float beginRadius = std::max(1.0f, 2.0f - gRackScene->zoomWidget->zoom);
		// float endRadius = std::max(0.01f, 0.8f - gRackScene->zoomWidget->zoom);
		// float radiusStep = (beginRadius - endRadius) / (float)n;
		// float radius = beginRadius;
		// float alphaStep = (color.a - 0.1f) / (float)n;
		// for (int i = 0; i < n; ++i) {
		// 	nvgBeginPath(vg);
		// 	nvgCircle(vg, cvToPixel(_midX, _drawSize.x, x.value(i)), cvToPixel(_midY, _drawSize.y, y.value(i)), radius);
		// 	nvgStrokeColor(vg, color);
		// 	nvgFillColor(vg, color);
		// 	nvgStroke(vg);
		// 	nvgFill(vg);
		// 	radius -= radiusStep;
		// 	color.a -= alphaStep;
		// }

		int n = _module->historyPoints;
		float beginWidth = std::max(1.0f, 4.0f - RACK_PLUGIN_UI_RACKSCENE->zoomWidget->zoom);
		float endWidth = std::max(0.5f, 2.0f - RACK_PLUGIN_UI_RACKSCENE->zoomWidget->zoom);
		float widthStep = (beginWidth - endWidth) / (float)n;
		float width = endWidth;
		float endAlpha = 0.1f;
		float alphaStep = (color.a - endAlpha) / (float)n;
		color.a = endAlpha;
		for (int i = n - 1; i > 0; --i) {
			nvgBeginPath(vg);
			nvgMoveTo(vg, cvToPixel(_midX, _drawSize.x, x.value(i - 1)), cvToPixel(_midY, _drawSize.y, y.value(i - 1)));
			nvgLineTo(vg, cvToPixel(_midX, _drawSize.x, x.value(i)), cvToPixel(_midY, _drawSize.y, y.value(i)));
			nvgStrokeWidth(vg, width);
			nvgStrokeColor(vg, color);
			nvgStroke(vg);
			width += widthStep;
			color.a += alphaStep;
		}
		nvgBeginPath(vg);
		nvgCircle(vg, cvToPixel(_midX, _drawSize.x, x.value(0)), cvToPixel(_midY, _drawSize.y, y.value(0)), 0.5f * width);
		nvgStrokeColor(vg, color);
		nvgFillColor(vg, color);
		nvgStroke(vg);
		nvgFill(vg);

		nvgRestore(vg);
	}

	inline float cvToPixel(float mid, float extent, float cv) {
		return mid + 0.1f * extent * cv;
	}
};

struct Walk2Widget : ModuleWidget {
	static constexpr int hp = 14;

	Walk2Widget(Walk2* module) : ModuleWidget(module) {
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);

		{
			SVGPanel *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(SVG::load(assetPlugin(plugin, "res/Walk2.svg")));
			addChild(panel);
		}

		{
			auto inset = Vec(10, 25);
			int dim = box.size.x - 2*inset.x;
			auto size = Vec(dim, dim);
			auto display = new Walk2Display(module, size);
			display->box.pos = inset;
			display->box.size = size;
			addChild(display);
		}

		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

		// generated by svg_widgets.rb
		auto rateXParamPosition = Vec(28.0, 240.0);
		auto rateYParamPosition = Vec(151.5, 240.0);
		auto offsetXParamPosition = Vec(75.0, 234.0);
		auto offsetYParamPosition = Vec(119.0, 234.0);
		auto scaleXParamPosition = Vec(75.0, 262.5);
		auto scaleYParamPosition = Vec(119.0, 262.5);
		auto trackXParamPosition = Vec(94.0, 290.7);
		auto trackYParamPosition = Vec(131.0, 290.7);

		auto holdXInputPosition = Vec(10.5, 284.0);
		auto rateXInputPosition = Vec(10.5, 323.0);
		auto holdYInputPosition = Vec(145.5, 284.0);
		auto rateYInputPosition = Vec(145.5, 323.0);
		auto jumpInputPosition = Vec(78.0, 313.0);

		auto holdXOutputPosition = Vec(41.5, 284.0);
		auto outXOutputPosition = Vec(41.5, 323.0);
		auto holdYOutputPosition = Vec(176.5, 284.0);
		auto outYOutputPosition = Vec(176.5, 323.0);
		auto distanceOutputPosition = Vec(109.0, 313.0);

		auto trackXLightPosition = Vec(72.0, 292.0);
		auto trackYLightPosition = Vec(109.0, 292.0);
		// end generated by svg_widgets.rb

		addParam(ParamWidget::create<Knob29>(rateXParamPosition, module, Walk2::RATE_X_PARAM, 0.0, 1.0, 0.1));
		addParam(ParamWidget::create<Knob29>(rateYParamPosition, module, Walk2::RATE_Y_PARAM, 0.0, 1.0, 0.1));
		addParam(ParamWidget::create<Knob16>(offsetXParamPosition, module, Walk2::OFFSET_X_PARAM, -1.0, 1.0, 0.0));
		addParam(ParamWidget::create<Knob16>(offsetYParamPosition, module, Walk2::OFFSET_Y_PARAM, -1.0, 1.0, 0.0));
		addParam(ParamWidget::create<Knob16>(scaleXParamPosition, module, Walk2::SCALE_X_PARAM, 0.0, 1.0, 1.0));
		addParam(ParamWidget::create<Knob16>(scaleYParamPosition, module, Walk2::SCALE_Y_PARAM, 0.0, 1.0, 1.0));
		addParam(ParamWidget::create<StatefulButton9>(trackXParamPosition, module, Walk2::TRACK_X_PARAM, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<StatefulButton9>(trackYParamPosition, module, Walk2::TRACK_Y_PARAM, 0.0, 1.0, 0.0));

		addInput(Port::create<Port24>(holdXInputPosition, Port::INPUT, module, Walk2::HOLD_X_INPUT));
		addInput(Port::create<Port24>(rateXInputPosition, Port::INPUT, module, Walk2::RATE_X_INPUT));
		addInput(Port::create<Port24>(holdYInputPosition, Port::INPUT, module, Walk2::HOLD_Y_INPUT));
		addInput(Port::create<Port24>(rateYInputPosition, Port::INPUT, module, Walk2::RATE_Y_INPUT));
		addInput(Port::create<Port24>(jumpInputPosition, Port::INPUT, module, Walk2::JUMP_INPUT));

		addOutput(Port::create<Port24>(holdXOutputPosition, Port::OUTPUT, module, Walk2::HOLD_X_OUTPUT));
		addOutput(Port::create<Port24>(outXOutputPosition, Port::OUTPUT, module, Walk2::OUT_X_OUTPUT));
		addOutput(Port::create<Port24>(holdYOutputPosition, Port::OUTPUT, module, Walk2::HOLD_Y_OUTPUT));
		addOutput(Port::create<Port24>(outYOutputPosition, Port::OUTPUT, module, Walk2::OUT_Y_OUTPUT));
		addOutput(Port::create<Port24>(distanceOutputPosition, Port::OUTPUT, module, Walk2::DISTANCE_OUTPUT));

		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(trackXLightPosition, module, Walk2::TRACK_X_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(trackYLightPosition, module, Walk2::TRACK_Y_LIGHT));
	}
};

RACK_PLUGIN_MODEL_INIT(Bogaudio, Walk2) {
   Model* modelWalk2 = createModel<Walk2, Walk2Widget>("Bogaudio-Walk2", "Walk2", "");
   return modelWalk2;
}


#include "Walk2.hpp"

#define ZOOM_OUT_KEY "zoom_out"
#define GRID_KEY "grid"
#define COLOR_KEY "color"

void Walk2::onReset() {
	_jumpTrigger.reset();
	_modulationStep = modulationSteps;
}

void Walk2::onSampleRateChange() {
	_modulationStep = modulationSteps;
	_historySteps = (historySeconds * engineGetSampleRate()) / historyPoints;
}

json_t* Walk2::toJson() {
	json_t* root = json_object();
	json_object_set_new(root, ZOOM_OUT_KEY, json_boolean(_zoomOut));
	json_object_set_new(root, GRID_KEY, json_boolean(_drawGrid));
	json_object_set_new(root, COLOR_KEY, json_integer(_traceColor));
	return root;
}

void Walk2::fromJson(json_t* root) {
	json_t* zo = json_object_get(root, ZOOM_OUT_KEY);
	if (zo) {
		_zoomOut = json_is_true(zo);
	}
	json_t* g = json_object_get(root, GRID_KEY);
	if (g) {
		_drawGrid = json_is_true(g);
	}
	json_t* c = json_object_get(root, COLOR_KEY);
	if (c) {
		_traceColor = (TraceColor)json_integer_value(c);
	}
}

inline float scaleRate(float rate) {
	return 0.2f * powf(rate, 5.0f);
}

void Walk2::step() {
	++_modulationStep;
	if (_modulationStep >= modulationSteps) {
		_modulationStep = 0;
		float sampleRate = engineGetSampleRate();

		float rateX = params[RATE_X_PARAM].value;
		if (inputs[RATE_X_INPUT].active) {
			rateX *= clamp(inputs[RATE_X_INPUT].value / 10.0f, 0.0f, 1.0f);
		}
		rateX = scaleRate(rateX);
		_walkX.setParams(sampleRate, rateX);
		_slewX.setParams(sampleRate, std::max((1.0f - rateX) * 100.0f, 0.0f), 10.0f);

		_offsetX = params[OFFSET_X_PARAM].value;
		if (inputs[OFFSET_X_INPUT].active) {
			_offsetX *= clamp(inputs[OFFSET_X_INPUT].value / 5.0f, -1.0f, 1.0f);
		}
		_offsetX *= 5.0f;

		_scaleX = params[SCALE_X_PARAM].value;
		if (inputs[SCALE_X_INPUT].active) {
			_scaleX *= clamp(inputs[SCALE_X_INPUT].value / 10.0f, 0.0f, 1.0f);
		}

		float rateY = params[RATE_Y_PARAM].value;
		if (inputs[RATE_Y_INPUT].active) {
			rateY *= clamp(inputs[RATE_Y_INPUT].value / 10.0f, 0.0f, 1.0f);
		}
		rateY = scaleRate(rateY);
		_walkY.setParams(sampleRate, rateY);
		_slewY.setParams(sampleRate, std::max((1.0f - rateY) * 100.0f, 0.0f), 10.0f);

		_offsetY = params[OFFSET_Y_PARAM].value;
		if (inputs[OFFSET_Y_INPUT].active) {
			_offsetY *= clamp(inputs[OFFSET_Y_INPUT].value / 5.0f, -1.0f, 1.0f);
		}
		_offsetY *= 5.0f;

		_scaleY = params[SCALE_Y_PARAM].value;
		if (inputs[SCALE_Y_INPUT].active) {
			_scaleY *= clamp(inputs[SCALE_Y_INPUT].value / 10.0f, 0.0f, 1.0f);
		}
	}

	Vec* jumpTo = _jumpTo;
	if (jumpTo != NULL) {
		_jumpTo = NULL;
		_walkX.tell(jumpTo->x);
		_walkY.tell(jumpTo->y);
		delete jumpTo;
	}
	else if (_jumpTrigger.process(inputs[JUMP_INPUT].value)) {
		_walkX.jump();
		_walkY.jump();
	}

	float outX = _slewX.next(_walkX.next());
	outX *= _scaleX;
	outX += _offsetX;
	outputs[OUT_X_OUTPUT].value = outX;

	float outY = _slewY.next(_walkY.next());
	outY *= _scaleY;
	outY += _offsetY;
	outputs[OUT_Y_OUTPUT].value = outY;

	outputs[DISTANCE_OUTPUT].value = sqrtf(outX*outX + outY*outY) * 0.707107f; // scaling constant is 10 / squrt(200)

	if (_historyStep == 0) {
		_outsX.push(outX);
		_outsY.push(outY);
	}
	++_historyStep;
	_historyStep %= _historySteps;
}

struct Walk2Display : TransparentWidget {
	const int _insetAround = 4;

	const NVGcolor _axisColor = nvgRGBA(0xff, 0xff, 0xff, 0x70);
	const NVGcolor _defaultTraceColor = nvgRGBA(0x00, 0xff, 0x00, 0xee);

	Walk2* _module;
	const Vec _size;
	const Vec _drawSize;
	int _midX, _midY;
	std::shared_ptr<Font> _font;
	NVGcolor _traceColor = _defaultTraceColor;

	Walk2Display(
		Walk2* module,
		Vec size
	)
	: _module(module)
	, _size(size)
	, _drawSize(2 * (_size.x - 2 * _insetAround), 2 * (_size.y - 2 * _insetAround))
	, _midX(_insetAround + _drawSize.x/2)
	, _midY(_insetAround + _drawSize.y/2)
	, _font(Font::load(assetPlugin(plugin, "res/fonts/inconsolata.ttf")))
	{
	}

	void onMouseDown(EventMouseDown& e) override {
		if (
			e.pos.x > _insetAround &&
			e.pos.x < _size.x - _insetAround &&
			e.pos.y > _insetAround &&
			e.pos.y < _size.y - _insetAround
		) {
			float x = 20.0f * ((e.pos.x - _insetAround) / (float)_drawSize.x);
			x -= 5.0f;
			float y = 20.0f * ((e.pos.y - _insetAround) / (float)_drawSize.y);
			y = 5.0f - y;
			_module->_jumpTo = new Vec(x, y);
		}
	}

	void draw(NVGcontext* vg) override {
		switch (_module->_traceColor) {
			case Walk2::ORANGE_TRACE_COLOR: {
				_traceColor = nvgRGBA(0xff, 0x80, 0x00, 0xee);
				break;
			}
			case Walk2::RED_TRACE_COLOR: {
				_traceColor = nvgRGBA(0xff, 0x00, 0x00, 0xee);
				break;
			}
			case Walk2::BLUE_TRACE_COLOR: {
				_traceColor = nvgRGBA(0x00, 0xdd, 0xff, 0xee);
				break;
			}
			case Walk2::GREEN_TRACE_COLOR:
			default: {
				_traceColor = _defaultTraceColor;
			}
		}
		drawBackground(vg);
		float strokeWidth = std::max(1.0f, 3 - RACK_PLUGIN_UI_RACKSCENE->zoomWidget->zoom);

		nvgSave(vg);
		nvgScissor(vg, _insetAround, _insetAround, _drawSize.x / 2, _drawSize.y / 2);
		if (_module->_zoomOut) {
			nvgScale(vg, 0.5f, 0.5f);
			strokeWidth *= 2.0f;
		}
		else {
			float tx = 1.0f + (clamp(_module->_offsetX, -5.0f, 5.0f) / 5.0f);
			tx *= -_drawSize.x / 4;
			float ty = 1.0f - (clamp(_module->_offsetY, -5.0f, 5.0f) / 5.0f);
			ty *= -_drawSize.y / 4;
			nvgTranslate(vg, tx, ty);
		}
		drawAxes(vg, strokeWidth);
		drawTrace(vg, _traceColor, _module->_outsX, _module->_outsY);
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
		const float shortTick = 4.0f;
		const float longTick = 8.0f;
		float dot = 0.5f * strokeWidth;

		nvgSave(vg);
		nvgStrokeColor(vg, _axisColor);
		nvgStrokeWidth(vg, strokeWidth);

		nvgBeginPath(vg);
		nvgMoveTo(vg, _insetAround, _midY);
		nvgLineTo(vg, _insetAround + _drawSize.x, _midY);
		nvgStroke(vg);

		nvgBeginPath(vg);
		nvgMoveTo(vg, _midX, _insetAround);
		nvgLineTo(vg, _midX, _insetAround + _drawSize.y);
		nvgStroke(vg);

		for (int i = 1; i <= 10; ++i) {
			float tick = i % 5 == 0 ? longTick : shortTick;

			float x = (i * 0.1f) * 0.5f * _drawSize.x;
			nvgBeginPath(vg);
			nvgMoveTo(vg, _midX + x, _midY - tick);
			nvgLineTo(vg, _midX + x, _midY + tick);
			nvgStroke(vg);

			nvgBeginPath(vg);
			nvgMoveTo(vg, _midX - x, _midY - tick);
			nvgLineTo(vg, _midX - x, _midY + tick);
			nvgStroke(vg);

			float y = (i * 0.1f) * 0.5f * _drawSize.y;
			nvgBeginPath(vg);
			nvgMoveTo(vg, _midX - tick, _midY + y);
			nvgLineTo(vg, _midX + tick, _midY + y);
			nvgStroke(vg);

			nvgBeginPath(vg);
			nvgMoveTo(vg, _midX - tick, _midY - y);
			nvgLineTo(vg, _midX + tick, _midY - y);
			nvgStroke(vg);

			if (_module->_drawGrid) {
				for (int j = 1; j <= 10; ++j) {
					float y = (j * 0.1f) * 0.5f * _drawSize.y;

					nvgBeginPath(vg);
					nvgMoveTo(vg, _midX + x - dot, _midY + y);
					nvgLineTo(vg, _midX + x + dot, _midY + y);
					nvgStroke(vg);

					nvgBeginPath(vg);
					nvgMoveTo(vg, _midX - x - dot, _midY + y);
					nvgLineTo(vg, _midX - x + dot, _midY + y);
					nvgStroke(vg);

					nvgBeginPath(vg);
					nvgMoveTo(vg, _midX - x - dot, _midY - y);
					nvgLineTo(vg, _midX - x + dot, _midY - y);
					nvgStroke(vg);

					nvgBeginPath(vg);
					nvgMoveTo(vg, _midX + x - dot, _midY - y);
					nvgLineTo(vg, _midX + x + dot, _midY - y);
					nvgStroke(vg);
				}
			}
		}

		if (_module->_drawGrid) {
			const float tick = shortTick;
			{
				float x = _midX - _drawSize.x / 4;
				float y = _midY - _drawSize.y / 4;

				nvgBeginPath(vg);
				nvgMoveTo(vg, x - tick, y);
				nvgLineTo(vg, x + tick, y);
				nvgStroke(vg);

				nvgBeginPath(vg);
				nvgMoveTo(vg, x, y - tick);
				nvgLineTo(vg, x, y + tick);
				nvgStroke(vg);
			}
			{
				float x = _midX + _drawSize.x / 4;
				float y = _midY - _drawSize.y / 4;

				nvgBeginPath(vg);
				nvgMoveTo(vg, x - tick, y);
				nvgLineTo(vg, x + tick, y);
				nvgStroke(vg);

				nvgBeginPath(vg);
				nvgMoveTo(vg, x, y - tick);
				nvgLineTo(vg, x, y + tick);
				nvgStroke(vg);
			}
			{
				float x = _midX + _drawSize.x / 4;
				float y = _midY + _drawSize.y / 4;

				nvgBeginPath(vg);
				nvgMoveTo(vg, x - tick, y);
				nvgLineTo(vg, x + tick, y);
				nvgStroke(vg);

				nvgBeginPath(vg);
				nvgMoveTo(vg, x, y - tick);
				nvgLineTo(vg, x, y + tick);
				nvgStroke(vg);
			}
			{
				float x = _midX - _drawSize.x / 4;
				float y = _midY + _drawSize.y / 4;

				nvgBeginPath(vg);
				nvgMoveTo(vg, x - tick, y);
				nvgLineTo(vg, x + tick, y);
				nvgStroke(vg);

				nvgBeginPath(vg);
				nvgMoveTo(vg, x, y - tick);
				nvgLineTo(vg, x, y + tick);
				nvgStroke(vg);
			}
		}

		nvgRestore(vg);
	}

	void drawTrace(NVGcontext* vg, NVGcolor color, HistoryBuffer<float>& x, HistoryBuffer<float>& y) {
		nvgSave(vg);
		// nvgGlobalCompositeOperation(vg, NVG_LIGHTER);

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
		if (_module->_zoomOut) {
			beginWidth *= 2.0f;
			endWidth *= 2.0f;
		}
		float widthStep = (beginWidth - endWidth) / (float)n;
		float width = endWidth;
		float endAlpha = 0.1f;
		float alphaStep = (color.a - endAlpha) / (float)n;
		color.a = endAlpha;
		for (int i = n - 1; i > 0; --i) {
			nvgBeginPath(vg);
			nvgMoveTo(vg, cvToPixelX(_midX, _drawSize.x, x.value(i - 1)), cvToPixelY(_midY, _drawSize.y, y.value(i - 1)));
			nvgLineTo(vg, cvToPixelX(_midX, _drawSize.x, x.value(i)), cvToPixelY(_midY, _drawSize.y, y.value(i)));
			nvgStrokeWidth(vg, width);
			nvgStrokeColor(vg, color);
			nvgStroke(vg);
			width += widthStep;
			color.a += alphaStep;
		}
		nvgBeginPath(vg);
		nvgCircle(vg, cvToPixelX(_midX, _drawSize.x, x.value(0)), cvToPixelY(_midY, _drawSize.y, y.value(0)), 0.5f * width);
		nvgStrokeColor(vg, color);
		nvgFillColor(vg, color);
		nvgStroke(vg);
		nvgFill(vg);

		nvgRestore(vg);
	}

	inline float cvToPixelX(float mid, float extent, float cv) {
		return mid + 0.05f * extent * cv;
	}

	inline float cvToPixelY(float mid, float extent, float cv) {
		return mid - 0.05f * extent * cv;
	}
};

struct ZoomOutMenuItem : MenuItem {
	Walk2* _module;
	const bool _zoomOut;

	ZoomOutMenuItem(Walk2* module, const char* label, bool zoomOut)
	: _module(module)
	, _zoomOut(zoomOut)
	{
		this->text = label;
	}

	void onAction(EventAction &e) override {
		_module->_zoomOut = _zoomOut;
	}

	void step() override {
		rightText = _module->_zoomOut == _zoomOut ? "?" : "";
	}
};

struct GridMenuItem : MenuItem {
	Walk2* _module;

	GridMenuItem(Walk2* module, const char* label) : _module(module) {
		this->text = label;
	}

	void onAction(EventAction &e) override {
		_module->_drawGrid = !_module->_drawGrid;
	}

	void step() override {
		rightText = _module->_drawGrid ? "?" : "";
	}
};

struct ColorMenuItem : MenuItem {
	Walk2* _module;
	const Walk2::TraceColor _color;

	ColorMenuItem(Walk2* module, const char* label, Walk2::TraceColor color)
	: _module(module)
	, _color(color)
	{
		this->text = label;
	}

	void onAction(EventAction &e) override {
		_module->_traceColor = _color;
	}

	void step() override {
		rightText = _module->_traceColor == _color ? "?" : "";
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

		auto offsetXInputPosition = Vec(10.5, 284.0);
		auto scaleXInputPosition = Vec(41.5, 284.0);
		auto rateXInputPosition = Vec(10.5, 323.0);
		auto offsetYInputPosition = Vec(145.5, 284.0);
		auto scaleYInputPosition = Vec(176.5, 284.0);
		auto rateYInputPosition = Vec(145.5, 323.0);
		auto jumpInputPosition = Vec(78.0, 303.0);

		auto outXOutputPosition = Vec(41.5, 323.0);
		auto outYOutputPosition = Vec(176.5, 323.0);
		auto distanceOutputPosition = Vec(109.0, 303.0);
		// end generated by svg_widgets.rb

		addParam(ParamWidget::create<Knob29>(rateXParamPosition, module, Walk2::RATE_X_PARAM, 0.0, 1.0, 0.1));
		addParam(ParamWidget::create<Knob29>(rateYParamPosition, module, Walk2::RATE_Y_PARAM, 0.0, 1.0, 0.1));
		addParam(ParamWidget::create<Knob16>(offsetXParamPosition, module, Walk2::OFFSET_X_PARAM, -1.0, 1.0, 0.0));
		addParam(ParamWidget::create<Knob16>(offsetYParamPosition, module, Walk2::OFFSET_Y_PARAM, -1.0, 1.0, 0.0));
		addParam(ParamWidget::create<Knob16>(scaleXParamPosition, module, Walk2::SCALE_X_PARAM, 0.0, 1.0, 1.0));
		addParam(ParamWidget::create<Knob16>(scaleYParamPosition, module, Walk2::SCALE_Y_PARAM, 0.0, 1.0, 1.0));

		addInput(Port::create<Port24>(offsetXInputPosition, Port::INPUT, module, Walk2::OFFSET_X_INPUT));
		addInput(Port::create<Port24>(scaleXInputPosition, Port::INPUT, module, Walk2::SCALE_X_INPUT));
		addInput(Port::create<Port24>(rateXInputPosition, Port::INPUT, module, Walk2::RATE_X_INPUT));
		addInput(Port::create<Port24>(offsetYInputPosition, Port::INPUT, module, Walk2::OFFSET_Y_INPUT));
		addInput(Port::create<Port24>(scaleYInputPosition, Port::INPUT, module, Walk2::SCALE_Y_INPUT));
		addInput(Port::create<Port24>(rateYInputPosition, Port::INPUT, module, Walk2::RATE_Y_INPUT));
		addInput(Port::create<Port24>(jumpInputPosition, Port::INPUT, module, Walk2::JUMP_INPUT));

		addOutput(Port::create<Port24>(outXOutputPosition, Port::OUTPUT, module, Walk2::OUT_X_OUTPUT));
		addOutput(Port::create<Port24>(outYOutputPosition, Port::OUTPUT, module, Walk2::OUT_Y_OUTPUT));
		addOutput(Port::create<Port24>(distanceOutputPosition, Port::OUTPUT, module, Walk2::DISTANCE_OUTPUT));
	}

	void appendContextMenu(Menu* menu) override {
		Walk2* w = dynamic_cast<Walk2*>(module);
		assert(w);

		menu->addChild(new MenuLabel());
		menu->addChild(new ZoomOutMenuItem(w, "Display range: +/-5V", false));
		menu->addChild(new ZoomOutMenuItem(w, "Display range: +/-10V", true));
		menu->addChild(new MenuLabel());
		menu->addChild(new GridMenuItem(w, "Show grid"));
		menu->addChild(new MenuLabel());
		menu->addChild(new ColorMenuItem(w, "Trace color: green", Walk2::GREEN_TRACE_COLOR));
		menu->addChild(new ColorMenuItem(w, "Trace color: orange", Walk2::ORANGE_TRACE_COLOR));
		menu->addChild(new ColorMenuItem(w, "Trace color: red", Walk2::RED_TRACE_COLOR));
		menu->addChild(new ColorMenuItem(w, "Trace color: blue", Walk2::BLUE_TRACE_COLOR));
	}
};

RACK_PLUGIN_MODEL_INIT(Bogaudio, Walk2) {
   Model* modelWalk2 = createModel<Walk2, Walk2Widget>("Bogaudio-Walk2", "Walk2", "");
   return modelWalk2;
}

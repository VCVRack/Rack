
#include "Blank6.hpp"

void Blank6::onSampleRateChange() {
	_rms.setSampleRate(engineGetSampleRate());
}

void Blank6::step() {
	if (inputs[IN_INPUT].active) {
		_level = _rms.next(inputs[IN_INPUT].value) / 5.0f;
	}
	else {
		_level = -1.0f;
	}
}

struct Blank6Display : OpaqueWidget {
	const NVGcolor textColor = nvgRGBA(0x33, 0x33, 0x33, 0xff);
	const NVGcolor bgTextColor = nvgRGBA(0xaa, 0xaa, 0xaa, 0xff);
	const NVGcolor bgColor = nvgRGBA(0xdd, 0xdd, 0xdd, 0xff);
	Blank6* _module;
	const char* _text;
	std::shared_ptr<Font> _font;

	Blank6Display(Blank6* module, const char* text)
	: _module(module)
	, _text(text)
	, _font(Font::load(assetPlugin(plugin, "res/fonts/audiowide.ttf")))
	{
	}

	void draw(NVGcontext* vg) override {
		float offsetX = box.size.x / 2.0f;
		float offsetY = box.size.y / 2.0f;
		nvgSave(vg);
		nvgTranslate(vg, offsetX, offsetY);
		nvgRotate(vg, M_PI/2.0f);
		nvgTranslate(vg, -offsetY, offsetX);
		nvgFontSize(vg, 52.0f);
		nvgFontFaceId(vg, _font->handle);
		nvgTextLetterSpacing(vg, 9.0f);
		if (_module->_level < 0.0f) {
			nvgFillColor(vg, textColor);
			nvgText(vg, 0, 0, _text, NULL);
		}
		else {
			nvgFillColor(vg, bgTextColor);
			nvgText(vg, 0, 0, _text, NULL);
			if (_module->_level > 0.0001f) {
				nvgFillColor(vg, decibelsToColor(amplitudeToDecibels(_module->_level)));
				nvgText(vg, 0, 0, _text, NULL);
			}
		}
		nvgBeginPath(vg);
		nvgRect(vg, 97, -20, 10, 10);
		nvgFillColor(vg, bgColor);
		nvgFill(vg);
		nvgRestore(vg);
	}
};

struct Blank6Widget : ModuleWidget {
	static constexpr int hp = 6;

	Blank6Widget(Blank6* module) : ModuleWidget(module) {
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);

		{
			SVGPanel *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(SVG::load(assetPlugin(plugin, "res/Blank6.svg")));
			addChild(panel);
		}

		{
			auto display = new Blank6Display(module, "BOGAUDIO");
			display->box.pos = Vec(30, 32);
			display->box.size = Vec(30, 316);
			addChild(display);
		}

		addChild(Widget::create<ScrewSilver>(Vec(0, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15, 365)));

		addInput(Port::create<BlankPort24>(Vec(33, 346), Port::INPUT, module, Blank6::IN_INPUT));
	}
};

RACK_PLUGIN_MODEL_INIT(Bogaudio, Blank6) {
   Model *modelBlank6 = createModel<Blank6, Blank6Widget>("Bogaudio-Blank6", "Blank6", "blank panel", BLANK_TAG);
   return modelBlank6;
}

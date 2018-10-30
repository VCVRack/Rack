#include <global_pre.hpp>
#include <global_ui.hpp>
#include "SubmarineFree.hpp"
#include "window.hpp"
#include "torpedo.hpp"

namespace rack_plugin_SubmarineFree {

struct TD_116;

struct TDInput : Torpedo::PatchInputPort {
	TD_116 *tdModule;
	TDInput(TD_116 *module, unsigned int portNum) : Torpedo::PatchInputPort((Module *)module, portNum) { tdModule = module; }
	void received(std::string pluginName, std::string moduleName, json_t *rootJ) override;
	NVGcolor decodeColor(std::string colorStr);
};

struct TD_116 : Module {
	TDInput inPort = TDInput(this, 0);
	Torpedo::PatchOutputPort outPort = Torpedo::PatchOutputPort(this, 0);
	TD_116() : Module (0, 1, 1, 0) {outPort.size(1);}
	void step() override {
		inPort.process();
		outPort.process();
	}
	void sendText(std::string text) {
		json_t *rootJ = json_object();;

		// text
		json_object_set_new(rootJ, "text", json_string(text.c_str()));

		outPort.send("SubmarineFree", "TDNotesText", rootJ); 
	}
	std::string text;
	int fontSize = 12;
	NVGcolor fg = nvgRGB(0x28, 0xb0, 0xf3);
	NVGcolor bg = nvgRGB(0,0,0);
	int isDirty = false;
	int isDirtyC = false;
};

struct TDText : LedDisplayTextField {
	TD_116 *tdModule;
	NVGcolor bgColor = nvgRGB(0x00, 0x00, 0x00);
	int fontSize = 12;
	TDText() {
		color = nvgRGB(0x28, 0xb0, 0xf3);
	}
	void onTextChange() override {
		LedDisplayTextField::onTextChange();
		tdModule->sendText(text);
	}
	int getTextPosition(Vec mousePos) override {
	    bndSetFont(font->handle);
	    int textPos = bndIconLabelTextPosition(rack::global_ui->window.gVg, textOffset.x, textOffset.y,
	      box.size.x - 2*textOffset.x, box.size.y - 2*textOffset.y,
	      -1, fontSize, text.c_str(), mousePos.x, mousePos.y);
	    bndSetFont(rack::global_ui->window.gGuiFont->handle);
	    return textPos;
	}
	void draw(NVGcontext *vg) override {
		nvgScissor(vg, 0, 0, box.size.x, box.size.y);
		//Background
		nvgBeginPath(vg);
		nvgRoundedRect(vg, 0, 0, box.size.x, box.size.y, 5.0);
		nvgFillColor(vg, bgColor);
		nvgFill(vg);

		//Text
		if (font->handle >= 0) {
			bndSetFont(font->handle);
			
			NVGcolor highlightColor = color;
			highlightColor.a = 0.5;
			int begin = min(cursor, selection);
			int end = (this == rack::global_ui->widgets.gFocusedWidget) ? max(cursor, selection) : -1;
			bndIconLabelCaret(vg, textOffset.x, textOffset.y,
				box.size.x - 2*textOffset.x, box.size.y - 2*textOffset.y,
				-1, color, fontSize, text.c_str(), highlightColor, begin, end);
		}
		nvgResetScissor(vg);
		bndSetFont(rack::global_ui->window.gGuiFont->handle);
	}
};

NVGcolor TDInput::decodeColor(std::string colorStr) {
	int r = (colorStr[0] - 'A') * 16 + (colorStr[1] - 'A');
	int g = (colorStr[2] - 'A') * 16 + (colorStr[3] - 'A');
	int b = (colorStr[4] - 'A') * 16 + (colorStr[5] - 'A');
	return nvgRGB(r, g, b);
}

void TDInput::received(std::string pluginName, std::string moduleName, json_t *rootJ) {
	if (pluginName.compare("SubmarineFree")) return;
	if (!moduleName.compare("TDNotesText")) { 
		json_t *text = json_object_get(rootJ, "text");
		if (text) {
			tdModule->text.assign(json_string_value(text));
			tdModule->isDirty = true;
		}
	}
	else if (!moduleName.compare("TDNotesColor")) {
		json_t *size = json_object_get(rootJ, "size");
		if (size) {
			tdModule->fontSize = json_number_value(size);
			tdModule->isDirtyC = true;
		}	
		json_t *fg = json_object_get(rootJ, "fg");
		if (fg) {
			tdModule->fg = decodeColor(std::string(json_string_value(fg)));
			tdModule->isDirtyC = true;
		}
		json_t *bg = json_object_get(rootJ, "bg");
		if (bg) {
			tdModule->bg = decodeColor(std::string(json_string_value(bg)));
			tdModule->isDirtyC = true;
		}
	}	
}

struct TD116 : ModuleWidget {
	TDText *textField;

	TD116(TD_116 *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/TD-116.svg")));

		addInput(Port::create<BlackPort>(Vec(4,19), Port::INPUT, module, 0));
		addOutput(Port::create<BlackPort>(Vec(211,19), Port::OUTPUT, module, 0));	

		textField = Widget::create<TDText>(mm2px(Vec(3.39962, 15.8373)));
		textField->box.size = mm2px(Vec(74.480, 102.753));
		textField->multiline = true;
		textField->tdModule = module;
		addChild(textField);
	}

	json_t *toJson() override {
		json_t *rootJ = ModuleWidget::toJson();

		json_object_set_new(rootJ, "text", json_string(textField->text.c_str()));
		json_object_set_new(rootJ, "size", json_real(textField->fontSize));
		json_object_set_new(rootJ, "fg", json_string(colorToHexString(textField->color).c_str()));
		json_object_set_new(rootJ, "bg", json_string(colorToHexString(textField->bgColor).c_str()));

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		ModuleWidget::fromJson(rootJ);

		json_t *textJ = json_object_get(rootJ, "text");
		if (textJ)
			textField->text = json_string_value(textJ);
		json_t *sizeJ = json_object_get(rootJ, "size");
		if (sizeJ)
			textField->fontSize = json_number_value(sizeJ);
		json_t *fgJ = json_object_get(rootJ, "fg");
		if (fgJ) {
			if (json_is_object(fgJ)) 
				textField->color = jsonToColor(fgJ);
			else
				textField->color = colorFromHexString(json_string_value(fgJ));
		}
		json_t *bgJ = json_object_get(rootJ, "bg");
		if (bgJ) {
			if (json_is_object(bgJ))
				textField->bgColor = jsonToColor(bgJ);
			else
				textField->bgColor = colorFromHexString(json_string_value(bgJ));
		}
	}

	void step() override {
		TD_116 *tdModule = dynamic_cast<TD_116 *>(module);
		if (tdModule->isDirty) {
			textField->text = tdModule->text;
			tdModule->isDirty = false;
		}
		if (tdModule->isDirtyC) {
			textField->fontSize = tdModule->fontSize;
			textField->color = tdModule->fg;
			textField->bgColor = tdModule->bg;
			tdModule->isDirtyC = false;
		}
		ModuleWidget::step();
	}

	void reset() override {
		textField->fontSize = 12;
		textField->text = "";
		textField->color = nvgRGB(0x28, 0xb0, 0xf3);
		textField->bgColor = nvgRGB(0,0,0);	
		ModuleWidget::reset();
	}

	void appendContextMenu(Menu *menu) override;
};

struct TD116_MenuItem : MenuItem {
	TD116 *widget;
	NVGcolor color;
	void onAction(EventAction &e) override {
		widget->textField->tdModule->fg = color;
		widget->textField->color = color;
	}
};

void TD116::appendContextMenu(Menu *menu) {
	menu->addChild(MenuEntry::create());
	TD116_MenuItem *m = MenuItem::create<TD116_MenuItem>("Blue");
	m->widget = this;
	m->color = nvgRGB(0x28, 0xb0, 0xf3);
	menu->addChild(m);
	
	m = MenuItem::create<TD116_MenuItem>("Yellow");
	m->widget = this;
	m->color = nvgRGB(0xc9, 0xb7, 0x0e);
	menu->addChild(m);

	m = MenuItem::create<TD116_MenuItem>("Red");
	m->widget = this;
	m->color = nvgRGB(0xff, 0x13, 0x13);
	menu->addChild(m);

	m = MenuItem::create<TD116_MenuItem>("Green");
	m->widget = this;
	m->color = nvgRGB(0x0a, 0xff, 0x13);
	menu->addChild(m);

	m = MenuItem::create<TD116_MenuItem>("Orange");
	m->widget = this;
	m->color = nvgRGB(0xff, 0xa5, 0x2d);
	menu->addChild(m);

	m = MenuItem::create<TD116_MenuItem>("Pink");
	m->widget = this;
	m->color = nvgRGB(0xff, 0x7d, 0xec);
	menu->addChild(m);

	m = MenuItem::create<TD116_MenuItem>("White");
	m->widget = this;
	m->color = nvgRGB(0xff, 0xff, 0xff);
	menu->addChild(m);

}

} // namespace rack_plugin_SubmarineFree

using namespace rack_plugin_SubmarineFree;

RACK_PLUGIN_MODEL_INIT(SubmarineFree, TD116) {
   Model *modelTD116 = Model::create<TD_116, TD116>("Submarine (Free)", "TD-116", "TD-116 Text Display", VISUAL_TAG);
   return modelTD116;
}

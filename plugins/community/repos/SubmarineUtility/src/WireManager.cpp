#include "SubControls.hpp"
#include <map>
#include <algorithm>
#include "global_pre.hpp"
#include "global_ui.hpp"
#include "window.hpp"
#include "osdialog.h"

namespace rack_plugin_SubmarineUtility {

struct WireManagerWidget;

// Icons

struct WMIconWidget : SubControls::ButtonBase,SVGWidget {
	WireManagerWidget *wmw;
};

struct WMMinimizeIcon : WMIconWidget {
	WMMinimizeIcon() {
		box.size.x = 30;
		box.size.y = 30;
	}
	void onAction(EventAction &e) override;
};

struct WMColorIcon : WMIconWidget {
	WMColorIcon() {
		box.size.x = 30;
		box.size.y = 30;
	}
	void onAction(EventAction &e) override;
};

struct WMOptionIcon : WMIconWidget {
	WMOptionIcon() {
		box.size.x = 30;
		box.size.y = 30;
	}
	void onAction(EventAction &e) override;
};

struct WMHighlightButton : SubControls::RadioButton {
	WireManagerWidget *wmw;
	int status;
	void onAction(EventAction &e) override;	
};

struct WMWireCheck : SubControls::CheckButton {
	WireManagerWidget *wmw;
	void onAction(EventAction &e) override;
};

struct WMWireAdd : SubControls::ButtonBase {
	WireManagerWidget *wmw;
	void onAction(EventAction &e) override;
	void draw(NVGcontext *vg) override {
		nvgBeginPath(vg);
		nvgFillColor(vg, nvgRGB(0xff, 0xff, 0xff));
		nvgRect(vg, box.size.x / 2 - 1.5, 2, 3, box.size.y - 4);
		nvgFill(vg);
		nvgBeginPath(vg);
		nvgRect(vg, 2, box.size.y / 2 - 1.5, box.size.x - 4, 3);
		nvgFill(vg);
		SubControls::ButtonBase::draw(vg);
	}
};

struct WMWireEdit;
struct WMWireUp;
struct WMWireDown;

struct WMWireButton : VirtualWidget {
	NVGcolor color;
	WMWireCheck *wmc;
	WMWireEdit *wme;
	WMWireUp *wmu;
	WMWireDown *wmd;
	WMWireButton();
	void draw(NVGcontext *vg) override {
		NVGcolor colorOutline = nvgLerpRGBA(color, nvgRGBf(0.0, 0.0, 0.0), 0.5);

		nvgBeginPath(vg);
		nvgMoveTo(vg, 32, box.size.y / 2);
		nvgLineTo(vg, box.size.x - 40, box.size.y / 2);
		nvgStrokeColor(vg, colorOutline);
		nvgStrokeWidth(vg, 5);
		nvgStroke(vg);

		nvgStrokeColor(vg, color);
		nvgStrokeWidth(vg, 3);
		nvgStroke(vg);

		nvgBeginPath(vg);
		nvgCircle(vg, 32, box.size.y / 2, 9);
		nvgFillColor(vg, color);
		nvgFill(vg);

		nvgStrokeWidth(vg, 1.0);
		nvgStrokeColor(vg, colorOutline);
		nvgStroke(vg);
	
		nvgBeginPath(vg);
		nvgCircle(vg, 32, box.size.y / 2, 5);
		nvgFillColor(vg, nvgRGBf(0.0, 0.0, 0.0));
		nvgFill(vg);

		Widget::draw(vg);
	}
};

struct WMCheckAll : SubControls::CheckButton {
	WireManagerWidget *wmw;
	void onAction(EventAction &e) override;
};

struct WMManageButton : VirtualWidget {
	WMCheckAll *wmc;
	WMWireAdd *wma;
	WMManageButton() {
		wmc = Widget::create<WMCheckAll>(Vec(1,1));
		wmc->box.size.x = 19;
		wmc->box.size.y = 19;
		addChild(wmc);
		wma = Widget::create<WMWireAdd>(Vec(108, 1));
		wma->box.size.x = 19;
		wma->box.size.y = 19;
		addChild(wma);
	}
}; 

struct WMWireEdit : SubControls::ButtonBase {
	WMWireButton *wmb;
	void onAction(EventAction &e) override;
	void draw(NVGcontext *vg) override {
		nvgBeginPath(vg);
		nvgFillColor(vg, nvgRGB(0, 0, 0));
		nvgRect(vg, 0, 0, box.size.x, box.size.y);
		nvgFill(vg);
		nvgBeginPath(vg);
		nvgFillColor(vg, nvgRGB(0xff, 0xff, 0xff));
		nvgCircle(vg, box.size.x / 2 - 5, box.size.y / 2, 2);
		nvgCircle(vg, box.size.x / 2, box.size.y / 2, 2);
		nvgCircle(vg, box.size.x / 2 + 5, box.size.y / 2, 2);
		nvgFill(vg);
		ButtonBase::draw(vg);
	}
};

struct WMWireUp : SubControls::ButtonBase {
	WMWireButton *wmb;
	void onAction(EventAction &e) override;
	void draw(NVGcontext *vg) override {
		nvgBeginPath(vg);
		nvgFillColor(vg, nvgRGB(0, 0, 0));
		nvgRect(vg, 0, 0, box.size.x, box.size.y);
		nvgFill(vg);
		if (wmb->box.pos.y > 22) {
			nvgBeginPath(vg);
			nvgFillColor(vg, nvgRGB(0xff, 0xff, 0xff));
			nvgMoveTo(vg, box.size.x / 2, 1);
			nvgLineTo(vg, box.size.x - 1, box.size.y - 1);
			nvgLineTo(vg, 1, box.size.y - 1);
			nvgClosePath(vg);
			nvgFill(vg);
		}
		ButtonBase::draw(vg);
	}
};

struct WMWireDown : SubControls::ButtonBase {
	WMWireButton *wmb;
	void onAction(EventAction &e) override;
	void draw(NVGcontext *vg) override {
		nvgBeginPath(vg);
		nvgFillColor(vg, nvgRGB(0, 0, 0));
		nvgRect(vg, 0, 0, box.size.x, box.size.y);
		nvgFill(vg);
		if ((wmb->box.pos.y / 21) < (wmb->parent->children.size() - 1)) {
			nvgBeginPath(vg);
			nvgFillColor(vg, nvgRGB(0xff, 0xff, 0xff));
			nvgMoveTo(vg, box.size.x / 2, box.size.y - 1);
			nvgLineTo(vg, box.size.x - 1, 1);
			nvgLineTo(vg, 1, 1);
			nvgClosePath(vg);
			nvgFill(vg);
		}
		ButtonBase::draw(vg);
	}
};

WMWireButton::WMWireButton() {
	wmc = Widget::create<WMWireCheck>(Vec(1,1));
	wmc->box.size.x = 19;
	wmc->box.size.y = 19;
	addChild(wmc);
	wme = Widget::create<WMWireEdit>(Vec(108,1));
	wme->box.size.x = 19;
	wme->box.size.y = 19;
	wme->wmb = this; 
	addChild(wme);
	wmu = Widget::create<WMWireUp>(Vec(92,2));
	wmu->box.size.x = 15;
	wmu->box.size.y = 7;
	wmu->wmb = this;
	addChild(wmu);
	wmd = Widget::create<WMWireDown>(Vec(92,12));
	wmd->box.size.x = 15;
	wmd->box.size.y = 7;
	wmd->wmb = this;
	addChild(wmd);
}

struct WMSlider : SubControls::HSlider {
	WireManagerWidget *wmw;
	void onAction(EventAction &e) override;
};

struct WMEditWidget : VirtualWidget {
	WireManagerWidget *wmw;
	void draw(NVGcontext *vg) override;
};

struct WMDeleteWidget : VirtualWidget {
	WireManagerWidget *wmw;
	void draw(NVGcontext *vg) override;
};

struct WMSaveButton : SubControls::ClickButton {
	WireManagerWidget *wmw;
	void onAction(EventAction &e) override;
};

struct WMCancelButton : SubControls::ClickButton {
	WireManagerWidget *wmw;
	void onAction(EventAction &e) override;
};

struct WMDeleteButton : SubControls::ClickButton {
	WireManagerWidget *wmw;
	void onAction(EventAction &e) override;
};

struct WMOKButton : SubControls::ClickButton {
	WireManagerWidget *wmw;
	void onAction(EventAction &e) override;
};

struct WireManagerWidget : SubControls::SizeableModuleWidget {

	enum {
		HIGHLIGHT_OFF,
		HIGHLIGHT_LOW,
		HIGHLIGHT_ON
	};

	WMColorIcon *colorIcon;
	WMOptionIcon *optionIcon;
	WMMinimizeIcon *minimizeIcon;
	ScrollWidget *colorWidget;
	VirtualWidget *optionWidget;
	WMEditWidget *editWidget;
	WMDeleteWidget *deleteWidget;
	WMDeleteButton *deleteButton;
	
	WMHighlightButton *highlightOff;
	WMHighlightButton *highlightLow;
	WMHighlightButton *highlightOn;
	WMSlider *highlightSlider;
	WMWireCheck *varyCheck;
	WMSlider *varyH;
	WMSlider *varyS;
	WMSlider *varyL;

	SubControls::HSlider *varyR;
	SubControls::HSlider *varyG;
	SubControls::HSlider *varyB;

	int wireCount = 0;
	Widget *lastWire = NULL;
	int highlight = 0;
	unsigned int newColorIndex = 1;
	WMWireButton *editingColor;
	ModuleWidget *lastHover = NULL;
	int highlightIsDirty = true;

	WireManagerWidget(Module *module) : SubControls::SizeableModuleWidget(module) {
		moduleName = "Wire Manager";
		moduleWidth = 150;
		minimumWidth = 150;
		sizeable = false;
		box.size.x = moduleWidth;
		Resize();

		colorIcon = Widget::create<WMColorIcon>(Vec(2, 2));
		colorIcon->wmw = this;
#define plugin "SubmarineUtility"
		colorIcon->setSVG(SVG::load(assetPlugin(plugin, "res/colors.svg")));
		backPanel->addChild(colorIcon);

		optionIcon = Widget::create<WMOptionIcon>(Vec(34, 2));
		optionIcon->wmw = this;
		optionIcon->setSVG(SVG::load(assetPlugin(plugin, "res/hls.svg")));
		backPanel->addChild(optionIcon);
	
		minimizeIcon = Widget::create<WMMinimizeIcon>(Vec(66, 2));
		minimizeIcon->wmw = this;
		minimizeIcon->setSVG(SVG::load(assetPlugin(plugin, "res/min.svg")));
		backPanel->addChild(minimizeIcon);	

		colorWidget = Widget::create<ScrollWidget>(Vec(0,35));
		colorWidget->box.size.x = box.size.x - 20;
		colorWidget->box.size.y = box.size.y - 65;
		backPanel->addChild(colorWidget);

		WMManageButton *wb = Widget::create<WMManageButton>(Vec(0, 0));
		wb->wmc->wmw = this;
		wb->wma->wmw = this;
		wb->box.size.x = colorWidget->box.size.x;
		wb->box.size.y = 21;
		colorWidget->container->addChild(wb);

		optionWidget = Widget::create<VirtualWidget>(Vec(0, 35));
		optionWidget->box.size.x = box.size.x - 20;
		optionWidget->box.size.y = box.size.y - 65;
		optionWidget->visible = false;
		backPanel->addChild(optionWidget);

		varyCheck = Widget::create<WMWireCheck>(Vec(10, 5));
		varyCheck->wmw = this;
		varyCheck->label = "Variation";
		varyCheck->box.size.x = box.size.x - 40;
		varyCheck->box.size.y = 19;
		optionWidget->addChild(varyCheck);
	
		SubControls::Label *label = Widget::create<SubControls::Label>(Vec(10, 25));
		label->label = "H";
		label->box.size.x = 10;
		label->box.size.y = 19;
		optionWidget->addChild(label);
	
		label = Widget::create<SubControls::Label>(Vec(10, 45));
		label->label = "S";
		label->box.size.x = 10;
		label->box.size.y = 19;
		optionWidget->addChild(label);
	
		label = Widget::create<SubControls::Label>(Vec(10, 65));
		label->label = "L";
		label->box.size.x = 10;
		label->box.size.y = 19;
		optionWidget->addChild(label);
	
		varyH = Widget::create<WMSlider>(Vec(20, 25));
		varyH->wmw = this;
		varyH->box.size.x = box.size.x - 50;
		varyH->box.size.y = 19;
		varyH->minValue = 0.0f;
		varyH->maxValue = 1.0f;
		varyH->value = 0.1f;
		varyH->defaultValue = 0.1f;
		optionWidget->addChild(varyH);

		varyS = Widget::create<WMSlider>(Vec(20, 45));
		varyS->wmw = this;
		varyS->box.size.x = box.size.x - 50;
		varyS->box.size.y = 19;
		varyS->minValue = 0.0f;
		varyS->maxValue = 1.0f;
		varyS->value = 0.1f;
		varyS->defaultValue = 0.1f;
		optionWidget->addChild(varyS);

		varyL = Widget::create<WMSlider>(Vec(20, 65));
		varyL->wmw = this;
		varyL->box.size.x = box.size.x - 50;
		varyL->box.size.y = 19;
		varyL->minValue = 0.0f;
		varyL->maxValue = 1.0f;
		varyL->value = 0.1f;
		varyL->defaultValue = 0.1f;
		optionWidget->addChild(varyL);

		label = Widget::create<SubControls::Label>(Vec(10, 105));
		label->label = "Highlighting";
		label->box.size.x = box.size.x - 40;
		label->box.size.y = 19;
		optionWidget->addChild(label);

		highlightOff = Widget::create<WMHighlightButton>(Vec(10, 125));
		highlightOff->wmw = this;
		highlightOff->status = HIGHLIGHT_OFF;
		highlightOff->box.size.x = box.size.x - 40;
		highlightOff->box.size.y = 19;
		highlightOff->selected = true;
		highlightOff->label = "Off";
		optionWidget->addChild(highlightOff);
	
		highlightLow = Widget::create<WMHighlightButton>(Vec(10, 145));
		highlightLow->wmw = this;
		highlightLow->status = HIGHLIGHT_LOW;
		highlightLow->box.size.x = box.size.x - 40;
		highlightLow->box.size.y = 19;
		highlightLow->label = "When hovering";
		optionWidget->addChild(highlightLow);
	
		highlightOn = Widget::create<WMHighlightButton>(Vec(10, 165));
		highlightOn->wmw = this;
		highlightOn->status = HIGHLIGHT_ON;
		highlightOn->box.size.x = box.size.x - 40;
		highlightOn->box.size.y = 19;
		highlightOn->label = "Always On";
		optionWidget->addChild(highlightOn);
	
		highlightSlider = Widget::create<WMSlider>(Vec(10, 185));
		highlightSlider->wmw = this;
		highlightSlider->box.size.x = box.size.x - 40;
		highlightSlider->box.size.y = 21;
		highlightSlider->minValue = 0;
		highlightSlider->maxValue = 1;
		highlightSlider->value = 0.1;
		highlightSlider->defaultValue = 0.1;
		optionWidget->addChild(highlightSlider);

		editWidget = Widget::create<WMEditWidget>(Vec(0, 35));
		editWidget->wmw = this;
		editWidget->box.size.x = box.size.x - 20;
		editWidget->box.size.y = box.size.y - 65;
		editWidget->visible = false;
		backPanel->addChild(editWidget);
	
		varyR = Widget::create<SubControls::HSlider>(Vec(10, 105));
		varyR->transparent = true;
		varyR->box.size.x = box.size.x - 40;
		varyR->box.size.y = 19;
		varyR->minValue = 0.0f;
		varyR->maxValue = 1.0f;
		varyR->value = 0.5f;
		varyR->defaultValue = 0.5f;
		editWidget->addChild(varyR);

		varyG = Widget::create<SubControls::HSlider>(Vec(10, 145));
		varyG->transparent = true;
		varyG->box.size.x = box.size.x - 40;
		varyG->box.size.y = 19;
		varyG->minValue = 0.0f;
		varyG->maxValue = 1.0f;
		varyG->value = 0.5f;
		varyG->defaultValue = 0.5f;
		editWidget->addChild(varyG);

		varyB = Widget::create<SubControls::HSlider>(Vec(10, 185));
		varyB->transparent = true;
		varyB->box.size.x = box.size.x - 40;
		varyB->box.size.y = 19;
		varyB->minValue = 0.0f;
		varyB->maxValue = 1.0f;
		varyB->value = 0.5f;
		varyB->defaultValue = 0.5f;
		editWidget->addChild(varyB);

		WMSaveButton *saveButton = Widget::create<WMSaveButton>(Vec(5, box.size.y - 90));
		saveButton->wmw = this;
		saveButton->box.size.x = 55;
		saveButton->box.size.y = 19;
		saveButton->label = "Save";
		editWidget->addChild(saveButton);

		WMCancelButton *cancelButton = Widget::create<WMCancelButton>(Vec(box.size.x - 80, box.size.y - 90));
		cancelButton->wmw = this;
		cancelButton->box.size.x = 55;
		cancelButton->box.size.y = 19;
		cancelButton->label = "Cancel";
		editWidget->addChild(cancelButton);
	
		deleteButton = Widget::create<WMDeleteButton>(Vec(box.size.x - 80, box.size.y - 130));
		deleteButton->wmw = this;
		deleteButton->box.size.x = 55;
		deleteButton->box.size.y = 19;
		deleteButton->label = "Delete...";
		editWidget->addChild(deleteButton);

		deleteWidget = Widget::create<WMDeleteWidget>(Vec(0, 35));
		deleteWidget->wmw = this;
		deleteWidget->box.size.x = box.size.x - 20;
		deleteWidget->box.size.y = box.size.y - 65;
		deleteWidget->visible = false;
		backPanel->addChild(deleteWidget);

		label = Widget::create<SubControls::Label>(Vec(25, 195));
		label->label = "Delete Color?";
		label->box.size.x = box.size.x - 40;
		label->box.size.y = 19;
		deleteWidget->addChild(label);

		WMOKButton *okButton = Widget::create<WMOKButton>(Vec(5, box.size.y - 90));
		okButton->wmw = this;
		okButton->box.size.x = 55;
		okButton->box.size.y = 19;
		okButton->label = "Okay";
		deleteWidget->addChild(okButton);

		cancelButton = Widget::create<WMCancelButton>(Vec(box.size.x - 80, box.size.y - 90));
		cancelButton->wmw = this;
		cancelButton->box.size.x = 55;
		cancelButton->box.size.y = 19;
		cancelButton->label = "Cancel";
		deleteWidget->addChild(cancelButton);
	
		LoadSettings();

		wireCount = RACK_PLUGIN_UI_RACKWIDGET->wireContainer->children.size();
		if (wireCount)
			lastWire = RACK_PLUGIN_UI_RACKWIDGET->wireContainer->children.back();
	}

	void _delete() override {
		if (highlight) {
			SetHighlight(HIGHLIGHT_OFF);
			highlightWires();
		}
	}

	void ResetIcons() {
	}

	void AddColor(NVGcolor color, int selected) {
		float y = colorWidget->container->children.size() * 21.0;
		WMWireButton *wb = Widget::create<WMWireButton>(Vec(0, y));
		wb->box.size.x = colorWidget->box.size.x;
		wb->box.size.y = 21;
		wb->color = color;
		wb->wmc->selected = selected;
		wb->wmc->wmw = this;
		colorWidget->container->addChild(wb);
	}

	void SetDefaults() {
		AddColor(nvgRGB(0xc9, 0xb7, 0x0e), true);
		AddColor(nvgRGB(0xc9, 0x18, 0x47), true);
		AddColor(nvgRGB(0x0c, 0x8e, 0x15), true);
		AddColor(nvgRGB(0x09, 0x86, 0xad), true);
		AddColor(nvgRGB(0xff, 0xae, 0xc9), false);
      		AddColor(nvgRGB(0xb7, 0x00, 0xb5), false);
      		AddColor(nvgRGB(0x80, 0x80, 0x80), false);
      		AddColor(nvgRGB(0xff, 0xff, 0xff), false);
      		AddColor(nvgRGB(0x10, 0x0f, 0x12), false);
      		AddColor(nvgRGB(0xff, 0x99, 0x41), false);
      		AddColor(nvgRGB(0x80, 0x36, 0x10), false);
    	}

	NVGcolor findColor(NVGcolor color) {
		auto vi = colorWidget->container->children.begin();
		std::advance(vi, newColorIndex);
		for (int i = 0; i < 2; i++) {
			while(newColorIndex < colorWidget->container->children.size()) {
				newColorIndex++;
				WMWireButton *wb = dynamic_cast<WMWireButton *>(*vi);
				if (wb->wmc->selected) {
					return wb->color;
				}
				std::advance(vi, 1);	
			}
			newColorIndex = 1;
			vi = colorWidget->container->children.begin();
			std::advance(vi, newColorIndex);
		}
		return color;
	}

	NVGcolor varyColor(NVGcolor color) {
		float r = color.r;
		float g = color.g;
		float b = color.b;
		float a = color.a;
	// convert to hsl

		float Cmax = std::max(r,std::max(g,b));
		float Cmin = std::min(r,std::min(g,b));
		float delta = Cmax - Cmin;

		float h = 0;
		float s = 0;
		float l = (Cmax + Cmin) / 2;

		if (delta > 0) {
			s = delta / (1 - std::abs(l * 2  - 1));
			if (Cmax == r) {
				h = std::fmod(6 + (g-b)/delta, 6);
			}
			else if (Cmax == g) {
				h = (b-r)/delta + 2;
			}
			else {
				h = (r-g)/delta + 4;
			}
		}
	// Modify color
		
		h = std::fmod(1 + h / 6 + (randomUniform() - 0.5f) * varyH->value, 1.0f);
		s = rescale(randomUniform(), 0.0f, 1.0f, std::max(s - varyS->value, 0.0f), std::min(s + varyS->value, 1.0f));
		l = rescale(randomUniform(), 0.0f, 1.0f, std::max(l - varyL->value, 0.0f), std::min(l + varyL->value, 1.0f));
		return nvgHSLA(h, s, l, a * 255);
	}

	void colorWire(Widget *widget) {
		WireWidget *wire = dynamic_cast<WireWidget *>(widget);
		wire->color = findColor(wire->color);
		if (varyCheck->selected) {
			wire->color = varyColor(wire->color);
		}
	}

	void step() override {
		if (!stabilized) {
			wireCount = RACK_PLUGIN_UI_RACKWIDGET->wireContainer->children.size();
			stabilized = true;
		}
		int newSize = RACK_PLUGIN_UI_RACKWIDGET->wireContainer->children.size();
		if (newSize < wireCount) {
			wireCount = newSize;
			if (wireCount)
				lastWire = RACK_PLUGIN_UI_RACKWIDGET->wireContainer->children.back();
			else
				lastWire = NULL;
		}
		else if (newSize > wireCount) {
			std::list<Widget *>::reverse_iterator iterator = RACK_PLUGIN_UI_RACKWIDGET->wireContainer->children.rbegin();
			for (int i = 0; i < newSize - wireCount; i++) {
				colorWire(*iterator);
				++iterator;
			}
			wireCount = newSize;
			if (wireCount)
				lastWire = RACK_PLUGIN_UI_RACKWIDGET->wireContainer->children.back();
			else
				lastWire = NULL;
			highlightIsDirty = true;
		}
		highlightWires();
		ModuleWidget::step();
	}
	void highlightWires() {
		ModuleWidget *focusedModuleWidget = nullptr;
		if (highlight) {
			if (RACK_PLUGIN_UI_HOVERED_WIDGET) {
				focusedModuleWidget = dynamic_cast<ModuleWidget *>(RACK_PLUGIN_UI_HOVERED_WIDGET);
				if (!focusedModuleWidget)
					focusedModuleWidget = RACK_PLUGIN_UI_HOVERED_WIDGET->getAncestorOfType<ModuleWidget>();
			}
		}
		if (focusedModuleWidget != lastHover) {
			lastHover = focusedModuleWidget;
			highlightIsDirty = true;
		}
		if (highlightIsDirty) {
			highlightIsDirty = false;
			for (Widget *widget : RACK_PLUGIN_UI_RACKWIDGET->wireContainer->children) {
				WireWidget *wire = dynamic_cast<WireWidget *>(widget);
				if (focusedModuleWidget) {
					if (!wire->outputPort || !wire->inputPort) {
						wire->color = nvgTransRGBA(wire->color, 0xFF);
					}
					else if (wire->outputPort->getAncestorOfType<ModuleWidget>() == focusedModuleWidget) {
						wire->color = nvgTransRGBA(wire->color, 0xFF);
					}
					else if (wire->inputPort->getAncestorOfType<ModuleWidget>() == focusedModuleWidget) {
						wire->color = nvgTransRGBA(wire->color, 0xFF);
					}
					else {
						wire->color = nvgTransRGBAf(wire->color, highlightSlider->value);
					}
				}
				else {
					if (highlight == 2)
						wire->color = nvgTransRGBAf(wire->color, highlightSlider->value);
					else
						wire->color = nvgTransRGBA(wire->color, 0xFF);
				}
			}
		}
	} 

	void Colors() {
		colorWidget->visible = true;
		optionWidget->visible = false;
		editWidget->visible = false;
		deleteWidget->visible = false;
	}

	void Options() {
		colorWidget->visible = false;
		optionWidget->visible = true;
		editWidget->visible = false;
		deleteWidget->visible = false;
	}

	void Edit(WMWireButton *wmb) {
		colorWidget->visible = false;
		optionWidget->visible = false;
		editWidget->visible = true;
		deleteWidget->visible = false;
		editingColor = wmb;
		deleteButton->visible = (wmb != NULL);
		varyR->defaultValue = varyR->value = wmb?wmb->color.r:0.5;
		varyG->defaultValue = varyG->value = wmb?wmb->color.g:0.5;
		varyB->defaultValue = varyB->value = wmb?wmb->color.b:0.5;
	}

	void Delete() {
		colorWidget->visible = false;
		optionWidget->visible = false;
		editWidget->visible = false;
		deleteWidget->visible = true;
	}

	void SetHighlight(int status) {
		highlightOff->selected = (highlightOff->status == status);
		highlightLow->selected = (highlightLow->status == status);
		highlightOn->selected = (highlightOn->status == status);
		highlight = status;
		highlightIsDirty = true;
	}

	void SaveSettings() {
		json_t *settings = json_object();
		json_t *arr = json_array();
		auto __begin = std::begin(colorWidget->container->children);
		auto __end = std::end(colorWidget->container->children);
		for (++__begin; __begin != __end; ++__begin) {
			WMWireButton *wb = dynamic_cast<WMWireButton *>(*__begin);
			json_t *color = json_object();
			std::string s = colorToHexString(wb->color);	
			json_object_set_new(color, "color", json_string(s.c_str()));
			json_object_set_new(color, "selected", json_real(wb->wmc->selected));
			json_array_append_new(arr, color);
		}
		json_object_set_new(settings, "colors", arr);
		json_object_set_new(settings, "highlight", json_real(highlight));
		json_object_set_new(settings, "highlight_trans", json_real(highlightSlider->value));
		json_object_set_new(settings, "variation", json_real(varyCheck->selected));
		json_object_set_new(settings, "variationH", json_real(varyH->value));
		json_object_set_new(settings, "variationS", json_real(varyS->value));
		json_object_set_new(settings, "variationL", json_real(varyL->value));

		systemCreateDirectory(assetLocal("SubmarineUtility"));
		std::string settingsFilename = assetLocal("SubmarineUtility/WireManager.json");
		FILE *file = fopen(settingsFilename.c_str(), "w");
		if (file) {
			json_dumpf(settings, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
			fclose(file);
		}
		json_decref(settings);
	}
	void LoadSettings() {
		json_error_t error;
		FILE *file = fopen(assetLocal("SubmarineUtility/WireManager.json").c_str(), "r");
		if (!file) {
			SetDefaults();
			return;
		}
		json_t *rootJ = json_loadf(file, 0, &error);
		fclose(file);
		if (!rootJ) {
			std::string message = stringf("Submarine Utilities Wire Manager: JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
			warn(message.c_str());
			return;
		}
		json_t *arr = json_object_get(rootJ, "colors");
		if (arr) {
			int size = json_array_size(arr);
			for (int i = 0; i < size; i++) {
				json_t *j1 = json_array_get(arr, i);
				if (j1) {
					json_t *c1 = json_object_get(j1, "color");
					if (c1) {
						int selected = false;
						json_t *s1 = json_object_get(j1, "selected");	
						if (s1) {
							selected = json_number_value(s1);
						}
						AddColor(colorFromHexString(json_string_value(c1)), selected);
					}	
				}
			}
		}
		json_t *h1 = json_object_get(rootJ, "highlight");
		if (h1) {
			highlight = json_number_value(h1);
			SetHighlight(highlight);
		}
		json_t *t1 = json_object_get(rootJ, "highlight_trans");
		if (t1) {
			highlightSlider->value = json_number_value(t1);
		}
		json_t *v1 = json_object_get(rootJ, "variation");
		if (v1) {
			varyCheck->selected = json_number_value(v1);
		}
		v1 = json_object_get(rootJ, "variationH");
		if (v1) {
			varyH->value = json_number_value(v1);
		}
		v1 = json_object_get(rootJ, "variationS");
		if (v1) {
			varyS->value = json_number_value(v1);
		}
		v1 = json_object_get(rootJ, "variationL");
		if (v1) {
			varyL->value = json_number_value(v1);
		}
		json_decref(rootJ);
	}

	void Swap(float x) {
		unsigned int i = (unsigned int)x;
		if (i < 1) return;
		if (i > colorWidget->container->children.size() - 2) return;	
		auto vi = std::begin(colorWidget->container->children);
		std::advance(vi, i);
		WMWireButton *wb1 = dynamic_cast<WMWireButton *>(* vi++);
		WMWireButton *wb2 = dynamic_cast<WMWireButton *>(* vi);
		NVGcolor col = wb1->color;
		wb1->color = wb2->color;
		wb2->color = col;
		int sel = wb1->wmc->selected;
		wb1->wmc->selected = wb2->wmc->selected;
		wb2->wmc->selected = sel;
		SaveSettings();
	}

	void Reflow() {
		float pos = 0.0f;
		for (Widget *w : colorWidget->container->children) {
			w->box.pos.y = pos;
			pos += w->box.size.y;
		}
	}
};

void WMWireCheck::onAction(EventAction &e) {
	selected = !selected;
	wmw->SaveSettings();
}

void WMCheckAll::onAction(EventAction &e) {
	selected = !selected;
	auto __begin = std::begin(wmw->colorWidget->container->children);
	auto __end = std::end(wmw->colorWidget->container->children);
	for (++__begin; __begin != __end; ++__begin) {
		WMWireButton *wb = dynamic_cast<WMWireButton *>(*__begin);
		wb->wmc->selected = selected;
	}
	wmw->SaveSettings();
}

void WMSlider::onAction(EventAction &e) {
	wmw->SaveSettings();
}

// Icon onAction

void WMMinimizeIcon::onAction(EventAction &e) {
	wmw->Minimize(true);
}

void WMOptionIcon::onAction(EventAction &e) {
	wmw->Options();
}

void WMColorIcon::onAction(EventAction &e) {
	wmw->Colors();
}

void WMHighlightButton::onAction(EventAction &e) {
	wmw->SetHighlight(status);
	wmw->SaveSettings();
}

void WMWireEdit::onAction(EventAction &e) {
	wmb->wmc->wmw->Edit(wmb);
}

void WMWireUp::onAction(EventAction &e) {
	wmb->wmc->wmw->Swap(wmb->box.pos.y / 21 - 1);
}

void WMWireDown::onAction(EventAction &e) {
	wmb->wmc->wmw->Swap(wmb->box.pos.y / 21);
}

void WMEditWidget::draw(NVGcontext *vg) {
	NVGcolor color = nvgRGBAf(wmw->varyR->value, wmw->varyG->value, wmw->varyB->value, 1.0f);
	NVGcolor colorOutline = nvgLerpRGBA(color, nvgRGBf(0.0, 0.0, 0.0), 0.5);

	nvgBeginPath(vg);
	nvgMoveTo(vg, 12, 12);
	nvgQuadTo(vg, box.size.x / 2, 150, box.size.x - 12, 12);
	nvgStrokeColor(vg, colorOutline);
	nvgStrokeWidth(vg, 5);
	nvgStroke(vg);

	nvgStrokeColor(vg, color);
	nvgStrokeWidth(vg, 3);
	nvgStroke(vg);

	nvgBeginPath(vg);
	nvgCircle(vg, 12, 12, 9);
	nvgCircle(vg, box.size.x - 12, 12, 9);
	nvgFillColor(vg, color);
	nvgFill(vg);

	nvgStrokeWidth(vg, 1.0);
	nvgStrokeColor(vg, colorOutline);
	nvgStroke(vg);

	nvgBeginPath(vg);
	nvgCircle(vg, 12, 12, 5);
	nvgCircle(vg, box.size.x - 12, 12, 5);
	nvgFillColor(vg, nvgRGBf(0.0, 0.0, 0.0));
	nvgFill(vg);
	
	float l = wmw->varyR->box.pos.x + 5;
	float r = wmw->varyR->box.size.x - 10;

	NVGpaint grad = nvgLinearGradient(vg, l, 109.5, l + r, 109.5, nvgRGBf(0.0f, wmw->varyG->value, wmw->varyB->value), nvgRGBf(1.0f, wmw->varyG->value, wmw->varyB->value));
	nvgBeginPath(vg);
	nvgFillPaint(vg, grad);
	nvgRect(vg, l - 5, 109.5, r + 10, 10);
	nvgFill(vg);

	grad = nvgLinearGradient(vg, l, 149.5, l + r, 149.5, nvgRGBf(wmw->varyR->value, 0.0f, wmw->varyB->value), nvgRGBf(wmw->varyR->value, 1.0f, wmw->varyB->value));
	nvgBeginPath(vg);
	nvgFillPaint(vg, grad);
	nvgRect(vg, l - 5, 149.5, r + 10, 10);
	nvgFill(vg);

	grad = nvgLinearGradient(vg, l, 189.5, l + r, 189.5, nvgRGBf(wmw->varyR->value, wmw->varyG->value, 0.0f), nvgRGBf(wmw->varyR->value, wmw->varyG->value, 1.0f));
	nvgBeginPath(vg);
	nvgFillPaint(vg, grad);
	nvgRect(vg, l - 5, 189.5, r + 10, 10);
	nvgFill(vg);

	VirtualWidget::draw(vg);
}

void WMDeleteWidget::draw(NVGcontext *vg) {
	NVGcolor color = wmw->editingColor?wmw->editingColor->color:nvgRGB(0,0,0);
	NVGcolor colorOutline = nvgLerpRGBA(color, nvgRGBf(0.0, 0.0, 0.0), 0.5);

	nvgBeginPath(vg);
	nvgMoveTo(vg, 12, 12);
	nvgQuadTo(vg, box.size.x / 2, 150, box.size.x - 12, 12);
	nvgStrokeColor(vg, colorOutline);
	nvgStrokeWidth(vg, 5);
	nvgStroke(vg);

	nvgStrokeColor(vg, color);
	nvgStrokeWidth(vg, 3);
	nvgStroke(vg);

	nvgBeginPath(vg);
	nvgCircle(vg, 12, 12, 9);
	nvgCircle(vg, box.size.x - 12, 12, 9);
	nvgFillColor(vg, color);
	nvgFill(vg);

	nvgStrokeWidth(vg, 1.0);
	nvgStrokeColor(vg, colorOutline);
	nvgStroke(vg);

	nvgBeginPath(vg);
	nvgCircle(vg, 12, 12, 5);
	nvgCircle(vg, box.size.x - 12, 12, 5);
	nvgFillColor(vg, nvgRGBf(0.0, 0.0, 0.0));
	nvgFill(vg);

	VirtualWidget::draw(vg);
}

void WMSaveButton::onAction(EventAction &e) {
	if (wmw->editingColor) {
		wmw->editingColor->color = nvgRGBAf(wmw->varyR->value, wmw->varyG->value, wmw->varyB->value, 1.0f);
	}
	else {
		wmw->AddColor(nvgRGBAf(wmw->varyR->value, wmw->varyG->value, wmw->varyB->value, 1.0f), false);
	}
	wmw->SaveSettings();
	wmw->Colors();
}

void WMCancelButton::onAction(EventAction &e) {
	wmw->Colors();
}

void WMWireAdd::onAction(EventAction &e) {
	wmw->Edit(NULL);
}

void WMDeleteButton::onAction(EventAction &e) {
	if (!wmw->editingColor)
		return;
	wmw->Delete();
}

void WMOKButton::onAction(EventAction &e) {
	if (!wmw->editingColor)
		return;
	wmw->colorWidget->container->removeChild(wmw->editingColor);
	delete wmw->editingColor;
	wmw->Reflow();
	wmw->SaveSettings();
	wmw->Colors();
}

} // namespace rack_plugin_SubmarineUtility

using namespace rack_plugin_SubmarineUtility;

RACK_PLUGIN_MODEL_INIT(SubmarineUtility, WireManager) {
   Model *modelWireManager = Model::create<Module, WireManagerWidget>("Submarine (Utilities)", "WireManager", "Wire Manager", UTILITY_TAG);
   return modelWireManager;
}

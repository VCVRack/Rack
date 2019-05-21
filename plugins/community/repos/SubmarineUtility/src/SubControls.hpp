#include "SubmarineUtility.hpp"
#undef plugin
#include "global.hpp"
#include "global_pre.hpp"
#include "window.hpp"

namespace rack_plugin_SubmarineUtility {

namespace SubControls {

struct BackPanel : OpaqueWidget {
	void draw (NVGcontext *vg) override {
		nvgBeginPath(vg);
		nvgRect(vg, 0, 0, box.size.x, box.size.y);
		nvgFillColor(vg, nvgRGB(0, 0, 0));
		nvgFill(vg);
		OpaqueWidget::draw(vg);
	}
};

struct ButtonBase : Component {
	void onDragEnd(EventDragEnd &e) override {
		EventAction eAction;
		onAction(eAction);
	}
};

struct RadioButton : ButtonBase {
	std::string label;
	int selected = false;
	void draw (NVGcontext *vg) override {
		nvgStrokeWidth(vg, 1); 
		nvgFillColor(vg, nvgRGB(0xff, 0xff, 0xff));
		if (!label.empty()) {
			nvgFontFaceId(vg, rack::global_ui->window.gGuiFont->handle);
			nvgFontSize(vg, 13);
			nvgTextAlign(vg, NVG_ALIGN_MIDDLE);
			nvgText(vg, 21, box.size.y / 2, label.c_str(), NULL);
		}
		if (selected) {
			nvgBeginPath(vg);
			nvgCircle(vg, box.size.y / 2 + 1, box.size.y / 2, 5);
			nvgFill(vg);
		}
		nvgBeginPath(vg);
		nvgCircle(vg, box.size.y / 2 + 1, box.size.y / 2, 8);
		nvgStrokeColor(vg, nvgRGB(0xff, 0xff, 0xff));
		nvgStroke(vg);
		Component::draw(vg);
	}
};

struct CheckButton : ButtonBase {
	std::string label;
	int selected = false;
	void draw (NVGcontext *vg) override {
		nvgFillColor(vg, nvgRGB(0xff, 0xff, 0xff));
		if (!label.empty()) {
			nvgFontFaceId(vg, rack::global_ui->window.gGuiFont->handle);
			nvgFontSize(vg, 13);
			nvgTextAlign(vg, NVG_ALIGN_MIDDLE);
			nvgText(vg, 21, box.size.y / 2, label.c_str(), NULL);
		}
		nvgStrokeWidth(vg, 1);
		nvgStrokeColor(vg, nvgRGB(0xff, 0xff, 0xff));
		if (selected) {
			nvgBeginPath(vg);
			nvgMoveTo(vg, box.size.y / 2 - 4, box.size.y / 2 - 5);
			nvgLineTo(vg, box.size.y / 2 + 6, box.size.y / 2 + 5);
			nvgMoveTo(vg, box.size.y / 2 - 4, box.size.y / 2 + 5);
			nvgLineTo(vg, box.size.y / 2 + 6, box.size.y / 2 - 5);
			nvgStroke(vg);
		}
		nvgBeginPath(vg);
		nvgRect(vg, box.size.y / 2 - 7, box.size.y / 2 - 8, 16, 16);
		nvgStroke(vg);
		Component::draw(vg);
	}
};

struct ClickButton : ButtonBase {
	std::string label;
	void draw (NVGcontext *vg) override {
		nvgFillColor(vg, nvgRGB(0xff, 0xff, 0xff));
		if (!label.empty()) {
			nvgFontFaceId(vg, rack::global_ui->window.gGuiFont->handle);
			nvgFontSize(vg, 13);
			nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
			nvgText(vg, box.size.x / 2, box.size.y / 2, label.c_str(), NULL);
		}
		nvgBeginPath(vg);
		nvgRect(vg, 0.5, 0.5, box.size.x - 1, box.size.y - 1);
		nvgStrokeColor(vg, nvgRGB(0xff, 0xff, 0xff));
		nvgStrokeWidth(vg, 1);
		nvgStroke(vg);
		Component::draw(vg);
	}
};

struct Label : TransparentWidget {
	std::string label;
	void draw (NVGcontext *vg) override {
		nvgFillColor(vg, nvgRGB(0xff, 0xff, 0xff));
		if (!label.empty()) {
			nvgFontFaceId(vg, rack::global_ui->window.gGuiFont->handle);
			nvgFontSize(vg, 13);
			nvgTextAlign(vg, NVG_ALIGN_MIDDLE);
			nvgText(vg, 1, box.size.y / 2, label.c_str(), NULL);
		}
		TransparentWidget::draw(vg);
	}
};

struct Slider : Knob {
	int transparent = false;
	void draw(NVGcontext *vg) override {
		Vec minHandlePos;
		Vec maxHandlePos;
		float width;
		float height;
		if (box.size.x < box.size.y) {
			width = box.size.x;
			height = 10;
			minHandlePos = Vec(box.size.x / 2, height / 2);
			maxHandlePos = Vec(box.size.x / 2, box.size.y - height / 2);
		}
		else {
			width = 10;
			height = box.size.y;
			minHandlePos = Vec(width / 2, box.size.y / 2);
			maxHandlePos = Vec(box.size.x - width / 2, box.size.y / 2);
		}
		Vec pos = Vec(rescale(value, minValue, maxValue, minHandlePos.x, maxHandlePos.x), rescale(value, minValue, maxValue, minHandlePos.y, maxHandlePos.y));
		nvgFillColor(vg, nvgRGB(0, 0, 0));	
		nvgStrokeColor(vg, nvgRGB(0xff, 0xff, 0xff));
		nvgStrokeWidth(vg, 1);
		if (!transparent) {
			nvgBeginPath(vg);
			nvgMoveTo(vg, minHandlePos.x, minHandlePos.y);
			nvgLineTo(vg, maxHandlePos.x, maxHandlePos.y);
			nvgStroke(vg);
		}
		nvgBeginPath(vg);
		nvgRect(vg, pos.x - width / 2 + 0.5, pos.y - height / 2 + 0.5 , width - 1, height - 1);
		if (!transparent) nvgFill(vg);
		nvgStroke(vg);
	}
};

struct HSlider : Slider {
	void onDragMove(EventDragMove &e) override {
		e.mouseRel.y = -e.mouseRel.x;
		Knob::onDragMove(e);
	}
};

struct SubLogo : SVGWidget{};

struct ModuleDragHandle;
struct SizeableModuleWidget;

struct MaximizeButton : ButtonBase {
	SizeableModuleWidget *smw;
	MaximizeButton() {
		box.size.x = 15;
		box.size.y = 30;
	}
	void draw(NVGcontext *vg) override {
		nvgBeginPath(vg);
		nvgMoveTo(vg, 2, 4);
		nvgLineTo(vg, 13, 15);
		nvgLineTo(vg, 2, 26);
		nvgClosePath(vg);
		nvgFillColor(vg, nvgRGB(0x71, 0x9f, 0xcf));
		nvgFill(vg);
		Component::draw(vg);
	}
	void onAction(EventAction &e) override;
};

struct SizeableModuleWidget : ModuleWidget {
	float moduleWidth = 300.0f;
	float minimumWidth = 185.0f;
	unsigned int sizeable = true;
	unsigned int stabilized = false;
	ModuleDragHandle *handle;
	BackPanel *backPanel;
	SubLogo *minimizeLogo;
	SubLogo *maximizeLogo;
	MaximizeButton *maximizeButton;	
	std::shared_ptr<Font> font = Font::load(assetGlobal("res/fonts/DejaVuSans.ttf"));
	std::string moduleName;

	SizeableModuleWidget(Module *module); 
	void draw(NVGcontext *vg) override;
	void Resize();
	void Minimize(unsigned int minimize);
	void ShiftOthers(float delta);
	json_t *toJson() override;
	void fromJson(json_t *rootJ) override;

	virtual void onResize() { }; 
};

struct ModuleDragHandle : VirtualWidget {
	SizeableModuleWidget *smw;
	float dragX;
	Rect originalBox;
	ModuleDragHandle() {
		box.size = Vec(10, 30);
	}
	void onMouseDown(EventMouseDown &e) override {
		if (e.button == 0) {
			e.consumed = true;
			e.target = this;
		}
	}
	void draw(NVGcontext *vg) override;
	void onDragStart(EventDragStart &e) override;
	void onDragMove(EventDragMove &e) override;
};

} // SubControls

} // namespace rack_plugin_SubmarineUtility

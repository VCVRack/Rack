#include "app.hpp"
#include "gui.hpp"


namespace rack {


struct PanelBorder : TransparentWidget {
	void draw(NVGcontext *vg) override {
		NVGcolor borderColor = nvgRGBAf(0.5, 0.5, 0.5, 0.5);
		nvgBeginPath(vg);
		nvgRect(vg, 0.5, 0.5, box.size.x - 1.0, box.size.y - 1.0);
		nvgStrokeColor(vg, borderColor);
		nvgStrokeWidth(vg, 1.0);
		nvgStroke(vg);
	}
};


void SVGPanel::step() {
	if (nearf(gPixelRatio, 1.0)) {
		oversample = 2.0;
	}
	FramebufferWidget::step();
}

void SVGPanel::setBackground(std::shared_ptr<SVG> svg) {
	clearChildren();

	SVGWidget *sw = new SVGWidget();
	sw->wrap();
	sw->svg = svg;
	addChild(sw);

	PanelBorder *pb = new PanelBorder();
	pb->box.size = box.size;
	addChild(pb);
}


} // namespace rack

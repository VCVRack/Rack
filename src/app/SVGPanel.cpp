#include "app.hpp"


namespace rack {


struct PanelBorder : TransparentWidget {
	void draw(NVGcontext *vg) {
		nvgBeginPath(vg);
		nvgRect(vg, 0.0, 0.0, box.size.x, box.size.y);

		NVGcolor borderColor = nvgRGBAf(0.5, 0.5, 0.5, 0.5);

		nvgBeginPath(vg);
		nvgRect(vg, 0.5, 0.5, box.size.x - 1.0, box.size.y - 1.0);
		nvgStrokeColor(vg, borderColor);
		nvgStrokeWidth(vg, 1.0);
		nvgStroke(vg);
	}
};


void SVGPanel::setBackground(std::shared_ptr<SVG> svg) {
	SVGWidget *sw = new SVGWidget();
	sw->wrap();
	sw->svg = svg;
	addChild(sw);

	PanelBorder *pb = new PanelBorder();
	pb->box.size = box.size;
	addChild(pb);
}


} // namespace rack

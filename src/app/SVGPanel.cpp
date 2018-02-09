#include "app.hpp"
#include "window.hpp"


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
	if (isNear(gPixelRatio, 1.0)) {
		// Small details draw poorly at low DPI, so oversample when drawing to the framebuffer
		oversample = 2.0;
	}
	FramebufferWidget::step();
}

void SVGPanel::setBackground(std::shared_ptr<SVG> svg) {
	SVGWidget *sw = new SVGWidget();
	sw->setSVG(svg);
	addChild(sw);

	// Set size
	box.size = sw->box.size.div(RACK_GRID_SIZE).round().mult(RACK_GRID_SIZE);

	PanelBorder *pb = new PanelBorder();
	pb->box.size = box.size;
	addChild(pb);
}


} // namespace rack

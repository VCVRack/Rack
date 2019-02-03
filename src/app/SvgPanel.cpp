#include "app/SvgPanel.hpp"


namespace rack {
namespace app {


void PanelBorder::draw(const widget::DrawContext &ctx) {
	NVGcolor borderColor = nvgRGBAf(0.5, 0.5, 0.5, 0.5);
	nvgBeginPath(ctx.vg);
	nvgRect(ctx.vg, 0.5, 0.5, box.size.x - 1.0, box.size.y - 1.0);
	nvgStrokeColor(ctx.vg, borderColor);
	nvgStrokeWidth(ctx.vg, 1.0);
	nvgStroke(ctx.vg);
}


void SvgPanel::step() {
	if (math::isNear(APP->window->pixelRatio, 1.0)) {
		// Small details draw poorly at low DPI, so oversample when drawing to the framebuffer
		oversample = 2.0;
	}
	widget::FramebufferWidget::step();
}

void SvgPanel::setBackground(std::shared_ptr<Svg> svg) {
	widget::SvgWidget *sw = new widget::SvgWidget;
	sw->setSvg(svg);
	addChild(sw);

	// Set size
	box.size = sw->box.size.div(RACK_GRID_SIZE).round().mult(RACK_GRID_SIZE);

	PanelBorder *pb = new PanelBorder;
	pb->box.size = box.size;
	addChild(pb);
}


} // namespace app
} // namespace rack

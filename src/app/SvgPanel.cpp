#include <app/SvgPanel.hpp>
#include <settings.hpp>


namespace rack {
namespace app {


void PanelBorder::draw(const DrawArgs& args) {
	NVGcolor borderColor = nvgRGBAf(0.5, 0.5, 0.5, 0.5);
	nvgBeginPath(args.vg);
	nvgRect(args.vg, 0.5, 0.5, box.size.x - 1.0, box.size.y - 1.0);
	nvgStrokeColor(args.vg, borderColor);
	nvgStrokeWidth(args.vg, 1.0);
	nvgStroke(args.vg);
}


SvgPanel::SvgPanel() {
	fb = new widget::FramebufferWidget;
	addChild(fb);

	sw = new widget::SvgWidget;
	fb->addChild(sw);

	panelBorder = new PanelBorder;
	fb->addChild(panelBorder);
}


void SvgPanel::step() {
	if (APP->window->pixelRatio < 2.0) {
		// Small details draw poorly at low DPI, so oversample when drawing to the framebuffer
		fb->oversample = 2.0;
	}
	else {
		fb->oversample = 1.0;
	}

	Widget::step();
}

void SvgPanel::setBackground(std::shared_ptr<window::Svg> svg) {
	if (svg == this->svg)
		return;
	this->svg = svg;

	sw->setSvg(svg);
	fb->setDirty();

	// Round framebuffer size to nearest grid
	fb->box.size = sw->box.size.div(RACK_GRID_SIZE).round().mult(RACK_GRID_SIZE);
	panelBorder->box.size = fb->box.size;
	box.size = fb->box.size;
}


void ThemedSvgPanel::step() {
	SvgPanel::setBackground(settings::preferDarkPanels ? darkSvg : lightSvg);
	SvgPanel::step();
}


void ThemedSvgPanel::setBackground(std::shared_ptr<window::Svg> lightSvg, std::shared_ptr<window::Svg> darkSvg) {
	this->lightSvg = lightSvg;
	this->darkSvg = darkSvg;
	SvgPanel::setBackground(settings::preferDarkPanels ? darkSvg : lightSvg);
}


} // namespace app
} // namespace rack

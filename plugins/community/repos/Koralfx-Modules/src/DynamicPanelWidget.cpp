#include "KoralfxWidgets.hpp"

void PanelBorderWidget::draw(NVGcontext *vg) {
    NVGcolor borderColor = nvgRGBAf(0.3, 0.3, 0.3, 0.3);
    nvgBeginPath(vg);
    nvgRect(vg, 0.5, 0.5, box.size.x - 1.0, box.size.y - 1.0);
    nvgStrokeColor(vg, borderColor);
    nvgStrokeWidth(vg, 1.0);
    nvgStroke(vg);
}

DynamicPanelWidget::DynamicPanelWidget() {
    mode = nullptr;
    oldMode = -1;
    visiblePanel = new SVGWidget();
    addChild(visiblePanel);
    border = new PanelBorderWidget();
    addChild(border);
}

void DynamicPanelWidget::addPanel(std::shared_ptr<SVG> svg) {
    panels.push_back(svg);
    if(!visiblePanel->svg) {
        visiblePanel->setSVG(svg);
        box.size = visiblePanel->box.size.div(RACK_GRID_SIZE).round().mult(RACK_GRID_SIZE);
        border->box.size = box.size;
    }
}

void DynamicPanelWidget::step() {
    oversample = 2.f;
    if(mode != nullptr && *mode != oldMode) {
        visiblePanel->setSVG(panels[*mode]);
        oldMode = *mode;
        dirty = true;
    }
}
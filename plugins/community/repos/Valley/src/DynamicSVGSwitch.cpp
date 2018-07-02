#include "ValleyWidgets.hpp"

DynamicSwitchWidget::DynamicSwitchWidget() {
    visibility = nullptr;
    viewMode = ACTIVE_HIGH_VIEW;
    sw = new SVGWidget();
    addChild(sw);
}

void DynamicSwitchWidget::addFrame(std::shared_ptr<SVG> svg) {
    frames.push_back(svg);
    // If this is our first frame, automatically set SVG and size
    if (!sw->svg) {
        sw->setSVG(svg);
        box.size = sw->box.size;
    }
}

void DynamicSwitchWidget::step() {
    if(visibility != nullptr) {
        if(*visibility) {
            visible = true;
        }
        else {
            visible = false;
        }
        if(viewMode == ACTIVE_LOW_VIEW) {
            visible = !visible;
        }
    }
    else {
        visible = true;
    }
    FramebufferWidget::step();
}

void DynamicSwitchWidget::onChange(EventChange &e) {
    assert(frames.size() > 0);
    float valueScaled = rescale(value, minValue, maxValue, 0, frames.size() - 1);
    int index = clamp((int) roundf(valueScaled), 0, frames.size() - 1);
    sw->setSVG(frames[index]);
    dirty = true;
    ParamWidget::onChange(e);
}

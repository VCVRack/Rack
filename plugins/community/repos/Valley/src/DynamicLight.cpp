#include "ValleyWidgets.hpp"

void DynamicModuleLightWidget::step() {
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

    assert(module);
    assert(module->lights.size() >= firstLightId + baseColors.size());
    std::vector<float> values(baseColors.size());

    for (size_t i = 0; i < baseColors.size(); i++) {
        float value = module->lights[firstLightId + i].getBrightness();
        value = clamp(value, 0.0, 1.0);
        values[i] = value;
    }
    setValues(values);
}

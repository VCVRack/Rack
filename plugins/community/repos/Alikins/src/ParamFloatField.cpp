#include "rack.hpp"
#include "ui.hpp"
#include "ParamFloatField.hpp"

ParamFloatField::ParamFloatField(Module *_module)
{
    module = _module;
}

void ParamFloatField::setValue(float value) {
    this->hovered_value = value;
    // this->module->param_value = value;
    EventChange e;
    onChange(e);
}

void ParamFloatField::onChange(EventChange &e) {
    std::string new_text = stringf("%#.4g", hovered_value);
    setText(new_text);
}

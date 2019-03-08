
using namespace rack;
#include "rack.hpp"
// #include "ui.hpp"

namespace rack_plugin_Alikins {

// TODO/FIXME: This is more or less adhoc TextField mixed with QuantityWidget
//             just inherit from both?
struct ParamFloatField : TextField
{
    Module *module;
    float hovered_value;

    ParamFloatField(Module *module);

    void setValue(float value);
    void onChange(EventChange &e) override;

};

} // namespace rack_plugin_Alikins

using namespace rack_plugin_Alikins;

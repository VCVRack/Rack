#include <iomanip>
#include <cmath>
#include "common.hpp"

namespace rack_plugin_RJModules {

std::stringstream format4display(float value){
    std::stringstream to_display;
    std::stringstream to_display2;

    float value_rounded = round(abs(10.0f * value))/10.0f;

    if (value_rounded < 1) {
        to_display << " " << std::setprecision(1) << std::setw(1) << std::fixed << value_rounded;
    }
    else if (value_rounded < 10) {
        to_display << " " << std::setprecision(2) << std::setw(3) << std::fixed << value_rounded;

    } else {
        to_display << std::setprecision(3) << std::setw(3) << std::fixed << value_rounded;
    }
    to_display2 << to_display.str().substr(0, 4);
    return to_display2;
}

} // namespace rack_plugin_RJModules


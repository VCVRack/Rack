#pragma once

#include "MicMusic.hpp"

namespace rack_plugin_MicMusic {

class Distortion : public Module {
public:
    enum class Params {
        HIGH,
        LOW,
        HIGH_CV,
        LOW_CV,
        COUNT
    };

    enum class Inputs {
        HIGH,
        LOW,
        SIGNAL,
        COUNT
    };

    enum class Lights {
        COUNT
    };

    enum class Outputs {
        SIGNAL,
        COUNT
    };
    
    Distortion();

    void step() override;
};

} // namespace rack_plugin_MicMusic

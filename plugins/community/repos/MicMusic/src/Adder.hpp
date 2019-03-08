#pragma once

#include "MicMusic.hpp"

namespace rack_plugin_MicMusic {

class Adder : public Module {
public:
    enum class Params {
        A_AMP,
        B_AMP,
        C_AMP,
        D_AMP,
        E_AMP,
        F_AMP,
        G_AMP,
        A_SIGN,
        B_SIGN,
        C_SIGN,
        D_SIGN,
        E_SIGN,
        F_SIGN,
        G_SIGN,
        A_MUTE,
        B_MUTE,
        C_MUTE,
        D_MUTE,
        E_MUTE,
        F_MUTE,
        G_MUTE,
        MUTE,
        COUNT
    };

    enum class Inputs {
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        COUNT
    };

    enum class Lights {
        COUNT
    };

    enum class Outputs {
        SIGNAL,
        COUNT
    };
    
    Adder();

    void step() override;
};

} // namespace rack_plugin_MicMusic

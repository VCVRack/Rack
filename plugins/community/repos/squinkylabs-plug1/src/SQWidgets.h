#pragma once

#include "rack.hpp"
#include "WidgetComposite.h"

#include <functional>


/**
 * Like Trimpot, but with blue stripe
 */
struct BlueTrimmer : SVGKnob {
	BlueTrimmer() {
      //  printf("ctrol of blue trimmer\n"); fflush(stdout);
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/BlueTrimmer.svg")));
	}
};

/**
 * Like Rogan1PSBlue, but smaller.
 */
struct Blue30Knob : SVGKnob {
	Blue30Knob() {    
		minAngle = -0.83*M_PI;
		maxAngle = 0.83*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/Blue30.svg")));
	}
};

struct Blue30SnapKnob : Blue30Knob {
	Blue30SnapKnob() {
		snap = true;
		smooth = false;
	}
};

struct NKKSmall : SVGSwitch, ToggleSwitch {
	NKKSmall() {
		addFrame(SVG::load(assetPlugin(plugin, "res/NKKSmall_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/NKKSmall_1.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/NKKSmall_2.svg")));
	}
};

struct BlueToggle : public SVGSwitch, ToggleSwitch {
    BlueToggle() {
        addFrame(SVG::load(assetPlugin(plugin, "res/BluePush_1.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/BluePush_0.svg")));
        #if 0
        setSVGs(
            SVG::load(assetPlugin(plugin, "res/BluePush_0.svg")),
            SVG::load(assetPlugin(plugin, "res/BluePush_1.svg"))
        );
        #endif
    }
};

/**
 * A very basic momentary push button.
 */
struct SQPush : SVGButton
{
    SQPush()
    {
        setSVGs(
            SVG::load(assetPlugin(plugin, "res/BluePush_0.svg")),
            SVG::load(assetPlugin(plugin, "res/BluePush_1.svg"))
        );
    }
    void center(Vec& pos)
    {
        this->box.pos = pos.minus(this->box.size.div(2));
    }

    void onDragEnd(EventDragEnd &e) override
    {
        SVGButton::onDragEnd(e);
        if (clickHandler) {
            clickHandler();
        }
    }

    /**
     * User of button passes in a callback lamba here
     */
    void onClick(std::function<void(void)> callback)
    {
        clickHandler = callback;
    }

    std::function<void(void)> clickHandler;
};

#pragma once

#include "rack.hpp"
#include "WidgetComposite.h"
#include "SqHelper.h"
#include "SqUI.h"
#include <functional>


/**
 * Like Trimpot, but with blue stripe
 */
struct BlueTrimmer : SVGKnob
{
    BlueTrimmer()
    {
        minAngle = -0.75*M_PI;
        maxAngle = 0.75*M_PI;
        setSVG(SVG::load(SqHelper::assetPlugin(plugin, "res/BlueTrimmer.svg")));
    }
};

/**
 * Like Rogan1PSBlue, but smaller.
 */
struct Blue30Knob : SVGKnob
{
    Blue30Knob()
    {
        minAngle = -0.83*M_PI;
        maxAngle = 0.83*M_PI;
        setSVG(SVG::load(SqHelper::assetPlugin(plugin, "res/Blue30.svg")));
    }
};

struct Blue30SnapKnob : Blue30Knob
{
    Blue30SnapKnob()
    {
        // TODO: snap for V1
#ifndef __V1
        snap = true;
        smooth = false;
#endif
    }
};


struct NKKSmall : SVGSwitch
#ifndef __V1
, ToggleSwitch
#endif
{
    NKKSmall()
    {
        addFrame(SVG::load(SqHelper::assetPlugin(plugin, "res/NKKSmall_0.svg")));
        addFrame(SVG::load(SqHelper::assetPlugin(plugin, "res/NKKSmall_1.svg")));
        addFrame(SVG::load(SqHelper::assetPlugin(plugin, "res/NKKSmall_2.svg")));
    }
};

struct BlueToggle : public SVGSwitch
#ifndef __V1
, ToggleSwitch
#endif
{
    BlueToggle()
    {
        addFrame(SVG::load(SqHelper::assetPlugin(plugin, "res/BluePush_1.svg")));
        addFrame(SVG::load(SqHelper::assetPlugin(plugin, "res/BluePush_0.svg")));
    }
};

/**
 * A very basic momentary push button.
 */
struct SQPush : SVGButton
{
    SQPush()
    {
#ifdef __V1
        addFrame(SVG::load(SqHelper::assetPlugin(plugin, "res/BluePush_0.svg")));
        addFrame(SVG::load(SqHelper::assetPlugin(plugin, "res/BluePush_1.svg")));
#else
        setSVGs(
            SVG::load(SqHelper::assetPlugin(plugin, "res/BluePush_0.svg")),
            SVG::load(SqHelper::assetPlugin(plugin, "res/BluePush_1.svg"))
        );
#endif
    }

    SQPush(const char* upSVG, const char* dnSVG)
    {
#ifdef __V1
        addFrame(SVG::load(SqHelper::assetPlugin(plugin, upSVG)));
        addFrame(SVG::load(SqHelper::assetPlugin(plugin, dnSVG)));
#else
        setSVGs(
            SVG::load(SqHelper::assetPlugin(plugin, upSVG)),
            SVG::load(SqHelper::assetPlugin(plugin, dnSVG))
        );
#endif
    }
    void center(Vec& pos)
    {
        this->box.pos = pos.minus(this->box.size.div(2));
    }
    // TODO: we just need to port
#ifndef __V1
    void onDragEnd(EventDragEnd &e) override
    {
        SVGButton::onDragEnd(e);
        if (clickHandler) {
            clickHandler();
        }
    }
#endif

    /**
     * User of button passes in a callback lamba here
     */
    void onClick(std::function<void(void)> callback)
    {
        clickHandler = callback;
    }

    std::function<void(void)> clickHandler;
};



/**************************************************************************
 **
 ** SQPanelItem
 ** a menu item made for expaning panels.
 **
 ************************************************************************/

using SQStatusCallback = std::function<bool()>;
using SQActionCAllback = std::function<void()>;

struct SQPanelItem : MenuItem
{

    SQPanelItem(SQStatusCallback, SQActionCAllback);

    void onAction(sq::EventAction &e) override
    {
        actionCallback();
    }

    void step() override
    {
        const bool b = statusCallback();
        rightText = CHECKMARK(b);
    }

    SQStatusCallback statusCallback;
    SQActionCAllback actionCallback;
};

inline SQPanelItem::SQPanelItem(SQStatusCallback s, SQActionCAllback a) :
    statusCallback(s),
    actionCallback(a)
{
    text = "Expanded Panel";
}

/**
 * A Widget that holds values and will serialize,
 * But doesn't interacts with mouse.
 */
class NullWidget : public ParamWidget
{
public:

    NullWidget() : ParamWidget()
    {
        // Make sure we have no dimensions to fool 
        // hit testing in VCV.
        box.pos.x = 0;
        box.pos.y = 0;
        box.size.x = 0;
        box.size.y = 0;
    }

    // TODO: port this
#if 0
    // Swallow mouse commands
    void onMouseDown(EventMouseDown &e) override
    {
        e.consumed = false;
    }
#endif

};

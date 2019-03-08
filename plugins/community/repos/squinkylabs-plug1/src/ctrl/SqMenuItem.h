#pragma once

#include "rack.hpp"
#include <functional>
#include "SqHelper.h"
#include "SqUI.h"

/**
 * This menu item takes generic lambdas,
 * so can be used for anything
 **/
struct SqMenuItem : rack::MenuItem
{

   // void onAction(rack::EventAction &e) override
    void onAction(sq::EventAction &e) override
    {
        _onActionFn();
    }


    void step() override
    {
        rightText = CHECKMARK(_isCheckedFn());
    }

    SqMenuItem(std::function<bool()> isCheckedFn,
        std::function<void()> clickFn) :
        _isCheckedFn(isCheckedFn),
        _onActionFn(clickFn)
    {
    }

private:
    std::function<bool()> _isCheckedFn;
    std::function<void()> _onActionFn;
};


struct ManualMenuItem : SqMenuItem
{
    ManualMenuItem(const char* url) : SqMenuItem(
        []() { return false; },
        [url]() { SqHelper::openBrowser(url); })
    {
        this->text = "Manual";
    }

};


struct  SqMenuItem_BooleanParam : rack::MenuItem
{
    SqMenuItem_BooleanParam(rack::ParamWidget* widget) :
        widget(widget)
    {
    }

    void onAction(sq::EventAction &e) override
    {
        const float newValue = isOn() ? 0 : 1;
#ifdef __V1
       //widget->dirtyValue = newValue;
       if (widget->paramQuantity) {
            widget->paramQuantity->setValue(newValue);
       }
#else
        widget->value = newValue;
#endif
        sq::EventChange ec;
        widget->onChange(ec);
#ifdef __V1
        e.consume(this);
#else
        e.consumed = true;
#endif
    }


    void step() override
    {
        rightText = CHECKMARK(isOn());
    }

private:
    bool isOn()
    {
#ifdef __V1
        //return false;
        bool ret = false;
        if (widget->paramQuantity) {
            ret = widget->paramQuantity->getValue() > .5f;
        }
#else
        bool ret = widget->value > .5f;
#endif
        return ret;
    }
    rack::ParamWidget* const widget;
};
#pragma once

#include "app.hpp"
#include "IComposite.h"
/** Wrap up all the .6/1.0 dependencies here
 */
#ifdef __V1
class SqHelper
{
public:

    static void openBrowser(const char* url)
    {
        system::openBrowser(url);
    }
    static std::string assetPlugin(Plugin *plugin, const std::string& filename)
    {
        return asset::plugin(plugin, filename);
    } 
    static float engineGetSampleRate()
    {
        return APP->engine->getSampleRate();
    }
      static float engineGetSampleTime()
    {
        return APP->engine->getSampleTime();
    }

    template <typename T>
    static T* createParam(std::shared_ptr<IComposite> dummy, const Vec& pos, Module* module, int paramId )
    {
        return rack::createParam<T>(pos, module, paramId);
    }

    template <typename T>
    static T* createParamCentered(std::shared_ptr<IComposite> dummy, const Vec& pos, Module* module, int paramId )
    {
        return rack::createParamCentered<T>(pos, module, paramId);
    }

    static const NVGcolor COLOR_WHITE;
    static const NVGcolor COLOR_BLACK;

    static void setupParams(std::shared_ptr<IComposite> comp, Module* module)
    {
        const int n = comp->getNumParams();
        for (int i=0; i<n; ++i) {
            auto param = comp->getParam(i);
            module->params[i].config(param.min, param.max, param.def, param.name);
        }
    }

    static float getValue(ParamWidget* widget) {
        return (widget->paramQuantity) ?
            widget->paramQuantity->getValue() :
            0;
    }

    static void setValue(ParamWidget* widget, float v) {
        if (widget->paramQuantity) {
            widget->paramQuantity->setValue(v);
        }
    }
};

#else


class SqHelper
{
public:
#ifdef USE_VST2
    static std::string assetPlugin(const char *_plugin, const std::string& filename)
#else
    static std::string assetPlugin(Plugin *_plugin, const std::string& filename)
#endif
    {
       return rack::assetPlugin(_plugin, filename.c_str());
    } 
    static float engineGetSampleRate()
    {
        return rack::engineGetSampleRate();
    }

    static float engineGetSampleTime()
    {
        return rack::engineGetSampleTime();
    }
    static void openBrowser(const char* url)
    {
        rack::systemOpenBrowser(url);
    }

   static const NVGcolor COLOR_WHITE;
   static const NVGcolor COLOR_BLACK;

   template <typename T>
   static T* createParam(std::shared_ptr<IComposite> composite, const Vec& pos, Module* module, int paramId )
   {
       const auto data = composite->getParam(paramId);
       assert(data.min < data.max);
       assert(data.def >= data.min);
       assert(data.def <= data.max);
       return rack::createParam<T>(
           pos,
           module, 
           paramId,
           data.min, data.max, data.def
       );
    }

    template <typename T>
    static T* createParamCentered(std::shared_ptr<IComposite> composite, const Vec& pos, Module* module, int paramId )
    {
        const auto data = composite->getParam(paramId);
        assert(data.min < data.max);
        assert(data.def >= data.min);
        assert(data.def <= data.max);
        return rack::createParamCentered<T>(
            pos,
            module, 
            paramId,
            data.min, data.max, data.def
        );
    }

    static float getValue(ParamWidget* widget) {
        return widget->value;
    }

    static void setValue(ParamWidget* widget, float v) {
        widget->setValue(v);
    }
};
#endif

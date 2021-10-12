#pragma once

/*
The following headers are the "public" API of Rack.

Directly including Rack headers other than rack.hpp in your plugin is unsupported/unstable, since filenames and locations of symbols may change in any Rack version.
*/

#include <common.hpp>
#include <math.hpp>
#include <string.hpp>
#include <system.hpp>
#include <random.hpp>
#include <network.hpp>
#include <asset.hpp>
#include <window/Window.hpp>
#include <context.hpp>
#include <audio.hpp>
#include <midi.hpp>
#include <settings.hpp>
#include <helpers.hpp>
#include <componentlibrary.hpp>

#include <widget/TransparentWidget.hpp>
#include <widget/OpenGlWidget.hpp>
#include <widget/OpaqueWidget.hpp>
#include <widget/FramebufferWidget.hpp>
#include <widget/TransformWidget.hpp>
#include <widget/event.hpp>
#include <widget/ZoomWidget.hpp>
#include <widget/SvgWidget.hpp>
#include <widget/Widget.hpp>

#include <ui/Tooltip.hpp>
#include <ui/MenuLabel.hpp>
#include <ui/MenuEntry.hpp>
#include <ui/List.hpp>
#include <ui/TooltipOverlay.hpp>
#include <ui/Slider.hpp>
#include <ui/Scrollbar.hpp>
#include <ui/ProgressBar.hpp>
#include <ui/MenuSeparator.hpp>
#include <ui/MenuOverlay.hpp>
#include <ui/Label.hpp>
#include <ui/TextField.hpp>
#include <ui/SequentialLayout.hpp>
#include <ui/MenuItem.hpp>
#include <ui/Button.hpp>
#include <ui/ChoiceButton.hpp>
#include <ui/OptionButton.hpp>
#include <ui/RadioButton.hpp>
#include <ui/Menu.hpp>
#include <ui/ScrollWidget.hpp>
#include <ui/PasswordField.hpp>

#include <app/SliderKnob.hpp>
#include <app/MultiLightWidget.hpp>
#include <app/MidiWidget.hpp>
#include <app/CircularShadow.hpp>
#include <app/AudioWidget.hpp>
#include <app/LedDisplay.hpp>
#include <app/ModuleLightWidget.hpp>
#include <app/LightWidget.hpp>
#include <app/RailWidget.hpp>
#include <app/PortWidget.hpp>
#include <app/CableWidget.hpp>
#include <app/Switch.hpp>
#include <app/RackScrollWidget.hpp>
#include <app/Knob.hpp>
#include <app/Scene.hpp>
#include <app/RackWidget.hpp>
#include <app/ParamWidget.hpp>
#include <app/SvgKnob.hpp>
#include <app/SvgPanel.hpp>
#include <app/SvgPort.hpp>
#include <app/SvgSwitch.hpp>
#include <app/SvgScrew.hpp>
#include <app/ModuleWidget.hpp>
#include <app/SvgSlider.hpp>
#include <app/SvgButton.hpp>

#include <engine/Param.hpp>
#include <engine/ParamHandle.hpp>
#include <engine/LightInfo.hpp>
#include <engine/PortInfo.hpp>
#include <engine/Light.hpp>
#include <engine/Cable.hpp>
#include <engine/Port.hpp>
#include <engine/ParamQuantity.hpp>
#include <engine/Module.hpp>
#include <engine/Engine.hpp>

#include <plugin/Plugin.hpp>
#include <plugin/Model.hpp>
#include <plugin/callbacks.hpp>

#include <dsp/window.hpp>
#include <dsp/ode.hpp>
#include <dsp/minblep.hpp>
#include <dsp/fft.hpp>
#include <dsp/ringbuffer.hpp>
#include <dsp/resampler.hpp>
#include <dsp/fir.hpp>
#include <dsp/approx.hpp>
#include <dsp/midi.hpp>
#include <dsp/vumeter.hpp>
#include <dsp/filter.hpp>
#include <dsp/digital.hpp>
#include <dsp/common.hpp>

#include <simd/Vector.hpp>
#include <simd/functions.hpp>


#undef INTERNAL
#if defined ARCH_WIN
	#define INTERNAL __attribute__((error("Using internal Rack function or symbol")))
#else
	#define INTERNAL __attribute__((visibility("hidden"))) __attribute__((error("Using internal Rack function or symbol")))
#endif


namespace rack {


// Import some namespaces for convenience
using namespace logger;
using namespace math;
using namespace window;
using namespace widget;
using namespace ui;
using namespace app;
using plugin::Plugin;
using plugin::Model;
using namespace engine;
using namespace componentlibrary;


} // namespace rack

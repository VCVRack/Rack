#pragma once

// Include most Rack headers for convenience
#include "common.hpp"
#include "math.hpp"
#include "string.hpp"
#include "system.hpp"
#include "random.hpp"
#include "network.hpp"
#include "asset.hpp"
#include "window.hpp"
#include "app.hpp"
#include "midi.hpp"
#include "helpers.hpp"
#include "componentlibrary.hpp"

#include "widget/Widget.hpp"
#include "widget/TransparentWidget.hpp"
#include "widget/OpaqueWidget.hpp"
#include "widget/TransformWidget.hpp"
#include "widget/ZoomWidget.hpp"
#include "widget/SvgWidget.hpp"
#include "widget/FramebufferWidget.hpp"
#include "widget/OpenGlWidget.hpp"

#include "ui/SequentialLayout.hpp"
#include "ui/Label.hpp"
#include "ui/List.hpp"
#include "ui/MenuOverlay.hpp"
#include "ui/Tooltip.hpp"
#include "ui/TextField.hpp"
#include "ui/PasswordField.hpp"
#include "ui/ScrollWidget.hpp"
#include "ui/Slider.hpp"
#include "ui/Menu.hpp"
#include "ui/MenuEntry.hpp"
#include "ui/MenuSeparator.hpp"
#include "ui/MenuLabel.hpp"
#include "ui/MenuItem.hpp"
#include "ui/Button.hpp"
#include "ui/IconButton.hpp"
#include "ui/ChoiceButton.hpp"
#include "ui/RadioButton.hpp"
#include "ui/ProgressBar.hpp"

#include "app/AudioWidget.hpp"
#include "app/CircularShadow.hpp"
#include "app/Knob.hpp"
#include "app/LedDisplay.hpp"
#include "app/LightWidget.hpp"
#include "app/MidiWidget.hpp"
#include "app/ModuleLightWidget.hpp"
#include "app/ModuleWidget.hpp"
#include "app/MultiLightWidget.hpp"
#include "app/ParamWidget.hpp"
#include "app/PortWidget.hpp"
#include "app/RackRail.hpp"
#include "app/Scene.hpp"
#include "app/RackScrollWidget.hpp"
#include "app/RackWidget.hpp"
#include "app/SvgButton.hpp"
#include "app/SvgKnob.hpp"
#include "app/SvgPanel.hpp"
#include "app/SvgPort.hpp"
#include "app/SvgScrew.hpp"
#include "app/SvgSlider.hpp"
#include "app/SvgSwitch.hpp"
#include "app/Toolbar.hpp"
#include "app/CableWidget.hpp"

#include "engine/Engine.hpp"
#include "engine/Param.hpp"
#include "engine/Port.hpp"
#include "engine/Module.hpp"
#include "engine/Param.hpp"
#include "engine/Cable.hpp"

#include "plugin/Plugin.hpp"
#include "plugin/Model.hpp"
#include "plugin/callbacks.hpp"

#include "dsp/common.hpp"
#include "dsp/digital.hpp"
#include "dsp/fft.hpp"
#include "dsp/filter.hpp"
#include "dsp/fir.hpp"
#include "dsp/frame.hpp"
#include "dsp/minblep.hpp"
#include "dsp/ode.hpp"
#include "dsp/resampler.hpp"
#include "dsp/ringbuffer.hpp"
#include "dsp/vumeter.hpp"
#include "dsp/window.hpp"

#include "simd/vector.hpp"
#include "simd/functions.hpp"


namespace rack {


/** Define this macro before including this header to prevent common namespaces from being included in the main `rack::` namespace. */
#ifndef RACK_FLATTEN_NAMESPACES
// Import some namespaces for convenience
using namespace math;
using namespace widget;
using namespace ui;
using namespace app;
using plugin::Plugin;
using plugin::Model;
using namespace engine;
using namespace componentlibrary;
#endif


} // namespace rack

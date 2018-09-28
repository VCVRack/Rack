#pragma once

#include "widgets/Widget.hpp"
#include "widgets/EventWidget.hpp"
#include "widgets/TransparentWidget.hpp"
#include "widgets/OpaqueWidget.hpp"
#include "widgets/TransformWidget.hpp"
#include "widgets/ZoomWidget.hpp"
#include "widgets/SVGWidget.hpp"
#include "widgets/FramebufferWidget.hpp"
#include "widgets/QuantityWidget.hpp"


#define CHECKMARK_STRING "✔"
#define CHECKMARK(_cond) ((_cond) ? CHECKMARK_STRING : "")

#pragma once
#include "app/common.hpp"
#include "widgets/TransparentWidget.hpp"
#include "widgets/FramebufferWidget.hpp"
#include "widgets/SVGWidget.hpp"
#include "app.hpp"


namespace rack {


struct PanelBorder : TransparentWidget {
	void draw(NVGcontext *vg) override;
};


struct SVGPanel : FramebufferWidget {
	void step() override;
	void setBackground(std::shared_ptr<SVG> svg);
};



} // namespace rack

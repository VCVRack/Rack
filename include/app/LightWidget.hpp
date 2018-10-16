#pragma once
#include "app/common.hpp"


namespace rack {


struct LightWidget : TransparentWidget {
	NVGcolor bgColor = nvgRGBA(0, 0, 0, 0);
	NVGcolor color = nvgRGBA(0, 0, 0, 0);
	NVGcolor borderColor = nvgRGBA(0, 0, 0, 0);
	void draw(NVGcontext *vg) override;
	virtual void drawLight(NVGcontext *vg);
	virtual void drawHalo(NVGcontext *vg);
};


} // namespace rack

#pragma once
#include "app/common.hpp"
#include "widgets/TransparentWidget.hpp"


namespace rack {


struct LightWidget : TransparentWidget {
	NVGcolor bgColor = nvgRGBA(0, 0, 0, 0);
	NVGcolor color = nvgRGBA(0, 0, 0, 0);
	NVGcolor borderColor = nvgRGBA(0, 0, 0, 0);
	void draw(const DrawContext &ctx) override;
	virtual void drawLight(const DrawContext &ctx);
	virtual void drawHalo(const DrawContext &ctx);
};


} // namespace rack

#pragma once
#include "app/common.hpp"
#include "widget/TransparentWidget.hpp"


namespace rack {
namespace app {


struct LightWidget : widget::TransparentWidget {
	NVGcolor bgColor = nvgRGBA(0, 0, 0, 0);
	NVGcolor color = nvgRGBA(0, 0, 0, 0);
	NVGcolor borderColor = nvgRGBA(0, 0, 0, 0);
	void draw(const widget::DrawContext &ctx) override;
	virtual void drawLight(const widget::DrawContext &ctx);
	virtual void drawHalo(const widget::DrawContext &ctx);
};


} // namespace app
} // namespace rack

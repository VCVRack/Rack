#pragma once
#include "widgets/OpaqueWidget.hpp"
#include "ui/Quantity.hpp"
#include "ui/common.hpp"
#include "app.hpp"


namespace rack {


struct Slider : OpaqueWidget {
	BNDwidgetState state = BND_DEFAULT;
	Quantity *quantity = NULL;

	Slider();
	~Slider();
	void draw(const DrawContext &ctx) override;
	void onDragStart(const event::DragStart &e) override;
	void onDragMove(const event::DragMove &e) override;
	void onDragEnd(const event::DragEnd &e) override;
	void onButton(const event::Button &e) override;
};


} // namespace rack

#pragma once
#include "app/common.hpp"
#include "widgets/OpaqueWidget.hpp"


namespace rack {


struct Toolbar : OpaqueWidget {
	// TODO Move these to future Rack app state
	float cableOpacity = 0.5;
	float cableTension = 0.5;

	Toolbar();
	void draw(const DrawContext &ctx) override;
};


} // namespace rack

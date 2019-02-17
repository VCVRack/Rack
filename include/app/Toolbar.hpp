#pragma once
#include "app/common.hpp"
#include "widget/OpaqueWidget.hpp"


namespace rack {
namespace app {


struct Toolbar : widget::OpaqueWidget {
	// TODO Move these to future Rack app state
	float cableOpacity = 0.5;
	float cableTension = 0.5;

	Toolbar();
	void draw(const DrawArgs &args) override;
};


} // namespace app
} // namespace rack

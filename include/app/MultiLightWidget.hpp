#pragma once
#include "app/common.hpp"
#include "app/LightWidget.hpp"


namespace rack {
namespace app {


/** Mixes a list of colors based on a list of brightness values */
struct MultiLightWidget : LightWidget {
	/** Colors of each value state */
	std::vector<NVGcolor> baseColors;
	void addBaseColor(NVGcolor baseColor);
	/** Sets the color to a linear combination of the baseColors with the given weights */
	void setBrightnesses(const std::vector<float> &brightnesses);
};


} // namespace app
} // namespace rack

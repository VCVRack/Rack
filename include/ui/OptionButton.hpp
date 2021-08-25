#pragma once
#include <ui/common.hpp>
#include <ui/RadioButton.hpp>


namespace rack {
namespace ui {


/** Behaves like a RadioButton and appears with a checkmark beside text.
*/
struct OptionButton : RadioButton {
	void draw(const DrawArgs& args) override;
};


} // namespace ui
} // namespace rack

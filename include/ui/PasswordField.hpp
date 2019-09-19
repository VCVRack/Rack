#pragma once
#include <ui/common.hpp>
#include <ui/TextField.hpp>


namespace rack {
namespace ui {


struct PasswordField : TextField {
	void draw(const DrawArgs& args) override;
};


} // namespace ui
} // namespace rack

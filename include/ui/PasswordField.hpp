#pragma once
#include "ui/common.hpp"
#include "ui/TextField.hpp"


namespace rack {


struct PasswordField : TextField {
	void draw(NVGcontext *vg) override;
};


} // namespace rack

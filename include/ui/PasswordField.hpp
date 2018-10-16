#pragma once
#include "ui/common.hpp"
#include "ui/TextField.hpp"


namespace rack {


struct PasswordField : TextField {
	void draw(NVGcontext *vg) override {
		std::string textTmp = text;
		text = std::string(textTmp.size(), '*');
		TextField::draw(vg);
		text = textTmp;
	}
};


} // namespace rack

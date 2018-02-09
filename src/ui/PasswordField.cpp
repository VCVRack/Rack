#include "ui.hpp"


namespace rack {


void PasswordField::draw(NVGcontext *vg) {
	std::string textTmp = text;
	text = std::string(textTmp.size(), '*');
	TextField::draw(vg);
	text = textTmp;
}


} // namespace rack

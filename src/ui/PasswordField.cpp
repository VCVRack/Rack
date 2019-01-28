#include "ui/PasswordField.hpp"


namespace rack {


void PasswordField::draw(const DrawContext &ctx) {
	std::string textTmp = text;
	text = std::string(textTmp.size(), '*');
	TextField::draw(ctx);
	text = textTmp;
}


} // namespace rack

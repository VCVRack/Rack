#include "ui/PasswordField.hpp"


namespace rack {
namespace ui {


void PasswordField::draw(const widget::DrawContext &ctx) {
	std::string textTmp = text;
	text = std::string(textTmp.size(), '*');
	TextField::draw(ctx);
	text = textTmp;
}


} // namespace ui
} // namespace rack

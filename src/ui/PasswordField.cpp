#include <ui/PasswordField.hpp>


namespace rack {
namespace ui {


void PasswordField::draw(const DrawArgs& args) {
	std::string textTmp = text;
	text = std::string(textTmp.size(), '*');
	TextField::draw(args);
	text = textTmp;
}


} // namespace ui
} // namespace rack

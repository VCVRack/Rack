#include "widgets.hpp"


namespace rack {


#define RIGHT_PADDING 10.0
#define BND_LABEL_FONT_SIZE 13

float MenuItem::computeMinWidth(NVGcontext *vg) {
	return MenuEntry::computeMinWidth(vg) + RIGHT_PADDING + bndLabelWidth(vg, -1, rightText.c_str());
}

void MenuItem::draw(NVGcontext *vg) {
	bndMenuItem(vg, 0.0, 0.0, box.size.x, box.size.y, state, -1, text.c_str());

	float x = box.size.x - bndLabelWidth(vg, -1, rightText.c_str());
	NVGcolor rightColor = (state == BND_DEFAULT) ? bndGetTheme()->menuTheme.textColor : bndGetTheme()->menuTheme.textSelectedColor;
	bndIconLabelValue(vg, x, 0.0, box.size.x, box.size.y, -1,
		rightColor, BND_LEFT, BND_LABEL_FONT_SIZE, rightText.c_str(), NULL);
}

void MenuItem::onMouseEnter() {
	state = BND_HOVER;
}

void MenuItem::onMouseLeave() {
	state = BND_DEFAULT;
}

void MenuItem::onDragDrop(Widget *origin) {
	if (origin != this)
		return;

	onAction();
	// deletes `this`
	gScene->setOverlay(NULL);
}


} // namespace rack

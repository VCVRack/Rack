#include "widgets.hpp"


namespace rack {


void ZoomWidget::draw(NVGcontext *vg) {
	nvgScale(vg, zoom, zoom);
	Widget::draw(vg);
}

Widget *ZoomWidget::onMouseDown(Vec pos, int button) {
	return Widget::onMouseDown(pos.div(zoom), button);
}

Widget *ZoomWidget::onMouseUp(Vec pos, int button) {
	return Widget::onMouseUp(pos.div(zoom), button);
}

Widget *ZoomWidget::onMouseMove(Vec pos, Vec mouseRel) {
	return Widget::onMouseMove(pos.div(zoom), mouseRel);
}

Widget *ZoomWidget::onHoverKey(Vec pos, int key) {
	return Widget::onHoverKey(pos.div(zoom), key);
}

Widget *ZoomWidget::onScroll(Vec pos, Vec scrollRel) {
	return Widget::onScroll(pos.div(zoom), scrollRel);
}


} // namespace rack

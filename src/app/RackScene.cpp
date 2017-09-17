#include "app.hpp"
#include "gui.hpp"


namespace rack {

RackScene::RackScene() {
	scrollWidget = new ScrollWidget();
	{
		assert(!gRackWidget);
		gRackWidget = new RackWidget();
		scrollWidget->container->addChild(gRackWidget);
	}
	addChild(scrollWidget);

	toolbar = new Toolbar();
	addChild(toolbar);
	scrollWidget->box.pos.y = toolbar->box.size.y;
}

void RackScene::step() {
	toolbar->box.size.x = box.size.x;
	scrollWidget->box.size = box.size.minus(scrollWidget->box.pos);

	Scene::step();
}

void RackScene::draw(NVGcontext *vg) {
	Scene::draw(vg);
}

Widget *RackScene::onHoverKey(Vec pos, int key) {
	switch (key) {
		case GLFW_KEY_N:
			if (guiIsModPressed()) {
				gRackWidget->clear();
				return this;
			}
			break;
		case GLFW_KEY_Q:
			if (guiIsModPressed()) {
				guiClose();
				return this;
			}
			break;
	}

	return Widget::onHoverKey(pos, key);
}



} // namespace rack

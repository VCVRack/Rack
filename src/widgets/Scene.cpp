#include "../5V.hpp"


Scene::Scene() {
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

void Scene::onResize() {
	toolbar->box.size.x = box.size.x;
	scrollWidget->box.size = box.size.minus(scrollWidget->box.pos);
	scrollWidget->onResize();
}

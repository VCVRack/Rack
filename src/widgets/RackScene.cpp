#include "scene.hpp"
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

	// // Draw custom stuff here
	// static std::shared_ptr<SVG> svg;
	// if (!svg)
	// 	svg = SVG::load("res/ComponentLibrary/CL1362.svg");

	// for (float y = 0.0; y < 1000.0; y += 200.0)
	// for (float x = 0.0; x < 1000.0; x += 200.0) {
	// 	nvgSave(vg);
	// 	nvgTranslate(vg, x, y);
	// 	drawSVG(vg, svg->handle);
	// 	nvgRestore(vg);
	// }
}



} // namespace rack

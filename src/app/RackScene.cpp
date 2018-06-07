#include "app.hpp"
#include "window.hpp"
#include "util/request.hpp"
#include <string.h>
#include <thread>


namespace rack {


RackScene::RackScene() {
	scrollWidget = new RackScrollWidget();
	{
		zoomWidget = new ZoomWidget();
		{
			assert(!gRackWidget);
			gRackWidget = new RackWidget();
			zoomWidget->addChild(gRackWidget);
		}
		scrollWidget->container->addChild(zoomWidget);
	}
	addChild(scrollWidget);

	gToolbar = new Toolbar();
	addChild(gToolbar);
	scrollWidget->box.pos.y = gToolbar->box.size.y;
}

void RackScene::step() {
	// Resize owned descendants
	gToolbar->box.size.x = box.size.x;
	scrollWidget->box.size = box.size.minus(scrollWidget->box.pos);

	// Resize to be a bit larger than the ScrollWidget viewport
	gRackWidget->box.size = scrollWidget->box.size
		.minus(scrollWidget->container->box.pos)
		.plus(Vec(500, 500))
		.div(zoomWidget->zoom);

	Scene::step();

	zoomWidget->box.size = gRackWidget->box.size.mult(zoomWidget->zoom);
}

void RackScene::draw(NVGcontext *vg) {
	Scene::draw(vg);
}

void RackScene::onHoverKey(EventHoverKey &e) {
	Widget::onHoverKey(e);

	if (!e.consumed) {
		switch (e.key) {
			case GLFW_KEY_N: {
				if (windowIsModPressed() && !windowIsShiftPressed()) {
					gRackWidget->reset();
					e.consumed = true;
				}
			} break;
			case GLFW_KEY_Q: {
				if (windowIsModPressed() && !windowIsShiftPressed()) {
					windowClose();
					e.consumed = true;
				}
			} break;
			case GLFW_KEY_O: {
				if (windowIsModPressed() && !windowIsShiftPressed()) {
					gRackWidget->openDialog();
					e.consumed = true;
				}
			} break;
			case GLFW_KEY_S: {
				if (windowIsModPressed() && !windowIsShiftPressed()) {
					gRackWidget->saveDialog();
					e.consumed = true;
				}
				if (windowIsModPressed() && windowIsShiftPressed()) {
					gRackWidget->saveAsDialog();
					e.consumed = true;
				}
			} break;
			case GLFW_KEY_ENTER:
			case GLFW_KEY_KP_ENTER: {
				appModuleBrowserCreate();
				e.consumed = true;
			} break;
			case GLFW_KEY_F11: {
				windowSetFullScreen(!windowGetFullScreen());
			}
		}
	}
}

void RackScene::onPathDrop(EventPathDrop &e) {
	if (e.paths.size() >= 1) {
		const std::string& firstPath = e.paths.front();
		if (stringExtension(firstPath) == "vcv") {
			gRackWidget->loadPatch(firstPath);
			e.consumed = true;
		}
	}

	if (!e.consumed)
		Scene::onPathDrop(e);
}


} // namespace rack

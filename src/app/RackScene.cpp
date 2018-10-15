#include <thread>
#include "osdialog.h"
#include "rack.hpp"


namespace rack {


RackScene::RackScene() {
	scrollWidget = new RackScrollWidget;
	{
		zoomWidget = new ZoomWidget;
		{
			assert(!gRackWidget);
			gRackWidget = new RackWidget;
			zoomWidget->addChild(gRackWidget);
		}
		scrollWidget->container->addChild(zoomWidget);
	}
	addChild(scrollWidget);

	gToolbar = new Toolbar;
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

	// Version popup message
	if (!gLatestVersion.empty()) {
		std::string versionMessage = string::f("Rack %s is available.\n\nYou have Rack %s.\n\nClose Rack and download new version on the website?", gLatestVersion.c_str(), gApplicationVersion.c_str());
		if (osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, versionMessage.c_str())) {
			std::thread t(system::openBrowser, "https://vcvrack.com/");
			t.detach();
			windowClose();
		}
		gLatestVersion = "";
	}
}

void RackScene::draw(NVGcontext *vg) {
	Scene::draw(vg);
}

void RackScene::onHoverKey(event::HoverKey &e) {
	Scene::onHoverKey(e);

	if (!e.target) {
		switch (e.key) {
			case GLFW_KEY_N: {
				if (windowIsModPressed() && !windowIsShiftPressed()) {
					gRackWidget->reset();
					e.target = this;
				}
			} break;
			case GLFW_KEY_Q: {
				if (windowIsModPressed() && !windowIsShiftPressed()) {
					windowClose();
					e.target = this;
				}
			} break;
			case GLFW_KEY_O: {
				if (windowIsModPressed() && !windowIsShiftPressed()) {
					gRackWidget->loadDialog();
					e.target = this;
				}
				if (windowIsModPressed() && windowIsShiftPressed()) {
					gRackWidget->revert();
					e.target = this;
				}
			} break;
			case GLFW_KEY_S: {
				if (windowIsModPressed() && !windowIsShiftPressed()) {
					gRackWidget->saveDialog();
					e.target = this;
				}
				if (windowIsModPressed() && windowIsShiftPressed()) {
					gRackWidget->saveAsDialog();
					e.target = this;
				}
			} break;
			case GLFW_KEY_V: {
				if (windowIsModPressed() && !windowIsShiftPressed()) {
					gRackWidget->pastePresetClipboard();
					e.target = this;
				}
			} break;
			case GLFW_KEY_ENTER:
			case GLFW_KEY_KP_ENTER: {
				appModuleBrowserCreate();
				e.target = this;
			} break;
			case GLFW_KEY_F11: {
				windowSetFullScreen(!windowGetFullScreen());
			}
		}
	}
}

void RackScene::onPathDrop(event::PathDrop &e) {
	if (e.paths.size() >= 1) {
		const std::string &path = e.paths[0];
		if (string::extension(path) == "vcv") {
			gRackWidget->load(path);
			e.target = this;
		}
	}

	if (!e.target)
		Scene::onPathDrop(e);
}


} // namespace rack

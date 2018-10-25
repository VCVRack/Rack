#include "global_pre.hpp"
#include "app.hpp"
#include "window.hpp"
#include "util/request.hpp"
#include "osdialog.h"
#include <string.h>
#include <thread>
#include "global_ui.hpp"


namespace rack {


RackScene::RackScene() {
	scrollWidget = new RackScrollWidget();
	{
		zoomWidget = new ZoomWidget();
		{
			assert(!global_ui->app.gRackWidget);
			global_ui->app.gRackWidget = new RackWidget();
			zoomWidget->addChild(global_ui->app.gRackWidget);
		}
		scrollWidget->container->addChild(zoomWidget);
	}
	addChild(scrollWidget);

	global_ui->app.gToolbar = new Toolbar();
	addChild(global_ui->app.gToolbar);
	scrollWidget->box.pos.y = global_ui->app.gToolbar->box.size.y;
}

void RackScene::step() {
	// Resize owned descendants
	global_ui->app.gToolbar->box.size.x = box.size.x;
	scrollWidget->box.size = box.size.minus(scrollWidget->box.pos);

	// Resize to be a bit larger than the ScrollWidget viewport
	global_ui->app.gRackWidget->box.size = scrollWidget->box.size
		.minus(scrollWidget->container->box.pos)
		.plus(Vec(500, 500))
		.div(zoomWidget->zoom);

	Scene::step();

	zoomWidget->box.size = global_ui->app.gRackWidget->box.size.mult(zoomWidget->zoom);

	// Version popup message
	if (!global_ui->app.gLatestVersion.empty()) {
		std::string versionMessage = stringf("Rack %s is available.\n\nYou have Rack %s.\n\nClose Rack and download new version on the website?", global_ui->app.gLatestVersion.c_str(), global_ui->app.gApplicationVersion.c_str());
		if (osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, versionMessage.c_str())) {
			std::thread t(systemOpenBrowser, "https://vcvrack.com/");
			t.detach();
			windowClose();
		}
		global_ui->app.gLatestVersion = "";
	}
}

void RackScene::draw(NVGcontext *vg) {
   // printf("xxx RackScene::draw\n");
	Scene::draw(vg);
}

void RackScene::onHoverKey(EventHoverKey &e) {
	Widget::onHoverKey(e);

	if (!e.consumed) {
		switch (e.key) {
			case 'n'/*GLFW_KEY_N*/:
				if (windowIsModPressed() && !windowIsShiftPressed()) {
					global_ui->app.gRackWidget->reset();
					e.consumed = true;
				}
            break;
			case 'q'/*GLFW_KEY_Q*/:
				if (windowIsModPressed() && !windowIsShiftPressed()) {
					windowClose();
					e.consumed = true;
				}
            break;
			case 'o'/*GLFW_KEY_O*/:
				if (windowIsModPressed() && !windowIsShiftPressed()) {
					global_ui->app.gRackWidget->openDialog();
					e.consumed = true;
				}
				if (windowIsModPressed() && windowIsShiftPressed()) {
					global_ui->app.gRackWidget->revert();
					e.consumed = true;
				}
            break;
			case 's'/*GLFW_KEY_S*/:
				if (windowIsModPressed() && !windowIsShiftPressed()) {
					global_ui->app.gRackWidget->saveDialog();
					e.consumed = true;
				}
				if (windowIsModPressed() && windowIsShiftPressed()) {
					global_ui->app.gRackWidget->saveAsDialog();
					e.consumed = true;
				}
            break;
			case LGLW_VKEY_RETURN/*GLFW_KEY_ENTER*/:
			// case GLFW_KEY_KP_ENTER:
				appModuleBrowserCreate();
				e.consumed = true;
            break;
			case LGLW_VKEY_F11/*GLFW_KEY_F11*/:
				windowSetFullScreen(!windowGetFullScreen());
            break;
		}
	}
}

void RackScene::onPathDrop(EventPathDrop &e) {
	if (e.paths.size() >= 1) {
		const std::string& firstPath = e.paths.front();
		if (stringExtension(firstPath) == "vcv") {
			global_ui->app.gRackWidget->loadPatch(firstPath);
			e.consumed = true;
		}
	}

	if (!e.consumed)
		Scene::onPathDrop(e);
}


} // namespace rack

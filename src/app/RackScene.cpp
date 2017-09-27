#include "app.hpp"
#include "gui.hpp"
#include "util/request.hpp"
#include "../ext/osdialog/osdialog.h"
#include <thread>


namespace rack {


static std::string versionMessage = "";

static void checkVersion() {
	json_t *resJ = requestJson(METHOD_GET, gApiHost + "/version", NULL);

	if (resJ) {
		json_t *versionJ = json_object_get(resJ, "version");
		if (versionJ) {
			const char *version = json_string_value(versionJ);
			if (version && version != gApplicationVersion) {
				versionMessage = stringf("Rack %s is available.\n\nYou have Rack %s.\n\nWould you like to download the new version on the website?", version, gApplicationVersion.c_str());
			}
		}
		json_decref(resJ);
	}
}


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

	// Check for new version
	if (gApplicationVersion != "dev") {
		std::thread versionThread(checkVersion);
		versionThread.detach();
	}
}

void RackScene::step() {
	toolbar->box.size.x = box.size.x;
	scrollWidget->box.size = box.size.minus(scrollWidget->box.pos);

	Scene::step();

	// Version popup message
	if (!versionMessage.empty()) {
		if (osdialog_message(OSDIALOG_INFO, OSDIALOG_YES_NO, versionMessage.c_str())) {
			std::thread t(openBrowser, "https://vcvrack.com/");
			t.detach();
			guiClose();
		}
		versionMessage = "";
	}
}

void RackScene::draw(NVGcontext *vg) {
	Scene::draw(vg);
}

Widget *RackScene::onHoverKey(Vec pos, int key) {
	Widget *w = Widget::onHoverKey(pos, key);
	if (w) return w;

	switch (key) {
		case GLFW_KEY_N:
			if (guiIsModPressed() && !guiIsShiftPressed()) {
				gRackWidget->clear();
				return this;
			}
			break;
		case GLFW_KEY_Q:
			if (guiIsModPressed() && !guiIsShiftPressed()) {
				guiClose();
				return this;
			}
			break;
		case GLFW_KEY_O:
			if (guiIsModPressed() && !guiIsShiftPressed()) {
				gRackWidget->openDialog();
				return this;
			}
			break;
		case GLFW_KEY_S:
			if (guiIsModPressed() && !guiIsShiftPressed()) {
				gRackWidget->saveDialog();
				return this;
			}
			if (guiIsModPressed() && guiIsShiftPressed()) {
				gRackWidget->saveAsDialog();
				return this;
			}
			break;
	}

	return NULL;
}



} // namespace rack

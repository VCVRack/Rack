#include "app/Scene.hpp"
#include "app/ModuleBrowser.hpp"
#include "app.hpp"
#include "system.hpp"
#include "network.hpp"
#include "history.hpp"
#include "settings.hpp"
#include "patch.hpp"
#include "asset.hpp"
#include "osdialog.h"
#include <thread>


namespace rack {
namespace app {


Scene::Scene() {
	rackScroll = new RackScrollWidget;
	addChild(rackScroll);

	rack = rackScroll->rackWidget;

	toolbar = new Toolbar;
	addChild(toolbar);
	rackScroll->box.pos.y = toolbar->box.size.y;

	moduleBrowser = moduleBrowserCreate();
	moduleBrowser->hide();
	addChild(moduleBrowser);
}

Scene::~Scene() {
}

void Scene::step() {
	// Resize owned descendants
	toolbar->box.size.x = box.size.x;
	rackScroll->box.size = box.size.minus(rackScroll->box.pos);

	// Autosave every 15 seconds
	double time = glfwGetTime();
	if (time - lastAutoSaveTime >= 15.0) {
		lastAutoSaveTime = time;
		APP->patch->save(asset::user("autosave.vcv"));
		settings.save(asset::user("settings.json"));
	}

	// Request latest version from server
	if (!settings.devMode && checkVersion && !checkedVersion) {
		std::thread t(&Scene::runCheckVersion, this);
		t.detach();
		checkedVersion = true;
	}

	// Version popup message
	if (!latestVersion.empty()) {
		std::string versionMessage = string::f("Rack v%s is available.\n\nYou have Rack v%s.\n\nClose Rack and download new version on the website?", latestVersion.c_str(), app::APP_VERSION);
		if (osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, versionMessage.c_str())) {
			std::thread t(system::openBrowser, "https://vcvrack.com/");
			t.detach();
			APP->window->close();
		}
		latestVersion = "";
	}

	OpaqueWidget::step();
}

void Scene::draw(const DrawArgs &args) {
	OpaqueWidget::draw(args);
}

void Scene::onHoverKey(const widget::HoverKeyEvent &e) {
	if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
		switch (e.key) {
			case GLFW_KEY_N: {
				if ((e.mods & WINDOW_MOD_MASK) == WINDOW_MOD_CTRL) {
					APP->patch->resetDialog();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_Q: {
				if ((e.mods & WINDOW_MOD_MASK) == WINDOW_MOD_CTRL) {
					APP->window->close();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_O: {
				if ((e.mods & WINDOW_MOD_MASK) == WINDOW_MOD_CTRL) {
					APP->patch->loadDialog();
					e.consume(this);
				}
				if ((e.mods & WINDOW_MOD_MASK) == (WINDOW_MOD_CTRL | GLFW_MOD_SHIFT)) {
					APP->patch->revertDialog();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_S: {
				if ((e.mods & WINDOW_MOD_MASK) == WINDOW_MOD_CTRL) {
					APP->patch->saveDialog();
					e.consume(this);
				}
				if ((e.mods & WINDOW_MOD_MASK) == (WINDOW_MOD_CTRL | GLFW_MOD_SHIFT)) {
					APP->patch->saveAsDialog();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_Z: {
				if ((e.mods & WINDOW_MOD_MASK) == WINDOW_MOD_CTRL) {
					APP->history->undo();
					e.consume(this);
				}
				if ((e.mods & WINDOW_MOD_MASK) == (WINDOW_MOD_CTRL | GLFW_MOD_SHIFT)) {
					APP->history->redo();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_ENTER:
			case GLFW_KEY_KP_ENTER: {
				moduleBrowser->show();
				e.consume(this);
			} break;
			case GLFW_KEY_F11: {
				APP->window->setFullScreen(!APP->window->isFullScreen());
				e.consume(this);
			}
		}
	}

	if (!e.getConsumed())
		OpaqueWidget::onHoverKey(e);
}

void Scene::onPathDrop(const widget::PathDropEvent &e) {
	if (e.paths.size() >= 1) {
		const std::string &path = e.paths[0];
		if (string::extension(path) == "vcv") {
			APP->patch->load(path);
			e.consume(this);
		}
	}

	if (!e.getConsumed())
		OpaqueWidget::onPathDrop(e);
}

void Scene::runCheckVersion() {
	std::string versionUrl = app::API_URL;
	versionUrl += "/version";
	json_t *versionResJ = network::requestJson(network::METHOD_GET, versionUrl, NULL);

	if (versionResJ) {
		json_t *versionJ = json_object_get(versionResJ, "version");
		if (versionJ) {
			std::string version = json_string_value(versionJ);
			if (version != app::APP_VERSION) {
				latestVersion = version;
			}
		}
		json_decref(versionResJ);
	}
}


} // namespace app
} // namespace rack

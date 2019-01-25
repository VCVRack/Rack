#include "system.hpp"
#include "network.hpp"
#include "app/Scene.hpp"
#include "app/ModuleBrowser.hpp"
#include "app/RackScrollWidget.hpp"
#include "app.hpp"
#include "history.hpp"
#include "settings.hpp"
#include "patch.hpp"
#include "asset.hpp"
#include "osdialog.h"
#include <thread>


namespace rack {


Scene::Scene() {
	scrollWidget = new RackScrollWidget;
	{
		zoomWidget = new ZoomWidget;
		{
			rackWidget = new RackWidget;
			zoomWidget->addChild(rackWidget);
		}
		scrollWidget->container->addChild(zoomWidget);
	}
	addChild(scrollWidget);

	toolbar = new Toolbar;
	addChild(toolbar);
	scrollWidget->box.pos.y = toolbar->box.size.y;

	moduleBrowser = new ModuleBrowser;
	moduleBrowser->visible = false;
	addChild(moduleBrowser);
}

Scene::~Scene() {
}

void Scene::step() {
	// Resize owned descendants
	toolbar->box.size.x = box.size.x;
	scrollWidget->box.size = box.size.minus(scrollWidget->box.pos);

	// Resize to be a bit larger than the ScrollWidget viewport
	rackWidget->box.size = scrollWidget->box.size
		.minus(scrollWidget->container->box.pos)
		.plus(math::Vec(500, 500))
		.div(zoomWidget->zoom);

	OpaqueWidget::step();

	zoomWidget->box.size = rackWidget->box.size.mult(zoomWidget->zoom);
	moduleBrowser->box.size = box.size;

	// Autosave every 15 seconds
	int frame = app()->window->frame;
	if (frame > 0 && frame % (60 * 15) == 0) {
		app()->patch->save(asset::user("autosave.vcv"));
		settings::save(asset::user("settings.json"));
	}

	// Set zoom every few frames
	if (app()->window->frame % 10 == 0)
		zoomWidget->setZoom(settings::zoom);

	// Request latest version from server
	if (!devMode && checkVersion && !checkedVersion) {
		std::thread t(&Scene::runCheckVersion, this);
		t.detach();
		checkedVersion = true;
	}

	// Version popup message
	if (!latestVersion.empty()) {
		std::string versionMessage = string::f("Rack %s is available.\n\nYou have Rack %s.\n\nClose Rack and download new version on the website?", latestVersion.c_str(), APP_VERSION);
		if (osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, versionMessage.c_str())) {
			std::thread t(system::openBrowser, "https://vcvrack.com/");
			t.detach();
			app()->window->close();
		}
		latestVersion = "";
	}
}

void Scene::draw(NVGcontext *vg) {
	OpaqueWidget::draw(vg);
}

void Scene::onHoverKey(const event::HoverKey &e) {
	if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
		switch (e.key) {
			case GLFW_KEY_N: {
				if ((e.mods & WINDOW_MOD_MASK) == WINDOW_MOD_CTRL) {
					app()->patch->resetDialog();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_Q: {
				if ((e.mods & WINDOW_MOD_MASK) == WINDOW_MOD_CTRL) {
					app()->window->close();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_O: {
				if ((e.mods & WINDOW_MOD_MASK) == WINDOW_MOD_CTRL) {
					app()->patch->loadDialog();
					e.consume(this);
				}
				if ((e.mods & WINDOW_MOD_MASK) == (WINDOW_MOD_CTRL | GLFW_MOD_SHIFT)) {
					app()->patch->revertDialog();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_S: {
				if ((e.mods & WINDOW_MOD_MASK) == WINDOW_MOD_CTRL) {
					app()->patch->saveDialog();
					e.consume(this);
				}
				if ((e.mods & WINDOW_MOD_MASK) == (WINDOW_MOD_CTRL | GLFW_MOD_SHIFT)) {
					app()->patch->saveAsDialog();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_Z: {
				if ((e.mods & WINDOW_MOD_MASK) == WINDOW_MOD_CTRL) {
					app()->history->undo();
					e.consume(this);
				}
				if ((e.mods & WINDOW_MOD_MASK) == (WINDOW_MOD_CTRL | GLFW_MOD_SHIFT)) {
					app()->history->redo();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_ENTER:
			case GLFW_KEY_KP_ENTER: {
				moduleBrowser->visible = true;
				e.consume(this);
			} break;
			case GLFW_KEY_F11: {
				app()->window->setFullScreen(!app()->window->isFullScreen());
				e.consume(this);
			}
		}
	}

	if (!e.getConsumed())
		OpaqueWidget::onHoverKey(e);
}

void Scene::onPathDrop(const event::PathDrop &e) {
	if (e.paths.size() >= 1) {
		const std::string &path = e.paths[0];
		if (string::extension(path) == "vcv") {
			app()->patch->load(path);
			e.consume(this);
		}
	}

	if (!e.getConsumed())
		OpaqueWidget::onPathDrop(e);
}

void Scene::runCheckVersion() {
	std::string versionUrl = API_HOST;
	versionUrl += "/version";
	json_t *versionResJ = network::requestJson(network::METHOD_GET, versionUrl, NULL);

	if (versionResJ) {
		json_t *versionJ = json_object_get(versionResJ, "version");
		if (versionJ) {
			std::string version = json_string_value(versionJ);
			if (version != APP_VERSION) {
				latestVersion = version;
			}
		}
		json_decref(versionResJ);
	}
}


} // namespace rack

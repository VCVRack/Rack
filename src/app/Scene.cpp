#include <app/Scene.hpp>
#include <app/ModuleBrowser.hpp>
#include <app.hpp>
#include <system.hpp>
#include <network.hpp>
#include <history.hpp>
#include <settings.hpp>
#include <patch.hpp>
#include <asset.hpp>
#include <osdialog.h>
#include <thread>


namespace rack {
namespace app {


Scene::Scene() {
	rackScroll = new RackScrollWidget;
	addChild(rackScroll);

	rack = rackScroll->rackWidget;

	menuBar = createMenuBar();
	addChild(menuBar);
	rackScroll->box.pos.y = menuBar->box.size.y;

	moduleBrowser = moduleBrowserCreate();
	moduleBrowser->hide();
	addChild(moduleBrowser);
}

Scene::~Scene() {
}

void Scene::step() {
	// Resize owned descendants
	menuBar->box.size.x = box.size.x;
	rackScroll->box.size = box.size.minus(rackScroll->box.pos);

	// Autosave every 15 seconds
	if (settings::autosavePeriod > 0.0) {
		double time = glfwGetTime();
		if (time - lastAutosaveTime >= settings::autosavePeriod) {
			lastAutosaveTime = time;
			APP->patch->save(asset::autosavePath);
			settings::save(asset::settingsPath);
		}
	}

	Widget::step();
}

void Scene::draw(const DrawArgs& args) {
	Widget::draw(args);
}

void Scene::onHoverKey(const event::HoverKey& e) {
	OpaqueWidget::onHoverKey(e);
	if (e.isConsumed())
		return;

	if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
		switch (e.key) {
			case GLFW_KEY_N: {
				if ((e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
					APP->patch->resetDialog();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_Q: {
				if ((e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
					APP->window->close();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_O: {
				if ((e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
					APP->patch->loadDialog();
					e.consume(this);
				}
				if ((e.mods & RACK_MOD_MASK) == (RACK_MOD_CTRL | GLFW_MOD_SHIFT)) {
					APP->patch->revertDialog();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_S: {
				if ((e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
					APP->patch->saveDialog();
					e.consume(this);
				}
				if ((e.mods & RACK_MOD_MASK) == (RACK_MOD_CTRL | GLFW_MOD_SHIFT)) {
					APP->patch->saveAsDialog();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_Z: {
				if ((e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
					APP->history->undo();
					e.consume(this);
				}
				if ((e.mods & RACK_MOD_MASK) == (RACK_MOD_CTRL | GLFW_MOD_SHIFT)) {
					APP->history->redo();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_MINUS: {
				if ((e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
					float zoom = settings::zoom;
					zoom *= 2;
					zoom = std::ceil(zoom - 0.01f) - 1;
					zoom /= 2;
					settings::zoom = zoom;
					e.consume(this);
				}
			} break;
			case GLFW_KEY_EQUAL: {
				if ((e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
					float zoom = settings::zoom;
					zoom *= 2;
					zoom = std::floor(zoom + 0.01f) + 1;
					zoom /= 2;
					settings::zoom = zoom;
					e.consume(this);
				}
			} break;
			case GLFW_KEY_0: {
				if ((e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
					settings::zoom = 0.f;
					e.consume(this);
				}
			} break;
			case GLFW_KEY_ENTER: {
				if ((e.mods & RACK_MOD_MASK) == 0) {
					moduleBrowser->show();
				}
				e.consume(this);
			} break;
			case GLFW_KEY_F1: {
				if ((e.mods & RACK_MOD_MASK) == 0) {
					std::thread t([] {
						system::openBrowser("https://vcvrack.com/manual/");
					});
					t.detach();
					e.consume(this);
				}
			} break;
			case GLFW_KEY_F11: {
				if ((e.mods & RACK_MOD_MASK) == 0) {
					APP->window->setFullScreen(!APP->window->isFullScreen());
					e.consume(this);
				}
			} break;
		}
	}
}

void Scene::onPathDrop(const event::PathDrop& e) {
	if (e.paths.size() >= 1) {
		const std::string& path = e.paths[0];
		if (string::filenameExtension(string::filename(path)) == "vcv") {
			APP->patch->loadPathDialog(path);
			e.consume(this);
			return;
		}
	}

	OpaqueWidget::onPathDrop(e);
}


} // namespace app
} // namespace rack

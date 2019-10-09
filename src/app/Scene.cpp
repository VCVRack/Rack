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


struct FrameRateWidget : widget::TransparentWidget {
	void draw(const DrawArgs& args) override {
		std::string text = string::f("%.2f Hz", APP->window->getLastFrameRate());
		bndLabel(args.vg, 0.0, 0.0, INFINITY, INFINITY, -1, text.c_str());
	}
};


Scene::Scene() {
	rackScroll = new RackScrollWidget;
	addChild(rackScroll);

	rack = rackScroll->rackWidget;

	menuBar = createMenuBar();
	addChild(menuBar);

	moduleBrowser = moduleBrowserCreate();
	moduleBrowser->hide();
	addChild(moduleBrowser);

	frameRateWidget = new FrameRateWidget;
	frameRateWidget->box.size = math::Vec(80.0, 30.0);
	frameRateWidget->hide();
	addChild(frameRateWidget);
}

Scene::~Scene() {
}

void Scene::step() {
	bool fullscreen = APP->window->isFullScreen();
	menuBar->visible = !fullscreen;
	rackScroll->box.pos.y = menuBar->visible ? menuBar->box.size.y : 0;
	frameRateWidget->box.pos.x = box.size.x - frameRateWidget->box.size.x;

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
		if (e.key == GLFW_KEY_N && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			APP->patch->resetDialog();
			e.consume(this);
		}
		else if (e.key == GLFW_KEY_Q && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			APP->window->close();
			e.consume(this);
		}
		else if (e.key == GLFW_KEY_O && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			APP->patch->loadDialog();
			e.consume(this);
		}
		else if (e.key == GLFW_KEY_O && (e.mods & RACK_MOD_MASK) == (RACK_MOD_CTRL | GLFW_MOD_SHIFT)) {
			APP->patch->revertDialog();
			e.consume(this);
		}
		else if (e.key == GLFW_KEY_S && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			APP->patch->saveDialog();
			e.consume(this);
		}
		else if (e.key == GLFW_KEY_S && (e.mods & RACK_MOD_MASK) == (RACK_MOD_CTRL | GLFW_MOD_SHIFT)) {
			APP->patch->saveAsDialog();
			e.consume(this);
		}
		else if (e.key == GLFW_KEY_Z && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			APP->history->undo();
			e.consume(this);
		}
		else if (e.key == GLFW_KEY_Z && (e.mods & RACK_MOD_MASK) == (RACK_MOD_CTRL | GLFW_MOD_SHIFT)) {
			APP->history->redo();
			e.consume(this);
		}
		else if ((e.key == GLFW_KEY_MINUS || e.key == GLFW_KEY_KP_SUBTRACT) && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			float zoom = settings::zoom;
			zoom *= 2;
			zoom = std::ceil(zoom - 0.01f) - 1;
			zoom /= 2;
			settings::zoom = zoom;
			e.consume(this);
		}
		else if ((e.key == GLFW_KEY_EQUAL || e.key == GLFW_KEY_KP_ADD) && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			float zoom = settings::zoom;
			zoom *= 2;
			zoom = std::floor(zoom + 0.01f) + 1;
			zoom /= 2;
			settings::zoom = zoom;
			e.consume(this);
		}
		else if ((e.key == GLFW_KEY_0 || e.key == GLFW_KEY_KP_0) && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			settings::zoom = 0.f;
			e.consume(this);
		}
		else if ((e.key == GLFW_KEY_ENTER || e.key == GLFW_KEY_KP_ENTER) && (e.mods & RACK_MOD_MASK) == 0) {
			moduleBrowser->show();
			e.consume(this);
		}
		else if (e.key == GLFW_KEY_F1 && (e.mods & RACK_MOD_MASK) == 0) {
			std::thread t([] {
				system::openBrowser("https://vcvrack.com/manual/");
			});
			t.detach();
			e.consume(this);
		}
		else if (e.key == GLFW_KEY_F3 && (e.mods & RACK_MOD_MASK) == 0) {
			settings::cpuMeter ^= true;
			e.consume(this);
		}
		else if (e.key == GLFW_KEY_F11 && (e.mods & RACK_MOD_MASK) == 0) {
			APP->window->setFullScreen(!APP->window->isFullScreen());
			e.consume(this);
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

#pragma once
#include "rack.hpp"

using namespace rack;

struct JWModuleResizeHandle : Widget {

	float minWidth;
	bool right = false;
	float dragX;
	Rect originalBox;

	JWModuleResizeHandle(float _minWidth) {
		box.size = Vec(RACK_GRID_WIDTH * 1, RACK_GRID_HEIGHT);
		minWidth = _minWidth;
	}
	
	void onMouseDown(EventMouseDown &e) override {
		if (e.button == 0) {
			e.consumed = true;
			e.target = this;
		}
	}

	void onDragStart(EventDragStart &e) override {
		dragX = RACK_PLUGIN_UI_RACKWIDGET->lastMousePos.x;
		ModuleWidget *m = getAncestorOfType<ModuleWidget>();
		originalBox = m->box;
	}

	void onDragMove(EventDragMove &e) override {
		ModuleWidget *m = getAncestorOfType<ModuleWidget>();

		float newDragX = RACK_PLUGIN_UI_RACKWIDGET->lastMousePos.x;
		float deltaX = newDragX - dragX;

		Rect newBox = originalBox;
		if (right) {
			newBox.size.x += deltaX;
			newBox.size.x = fmaxf(newBox.size.x, minWidth);
			newBox.size.x = roundf(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
		}
		else {
			newBox.size.x -= deltaX;
			newBox.size.x = fmaxf(newBox.size.x, minWidth);
			newBox.size.x = roundf(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
			newBox.pos.x = originalBox.pos.x + originalBox.size.x - newBox.size.x;
		}
		RACK_PLUGIN_UI_RACKWIDGET->requestModuleBox(m, newBox);
	}
};

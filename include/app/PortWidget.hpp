#pragma once
#include "app/common.hpp"
#include "widget/OpaqueWidget.hpp"
#include "app/MultiLightWidget.hpp"
#include "engine/Module.hpp"


namespace rack {
namespace app {


/** Manages an engine::Port on a ModuleWidget. */
struct PortWidget : widget::OpaqueWidget {
	engine::Module *module = NULL;
	int portId;
	bool hovered = false;

	enum Type {
		OUTPUT,
		INPUT
	};
	Type type;
	MultiLightWidget *plugLight;

	PortWidget();
	~PortWidget();

	void step() override;
	void draw(const DrawArgs &args) override;

	void onButton(const widget::ButtonEvent &e) override;
	void onEnter(const widget::EnterEvent &e) override;
	void onLeave(const widget::LeaveEvent &e) override;
	void onDragStart(const widget::DragStartEvent &e) override;
	void onDragEnd(const widget::DragEndEvent &e) override;
	void onDragDrop(const widget::DragDropEvent &e) override;
	void onDragEnter(const widget::DragEnterEvent &e) override;
	void onDragLeave(const widget::DragLeaveEvent &e) override;
};


} // namespace app
} // namespace rack

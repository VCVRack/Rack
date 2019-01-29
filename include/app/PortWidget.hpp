#pragma once
#include "app/common.hpp"
#include "widget/OpaqueWidget.hpp"
#include "app/MultiLightWidget.hpp"
#include "engine/Module.hpp"


namespace rack {
namespace app {


struct PortWidget : widget::OpaqueWidget {
	Module *module = NULL;
	int portId;

	enum Type {
		OUTPUT,
		INPUT
	};
	Type type;
	MultiLightWidget *plugLight;

	PortWidget();
	~PortWidget();

	void step() override;
	void draw(const widget::DrawContext &ctx) override;

	void onButton(const event::Button &e) override;
	void onDragStart(const event::DragStart &e) override;
	void onDragEnd(const event::DragEnd &e) override;
	void onDragDrop(const event::DragDrop &e) override;
	void onDragEnter(const event::DragEnter &e) override;
	void onDragLeave(const event::DragLeave &e) override;
};


} // namespace app
} // namespace rack

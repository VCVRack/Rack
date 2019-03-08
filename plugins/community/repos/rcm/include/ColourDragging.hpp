#include "ModuleDragType.hpp"

namespace rack_plugin_rcm {

struct BaseWidget;

struct ColourDragging : public ModuleDragType {
  BaseWidget* widget;

	ColourDragging(BaseWidget* widget);
	virtual ~ColourDragging();

	void onDragMove(rack::EventDragMove& e) override;
};

} // namespace rack_plugin_rcm

#include "rack.hpp"

namespace rack_plugin_rcm {

struct ModuleDragType {
	ModuleDragType();
	virtual ~ModuleDragType();

	virtual void onDragMove(rack::EventDragMove& e) = 0;
};

} // namespace rack_plugin_rcm

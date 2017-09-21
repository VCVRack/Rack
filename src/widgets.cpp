#include "widgets.hpp"

namespace rack {

Vec gMousePos;
Widget *gHoveredWidget = NULL;
Widget *gDraggedWidget = NULL;
Widget *gDragHoveredWidget = NULL;
Widget *gFocusedWidget = NULL;
int gGuiFrame;

Scene *gScene = NULL;

} // namespace rack

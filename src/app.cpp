#include "app.hpp"


namespace rack {

std::string gApplicationName = "VCV Rack";
std::string gApplicationVersion = TOSTRING(VERSION);

RackWidget *gRackWidget = NULL;


void sceneInit() {
	gScene = new RackScene();
}

void sceneDestroy() {
	delete gScene;
	gScene = NULL;
}


} // namespace rack

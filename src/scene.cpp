#include "scene.hpp"


namespace rack {

std::string gApplicationName = "VCV Rack";
std::string gApplicationVersion = "v0.1.0 alpha";

RackWidget *gRackWidget = NULL;


void sceneInit() {
	gScene = new RackScene();
}

void sceneDestroy() {
	delete gScene;
	gScene = NULL;
}


} // namespace rack

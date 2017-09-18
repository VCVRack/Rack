#include "app.hpp"


namespace rack {

std::string gApplicationName = "VCV Rack";
std::string gApplicationVersion = TOSTRING(VERSION);
std::string gApiHost = "http://api.vcvrack.com";

RackWidget *gRackWidget = NULL;


void sceneInit() {
	gScene = new RackScene();
}

void sceneDestroy() {
	delete gScene;
	gScene = NULL;
}


} // namespace rack

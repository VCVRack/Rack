#include "app.hpp"


namespace rack {

std::string gApplicationName = "VCV Rack";
std::string gApplicationVersion =
#ifdef VERSION
	TOSTRING(VERSION);
#else
	"";
#endif
std::string gApiHost = "http://api.vcvrack.com";

RackWidget *gRackWidget = NULL;
Toolbar *gToolbar = NULL;
RackScene *gRackScene = NULL;


void sceneInit() {
	gRackScene = new RackScene();
	gScene = gRackScene;
}

void sceneDestroy() {
	delete gScene;
	gScene = NULL;
}


} // namespace rack

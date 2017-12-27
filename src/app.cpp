#include "app.hpp"

#include <ossia/network/network.hpp>
#include <ossia/network/oscquery/oscquery_server.hpp>

namespace rack {

std::string gApplicationName = "VCV Rack";
std::string gApplicationVersion =
#ifdef VERSION
	TOSTRING(VERSION);
#else
	"";
#endif
std::string gApiHost = "https://api.vcvrack.com";
// std::string gApiHost = "http://localhost:8081";

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

ossia::net::generic_device& root_dev(){
    static ossia::net::generic_device dev{
        std::make_unique<ossia::oscquery::oscquery_server_protocol>(1234, 5678),
        "VCV-Rack"};

    return dev;
}

} // namespace rack

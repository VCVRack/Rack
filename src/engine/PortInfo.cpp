#include <engine/PortInfo.hpp>
#include <string.hpp>


namespace rack {
namespace engine {


std::string PortInfo::getName() {
	if (name == "")
		return string::f("#%d", portId + 1);
	return name;
}

std::string PortInfo::getDescription() {
	return description;
}


} // namespace engine
} // namespace rack

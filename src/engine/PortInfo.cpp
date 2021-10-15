#include <engine/PortInfo.hpp>
#include <string.hpp>


namespace rack {
namespace engine {


std::string PortInfo::getName() {
	if (name == "")
		return string::f("#%d", portId + 1);
	return name;
}


std::string PortInfo::getFullName() {
	std::string name = getName();
	name += " ";
	name += (type == Port::INPUT) ? "input" : "output";
	return name;
}


std::string PortInfo::getDescription() {
	return description;
}


} // namespace engine
} // namespace rack

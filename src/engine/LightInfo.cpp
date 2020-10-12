#include <engine/LightInfo.hpp>
#include <string.hpp>


namespace rack {
namespace engine {


std::string LightInfo::getName() {
	return name;
}

std::string LightInfo::getDescription() {
	return description;
}


} // namespace engine
} // namespace rack

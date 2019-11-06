#pragma once
#include <common.hpp>
#include <engine/Port.hpp>


namespace rack {
namespace engine {


struct PortInfo {
	/** The name of the port, using sentence capitalization.
	e.g. "Sine", "Pitch input", "Mode CV"
	*/
	std::string name;
	/** An optional one-sentence description of the parameter. */
	std::string description;
};


} // namespace engine
} // namespace rack

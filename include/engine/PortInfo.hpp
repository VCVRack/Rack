#pragma once
#include <common.hpp>
#include <engine/Port.hpp>


namespace rack {
namespace engine {


struct PortInfo {
	Module* module = NULL;
	Port::Type type;
	int portId;

	/** The name of the port, using sentence capitalization.
	e.g. "Sine", "Pitch input", "Mode CV"
	*/
	std::string name;
	/** An optional one-sentence description of the parameter. */
	std::string description;

	virtual ~PortInfo() {}
	virtual std::string getName() {return name;}
	virtual std::string getDescription() {return description;}
};


} // namespace engine
} // namespace rack

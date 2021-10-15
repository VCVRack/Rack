#pragma once
#include <common.hpp>
#include <engine/Port.hpp>


namespace rack {
namespace engine {


struct Module;


struct PortInfo {
	Module* module = NULL;
	Port::Type type = Port::INPUT;
	int portId = -1;

	/** The name of the port, using sentence capitalization.
	e.g. "Sine", "Pitch input", "Mode CV".

	Don't use the words "input" or "output" in the name.
	Since this text is often prepended or appended to the name, the name will appear as e.g. "Sine input input", "Input: Sine input".
	*/
	std::string name;

	/** An optional one-sentence description of the parameter. */
	std::string description;

	virtual ~PortInfo() {}
	virtual std::string getName();
	/** Returns name with "input" or "output" appended. */
	std::string getFullName();
	virtual std::string getDescription();
};


} // namespace engine
} // namespace rack

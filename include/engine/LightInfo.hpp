#pragma once
#include <common.hpp>


namespace rack {
namespace engine {


struct Module;


struct LightInfo {
	Module* module = NULL;
	int lightId = -1;

	/** The name of the light, using sentence capitalization.
	e.g. "Level", "Oscillator phase", "Mode CV".

	Don't use the word "light" or "LED" in the name.
	Since this text is often prepended or appended to the name, the name will appear as e.g. "Level light light".
	*/
	std::string name;

	/** An optional one-sentence description of the light. */
	std::string description;

	virtual ~LightInfo() {}
	virtual std::string getName();
	virtual std::string getDescription();
};


} // namespace engine
} // namespace rack

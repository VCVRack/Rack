#pragma once
#include <common.hpp>
#include <engine/Module.hpp>
#include <engine/Param.hpp>
#include <color.hpp>


namespace rack {
namespace engine {


/** A weak handle to a Param. Managed by Engine */
struct ParamHandle {
	/** Do not set these directly.
	Use Engine ParamHandle methods.
	*/
	int64_t moduleId = -1;
	int paramId = 0;
	Module* module = NULL;

	std::string text;
	NVGcolor color;
};


} // namespace engine
} // namespace rack

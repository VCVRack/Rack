#pragma once
#include <common.hpp>
#include <engine/Module.hpp>
#include <engine/Param.hpp>


namespace rack {
namespace engine {


/** A weak handle to a Param. Managed by Engine */
struct ParamHandle {
	/** Do not set these directly.
	Use Engine ParamHandle methods.
	*/
	int moduleId = -1;
	int paramId = 0;
	Module *module = NULL;
};


} // namespace engine
} // namespace rack

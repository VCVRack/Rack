#pragma once
#include "common.hpp"
#include "engine/Module.hpp"
#include "engine/Param.hpp"


namespace rack {
namespace engine {


/** A weak handle to a Param. Managed by Engine */
struct ParamHandle {
	/** Do not set these directly.
	They are handled by Engine methods.
	*/
	int moduleId;
	int paramId;
	Module *module;

	ParamHandle() {
		reset();
	}

	void reset() {
		moduleId = -1;
		paramId = 0;
		module = NULL;
	}
};


} // namespace engine
} // namespace rack

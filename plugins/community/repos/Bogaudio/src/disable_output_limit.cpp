
#include "disable_output_limit.hpp"

#define DISABLE_OUTPUT_LIMIT "disableOutputLimit"

json_t* DisableOutputLimitModule::toJson() {
	json_t* root = json_object();
	json_object_set_new(root, DISABLE_OUTPUT_LIMIT, json_boolean(_disableOutputLimit));
	return root;
}

void DisableOutputLimitModule::fromJson(json_t* root) {
	json_t* dol = json_object_get(root, DISABLE_OUTPUT_LIMIT);
	if (dol) {
		_disableOutputLimit = json_is_true(dol);
	}
}

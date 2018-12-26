#include "plugin/Model.hpp"


namespace rack {


void Model::fromJson(json_t *rootJ) {
	json_t *nameJ = json_object_get(rootJ, "name");
	if (nameJ)
		name = json_string_value(nameJ);

	DEBUG("name: %s", name.c_str());

	json_t *tagsJ = json_object_get(rootJ, "tags");
	if (tagsJ) {
		size_t i;
		json_t *tagJ;
		json_array_foreach(tagsJ, i, tagJ) {
			std::string tag = json_string_value(tagJ);
			DEBUG("tag: %s", tag.c_str());
		}
	}
}


} // namespace rack

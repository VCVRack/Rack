#include <plugin/Model.hpp>


namespace rack {
namespace plugin {


void Model::fromJson(json_t *rootJ) {
	json_t *nameJ = json_object_get(rootJ, "name");
	if (nameJ)
		name = json_string_value(nameJ);

	json_t *descriptionJ = json_object_get(rootJ, "description");
	if (descriptionJ)
		description = json_string_value(descriptionJ);

	json_t *tagsJ = json_object_get(rootJ, "tags");
	if (tagsJ) {
		size_t i;
		json_t *tagJ;
		json_array_foreach(tagsJ, i, tagJ) {
			std::string tag = json_string_value(tagJ);
			tags.push_back(tag);
		}
	}
}


} // namespace plugin
} // namespace rack

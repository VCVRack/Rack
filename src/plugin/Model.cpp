#include <plugin/Model.hpp>
#include <plugin.hpp>
#include <asset.hpp>
#include <system.hpp>
#include <string.hpp>
#include <tag.hpp>


namespace rack {
namespace plugin {


void Model::fromJson(json_t* rootJ) {
	assert(plugin);

	json_t* nameJ = json_object_get(rootJ, "name");
	if (nameJ)
		name = json_string_value(nameJ);
	if (name == "")
		throw Exception(string::f("No module name for slug %s", slug.c_str()));

	json_t* descriptionJ = json_object_get(rootJ, "description");
	if (descriptionJ)
		description = json_string_value(descriptionJ);

	// Tags
	tags.clear();
	json_t* tagsJ = json_object_get(rootJ, "tags");
	if (tagsJ) {
		size_t i;
		json_t* tagJ;
		json_array_foreach(tagsJ, i, tagJ) {
			std::string tag = json_string_value(tagJ);
			int tagId = tag::findId(tag);
			if (tagId >= 0)
				tags.push_back(tagId);
		}
	}

	// manualUrl
	json_t* manualUrlJ = json_object_get(rootJ, "manualUrl");
	if (manualUrlJ)
		manualUrl = json_string_value(manualUrlJ);

	// Preset paths
	presetPaths.clear();
	std::string presetDir = asset::plugin(plugin, "presets/" + slug);
	for (const std::string& presetPath : system::getEntries(presetDir)) {
		presetPaths.push_back(presetPath);
	}
}


std::string Model::getFullName() {
	assert(plugin);
	return plugin->getBrand() + " " + name;
}


} // namespace plugin
} // namespace rack

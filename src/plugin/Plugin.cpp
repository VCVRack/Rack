#include "plugin/Plugin.hpp"
#include "plugin/Model.hpp"


namespace rack {
namespace plugin {


Plugin::~Plugin() {
	for (Model *model : models) {
		delete model;
	}
}

void Plugin::addModel(Model *model) {
	assert(!model->plugin);
	model->plugin = this;
	models.push_back(model);
}

Model *Plugin::getModel(std::string slug) {
	for (Model *model : models) {
		if (model->slug == slug) {
			return model;
		}
	}
	return NULL;
}

void Plugin::fromJson(json_t *rootJ) {
	json_t *slugJ = json_object_get(rootJ, "slug");
	if (slugJ)
		slug = json_string_value(slugJ);

	json_t *versionJ = json_object_get(rootJ, "version");
	if (versionJ)
		version = json_string_value(versionJ);

	json_t *nameJ = json_object_get(rootJ, "name");
	if (nameJ)
		name = json_string_value(nameJ);

	json_t *brandJ = json_object_get(rootJ, "brand");
	if (brandJ)
		brand = json_string_value(brandJ);
	// Use name for brand name by default
	if (brand == "")
		brand = name;

	json_t *authorJ = json_object_get(rootJ, "author");
	if (authorJ)
		author = json_string_value(authorJ);

	json_t *licenseJ = json_object_get(rootJ, "license");
	if (licenseJ)
		license = json_string_value(licenseJ);

	json_t *authorEmailJ = json_object_get(rootJ, "authorEmail");
	if (authorEmailJ)
		authorEmail = json_string_value(authorEmailJ);

	json_t *pluginUrlJ = json_object_get(rootJ, "pluginUrl");
	if (pluginUrlJ)
		pluginUrl = json_string_value(pluginUrlJ);

	json_t *authorUrlJ = json_object_get(rootJ, "authorUrl");
	if (authorUrlJ)
		authorUrl = json_string_value(authorUrlJ);

	json_t *manualUrlJ = json_object_get(rootJ, "manualUrl");
	if (manualUrlJ)
		manualUrl = json_string_value(manualUrlJ);

	json_t *sourceUrlJ = json_object_get(rootJ, "sourceUrl");
	if (sourceUrlJ)
		sourceUrl = json_string_value(sourceUrlJ);

	json_t *donateUrlJ = json_object_get(rootJ, "donateUrl");
	if (donateUrlJ)
		donateUrl = json_string_value(donateUrlJ);

	json_t *modulesJ = json_object_get(rootJ, "modules");
	if (modulesJ) {
		size_t moduleId;
		json_t *moduleJ;
		json_array_foreach(modulesJ, moduleId, moduleJ) {
			json_t *modelSlugJ = json_object_get(moduleJ, "slug");
			if (!modelSlugJ)
				continue;
			std::string modelSlug = json_string_value(modelSlugJ);

			Model *model = getModel(modelSlug);
			if (!model) {
				WARN("plugin.json of \"%s\" contains module \"%s\" but it is not defined in the plugin", slug.c_str(), modelSlug.c_str());
				continue;
			}

			model->fromJson(moduleJ);
		}
	}
}


} // namespace plugin
} // namespace rack

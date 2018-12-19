#include "plugin/Plugin.hpp"
#include "plugin/Model.hpp"
#include "logger.hpp"


namespace rack {


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
		const char *slug;
		json_t *moduleJ;
		json_object_foreach(modulesJ, slug, moduleJ) {
			Model *model = getModel(slug);
			if (!model) {
				WARN("Metadata references module \"%s\" but it is not registered in the plugin library", slug);
				continue;
			}

			model->fromJson(moduleJ);
		}
	}
}


} // namespace rack

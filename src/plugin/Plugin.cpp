#include <plugin/Plugin.hpp>
#include <plugin/Model.hpp>
#include <plugin.hpp>
#include <string.hpp>
#include <app/common.hpp>


namespace rack {
namespace plugin {


Plugin::~Plugin() {
	for (Model* model : models) {
		delete model;
	}
}

void Plugin::addModel(Model* model) {
	// Check that the model is not added to a plugin already
	assert(!model->plugin);
	model->plugin = this;
	models.push_back(model);
}

Model* Plugin::getModel(std::string slug) {
	for (Model* model : models) {
		if (model->slug == slug) {
			return model;
		}
	}
	return NULL;
}

void Plugin::fromJson(json_t* rootJ) {
	// Slug
	json_t* slugJ = json_object_get(rootJ, "slug");
	if (slugJ)
		slug = json_string_value(slugJ);
	if (slug == "")
		throw UserException("No plugin slug");
	if (!isSlugValid(slug))
		throw UserException(string::f("Plugin slug \"%s\" is invalid", slug.c_str()));

	// Version
	json_t* versionJ = json_object_get(rootJ, "version");
	if (versionJ)
		version = json_string_value(versionJ);
	if (!string::startsWith(version, app::ABI_VERSION + "."))
		throw UserException(string::f("Plugin version %s does not match Rack ABI version %s", version.c_str(), app::ABI_VERSION.c_str()));
	if (version == "")
		throw UserException("No plugin version");

	// Name
	json_t* nameJ = json_object_get(rootJ, "name");
	if (nameJ)
		name = json_string_value(nameJ);
	if (name == "")
		throw UserException("No plugin name");

	// Brand
	json_t* brandJ = json_object_get(rootJ, "brand");
	if (brandJ)
		brand = json_string_value(brandJ);
	// Use name for brand name by default
	if (brand == "")
		brand = name;

	json_t* authorJ = json_object_get(rootJ, "author");
	if (authorJ)
		author = json_string_value(authorJ);

	json_t* licenseJ = json_object_get(rootJ, "license");
	if (licenseJ)
		license = json_string_value(licenseJ);

	json_t* authorEmailJ = json_object_get(rootJ, "authorEmail");
	if (authorEmailJ)
		authorEmail = json_string_value(authorEmailJ);

	json_t* pluginUrlJ = json_object_get(rootJ, "pluginUrl");
	if (pluginUrlJ)
		pluginUrl = json_string_value(pluginUrlJ);

	json_t* authorUrlJ = json_object_get(rootJ, "authorUrl");
	if (authorUrlJ)
		authorUrl = json_string_value(authorUrlJ);

	json_t* manualUrlJ = json_object_get(rootJ, "manualUrl");
	if (manualUrlJ)
		manualUrl = json_string_value(manualUrlJ);

	json_t* sourceUrlJ = json_object_get(rootJ, "sourceUrl");
	if (sourceUrlJ)
		sourceUrl = json_string_value(sourceUrlJ);

	json_t* donateUrlJ = json_object_get(rootJ, "donateUrl");
	if (donateUrlJ)
		donateUrl = json_string_value(donateUrlJ);

	json_t* modulesJ = json_object_get(rootJ, "modules");
	if (modulesJ) {
		size_t moduleId;
		json_t* moduleJ;
		json_array_foreach(modulesJ, moduleId, moduleJ) {
			// Check if module is disabled
			json_t* disabledJ = json_object_get(moduleJ, "disabled");
			if (disabledJ) {
				if (json_boolean_value(disabledJ))
					continue;
			}

			// Get model slug
			json_t* modelSlugJ = json_object_get(moduleJ, "slug");
			if (!modelSlugJ) {
				throw UserException(string::f("No slug found for module entry %d", moduleId));
			}
			std::string modelSlug = json_string_value(modelSlugJ);

			// Check model slug
			if (!isSlugValid(modelSlug)) {
				throw UserException(string::f("Module slug \"%s\" is invalid", modelSlug.c_str()));
			}

			// Get model
			Model* model = getModel(modelSlug);
			if (!model) {
				throw UserException(string::f("Manifest contains module %s but it is not defined in the plugin", modelSlug.c_str()));
			}

			model->fromJson(moduleJ);
		}
	}

	// Remove models without names
	// This is a hacky way of matching JSON models with C++ models.
	for (auto it = models.begin(); it != models.end();) {
		Model* model = *it;
		if (model->name == "") {
			it = models.erase(it);
			delete model;
			continue;
		}
		it++;
	}
}


} // namespace plugin
} // namespace rack

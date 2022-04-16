#include <plugin/Plugin.hpp>
#include <plugin/Model.hpp>
#include <plugin.hpp>
#include <string.hpp>
#include <app/common.hpp>

#include <algorithm>


namespace rack {
namespace plugin {


Plugin::~Plugin() {
	for (Model* model : models) {
		model->plugin = NULL;
		// Don't delete model because it's allocated once and referenced by a global.
	}
}

void Plugin::addModel(Model* model) {
	// Check that the model is not added to a plugin already
	assert(!model->plugin);
	model->plugin = this;
	models.push_back(model);
}

Model* Plugin::getModel(const std::string& slug) {
	auto it = std::find_if(models.begin(), models.end(), [&](Model* m) {
		return m->slug == slug;
	});
	if (it == models.end())
		return NULL;
	return *it;
}

void Plugin::fromJson(json_t* rootJ) {
	// slug
	json_t* slugJ = json_object_get(rootJ, "slug");
	if (slugJ)
		slug = json_string_value(slugJ);
	if (slug == "")
		throw Exception("No plugin slug");
	if (!isSlugValid(slug))
		throw Exception("Plugin slug \"%s\" is invalid", slug.c_str());

	// version
	json_t* versionJ = json_object_get(rootJ, "version");
	if (versionJ)
		version = json_string_value(versionJ);
	if (!string::startsWith(version, APP_VERSION_MAJOR + "."))
		throw Exception("Plugin version %s does not match Rack ABI version %s", version.c_str(), APP_VERSION_MAJOR.c_str());
	if (version == "")
		throw Exception("No plugin version");

	// name
	json_t* nameJ = json_object_get(rootJ, "name");
	if (nameJ)
		name = json_string_value(nameJ);
	if (name == "")
		throw Exception("No plugin name");

	// brand
	json_t* brandJ = json_object_get(rootJ, "brand");
	if (brandJ)
		brand = json_string_value(brandJ);
	// If brand is not set, fall back to the plugin name
	if (brand == "")
		brand = name;

	json_t* descriptionJ = json_object_get(rootJ, "description");
	if (descriptionJ)
		description = json_string_value(descriptionJ);

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

	json_t* changelogUrlJ = json_object_get(rootJ, "changelogUrl");
	if (changelogUrlJ)
		changelogUrl = json_string_value(changelogUrlJ);
}


void Plugin::modulesFromJson(json_t* rootJ) {
	// Reordered models vector
	std::list<Model*> newModels;

	if (rootJ && json_array_size(rootJ) > 0) {
		size_t moduleId;
		json_t* moduleJ;
		json_array_foreach(rootJ, moduleId, moduleJ) {
			// Get model slug
			json_t* modelSlugJ = json_object_get(moduleJ, "slug");
			if (!modelSlugJ) {
				throw Exception("No slug found for module entry #%d", (int) moduleId);
			}
			std::string modelSlug = json_string_value(modelSlugJ);

			// Check model slug
			if (!isSlugValid(modelSlug)) {
				throw Exception("Module slug \"%s\" is invalid", modelSlug.c_str());
			}

			// Get model
			auto it = std::find_if(models.begin(), models.end(), [&](Model* m) {
				return m->slug == modelSlug;
			});
			if (it == models.end()) {
				throw Exception("Manifest contains module %s but it is not defined in plugin", modelSlug.c_str());
			}

			Model* model = *it;
			models.erase(it);
			newModels.push_back(model);

			model->fromJson(moduleJ);
		}
	}
	else {
		WARN("No modules in plugin manifest %s", slug.c_str());
	}

	if (!models.empty()) {
		std::vector<std::string> slugs;
		for (Model* model : models) {
			slugs.push_back(model->slug);
			delete model;
		}
		throw Exception("Plugin defines module %s but it is not defined in manifest", string::join(slugs, ", ").c_str());
	}

	models = newModels;

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


std::string Plugin::getBrand() {
	if (brand == "")
		return name;
	return brand;
}


} // namespace plugin
} // namespace rack

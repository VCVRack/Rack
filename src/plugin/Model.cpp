#include <algorithm>

#include <plugin/Model.hpp>
#include <plugin.hpp>
#include <asset.hpp>
#include <system.hpp>
#include <settings.hpp>
#include <string.hpp>
#include <tag.hpp>
#include <ui/Menu.hpp>
#include <ui/MenuSeparator.hpp>
#include <helpers.hpp>
#include <window/Window.hpp>


namespace rack {
namespace plugin {


void Model::fromJson(json_t* rootJ) {
	assert(plugin);

	json_t* nameJ = json_object_get(rootJ, "name");
	if (nameJ)
		name = json_string_value(nameJ);
	if (name == "")
		throw Exception("No module name for slug %s", slug.c_str());

	json_t* descriptionJ = json_object_get(rootJ, "description");
	if (descriptionJ)
		description = json_string_value(descriptionJ);

	// Tags
	tagIds.clear();
	json_t* tagsJ = json_object_get(rootJ, "tags");
	if (tagsJ) {
		size_t i;
		json_t* tagJ;
		json_array_foreach(tagsJ, i, tagJ) {
			std::string tag = json_string_value(tagJ);
			int tagId = tag::findId(tag);

			// Omit duplicates
			auto it = std::find(tagIds.begin(), tagIds.end(), tagId);
			if (it != tagIds.end())
				continue;

			if (tagId >= 0)
				tagIds.push_back(tagId);
		}
	}

	// manualUrl
	json_t* manualUrlJ = json_object_get(rootJ, "manualUrl");
	if (manualUrlJ)
		manualUrl = json_string_value(manualUrlJ);

	// modularGridUrl
	json_t* modularGridUrlJ = json_object_get(rootJ, "modularGridUrl");
	if (modularGridUrlJ)
		modularGridUrl = json_string_value(modularGridUrlJ);

	// hidden
	json_t* hiddenJ = json_object_get(rootJ, "hidden");
	// "disabled" was a deprecated alias in Rack <2
	if (!hiddenJ)
		hiddenJ = json_object_get(rootJ, "disabled");
	// "deprecated" was a deprecated alias in Rack <2.2.4
	if (!hiddenJ)
		hiddenJ = json_object_get(rootJ, "deprecated");
	if (hiddenJ) {
		// Don't un-hide Model if already hidden by C++
		if (json_boolean_value(hiddenJ))
			hidden = true;
	}
}


std::string Model::getFullName() {
	assert(plugin);
	return plugin->getBrand() + " " + name;
}


std::string Model::getFactoryPresetDirectory() {
	return asset::plugin(plugin, system::join("presets", slug));
}


std::string Model::getUserPresetDirectory() {
	return asset::user(system::join("presets", plugin->slug, slug));
}


std::string Model::getManualUrl() {
	if (!manualUrl.empty())
		return manualUrl;
	return plugin->manualUrl;
}


void Model::appendContextMenu(ui::Menu* menu, bool inBrowser) {
	// plugin
	menu->addChild(createMenuItem("Plugin: " + plugin->name, "", [=]() {
		system::openBrowser(plugin->pluginUrl);
	}, plugin->pluginUrl == ""));

	// version
	menu->addChild(createMenuLabel("Version: " + plugin->version));

	// author
	if (plugin->author != "") {
		menu->addChild(createMenuItem("Author: " + plugin->author, "", [=]() {
			system::openBrowser(plugin->authorUrl);
		}, plugin->authorUrl.empty()));
	}

	// license
	std::string license = plugin->license;
	if (string::startsWith(license, "https://") || string::startsWith(license, "http://")) {
		menu->addChild(createMenuItem("License: Open in browser", "", [=]() {
			system::openBrowser(license);
		}));
	}
	else if (license != "") {
		menu->addChild(createMenuLabel("License: " + license));
	}

	// tags
	if (!tagIds.empty()) {
		menu->addChild(createMenuLabel("Tags:"));
		for (int tagId : tagIds) {
			menu->addChild(createMenuLabel("• " + tag::getTag(tagId)));
		}
	}

	menu->addChild(new ui::MenuSeparator);

	// VCV Library page
	menu->addChild(createMenuItem("VCV Library page", "", [=]() {
		system::openBrowser("https://library.vcvrack.com/" + plugin->slug + "/" + slug);
	}));

	// modularGridUrl
	if (modularGridUrl != "") {
		menu->addChild(createMenuItem("ModularGrid page", "", [=]() {
			system::openBrowser(modularGridUrl);
		}));
	}

	// manual
	std::string manualUrl = getManualUrl();
	if (manualUrl != "") {
		menu->addChild(createMenuItem("User manual", RACK_MOD_CTRL_NAME "+F1", [=]() {
			system::openBrowser(manualUrl);
		}));
	}

	// donate
	if (plugin->donateUrl != "") {
		menu->addChild(createMenuItem("Donate", "", [=]() {
			system::openBrowser(plugin->donateUrl);
		}));
	}

	// source code
	if (plugin->sourceUrl != "") {
		menu->addChild(createMenuItem("Source code", "", [=]() {
			system::openBrowser(plugin->sourceUrl);
		}));
	}

	// changelog
	if (plugin->changelogUrl != "") {
		menu->addChild(createMenuItem("Changelog", "", [=]() {
			system::openBrowser(plugin->changelogUrl);
		}));
	}

	// author email
	if (plugin->authorEmail != "") {
		menu->addChild(createMenuItem("Author email", "Copy to clipboard", [=]() {
			glfwSetClipboardString(APP->window->win, plugin->authorEmail.c_str());
		}));
	}

	// plugin folder
	if (plugin->path != "") {
		menu->addChild(createMenuItem("Open plugin folder", "", [=]() {
			system::openDirectory(plugin->path);
		}));
	}

	// Favorite
	std::string favoriteRightText = inBrowser ? (RACK_MOD_CTRL_NAME "+click") : "";
	if (isFavorite())
		favoriteRightText += " " CHECKMARK_STRING;
	menu->addChild(createMenuItem("Favorite", favoriteRightText,
		[=]() {
			setFavorite(!isFavorite());
		}
	));
}


bool Model::isFavorite() {
	const settings::ModuleInfo* mi = settings::getModuleInfo(plugin->slug, slug);
	return mi && mi->favorite;
}


void Model::setFavorite(bool favorite) {
	settings::ModuleInfo& mi = settings::moduleInfos[plugin->slug][slug];
	mi.favorite = favorite;
}


} // namespace plugin
} // namespace rack

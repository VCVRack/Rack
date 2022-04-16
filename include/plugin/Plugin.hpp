#pragma once
#include <common.hpp>

#include <jansson.h>

#include <list>


namespace rack {
namespace plugin {


struct Model;


// Subclass this and return a pointer to a new one when init() is called
struct Plugin {
	/** List of models contained in this plugin.
	Add with addModel().
	*/
	std::list<Model*> models;
	/** The file path to the plugin's directory.
	*/
	std::string path;
	/** OS-dependent library handle.
	*/
	void* handle = NULL;

	/** Must be unique. Used for saving patches. Never change this after releasing your plugin.
	To guarantee uniqueness, it is a good idea to prefix the slug by your "company name" if available, e.g. "MyCompany-MyPlugin"
	*/
	std::string slug;
	/** Your plugin's latest version.
	Do not include the "v" prefix.
	*/
	std::string version;
	/** The license type of your plugin. Use "proprietary" if all rights are reserved. If your license is in the [SPDX license list](https://spdx.org/licenses/), use its abbreviation in the "Identifier" column.
	*/
	std::string license;
	/** Human-readable display name for your plugin. You can change this on a whim, unlike slugs.
	*/
	std::string name;
	/** Prefix of each module name in the Module Browser.
	If blank, `name` is used.
	*/
	std::string brand;
	/** A one-line summary of the plugin's purpose.
	If your plugin doesn't follow a theme, it’s probably best to omit this.
	*/
	std::string description;
	/** Your name, company, alias, or GitHub username.
	*/
	std::string author;
	/** Your email address for support inquiries.
	*/
	std::string authorEmail;
	/** Homepage of the author.
	*/
	std::string authorUrl;
	/** Homepage featuring the plugin itself.
	*/
	std::string pluginUrl;
	/** The manual of your plugin. HTML, PDF, or GitHub readme/wiki are fine.
	*/
	std::string manualUrl;
	/** The source code homepage. E.g. GitHub repo.
	*/
	std::string sourceUrl;
	/** Link to donation page for users who wish to donate. E.g. PayPal URL.
	*/
	std::string donateUrl;
	/** Link to the changelog of the plugin.
	*/
	std::string changelogUrl;
	/** Last modified timestamp of the plugin directory.
	*/
	double modifiedTimestamp = -INFINITY;

	~Plugin();
	void addModel(Model* model);
	Model* getModel(const std::string& slug);
	void fromJson(json_t* rootJ);
	void modulesFromJson(json_t* rootJ);
	std::string getBrand();
};


} // namespace plugin
} // namespace rack

#pragma once
#include <jansson.h>

#include <common.hpp>


namespace rack {
/** Handles the Rack patch file state */
namespace patch {


/** Handles the Rack patch file state. */
struct Manager {
	struct Internal;
	Internal* internal;

	/** The currently loaded patch file path */
	std::string path;
	/** Path to autosave dir */
	std::string autosavePath;
	/** Path to user template patch */
	std::string templatePath;
	/** Path to factory template patch */
	std::string factoryTemplatePath;
	/** Append to this while loading/saving a patch to display messages to the user after success. */
	std::string warningLog;

	PRIVATE Manager();
	PRIVATE ~Manager();
	PRIVATE void launch(std::string pathArg);
	/** Clears the patch. */
	void clear();
	/** Saves the patch and nothing else. */
	void save(std::string path);
	void saveDialog();
	void saveAsDialog(bool setPath = true);
	void saveTemplateDialog();
	void saveAutosave();
	/** Delete and re-create autosave dir. */
	void clearAutosave();
	/** Clean up nonexistent module patch storage dirs in autosave dir. */
	void cleanAutosave();
	/** Loads a patch and nothing else.
	Returns whether the patch was loaded successfully.
	*/
	void load(std::string path);
	/** Loads the template patch. */
	void loadTemplate();
	void loadTemplateDialog();
	bool hasAutosave();
	void loadAutosave();
	/** Loads a patch, sets the current path, and updates the recent patches. */
	void loadAction(std::string path);
	void loadDialog();
	void loadPathDialog(std::string path);
	/** Asks the user to reload the current patch. */
	void revertDialog();
	void pushRecentPath(std::string path);
	/** Disconnects all cables. */
	void disconnectDialog();

	json_t* toJson();
	void fromJson(json_t* rootJ);
	void log(std::string msg);
};


} // namespace patch
} // namespace rack

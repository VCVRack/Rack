#pragma once
#include <common.hpp>
#include <jansson.h>


namespace rack {


struct PatchManager {
	/** The currently loaded patch file path */
	std::string path;
	/** Enables certain compatibility behavior based on the value */
	int legacy;
	std::string warningLog;

	PatchManager();
	~PatchManager();
	/** Resets the history and scroll position to prepare for a new patch. */
	void reset();
	/** Clears the patch. */
	void clear();
	/** Saves the patch and nothing else. */
	void save(std::string path);
	void saveDialog();
	void saveAsDialog();
	void saveTemplateDialog();
	void saveAutosave();
	/** Loads a patch and nothing else.
	Returns whether the patch was loaded successfully.
	*/
	bool load(std::string path);
	/** Loads the template patch. */
	void loadTemplate();
	void loadTemplateDialog();
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
	bool isLegacy(int level);
	void log(std::string msg);
};


} // namespace rack

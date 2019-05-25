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
	void init(std::string path);
	void reset();
	void resetDialog();
	void save(std::string path);
	void saveDialog();
	void saveAsDialog();
	void saveTemplateDialog();
	bool load(std::string path);
	void loadDialog();
	/** If `lastPath` is defined, ask the user to reload it */
	void revertDialog();
	/** Disconnects all cables */
	void disconnectDialog();

	json_t *toJson();
	void fromJson(json_t *rootJ);
	bool isLegacy(int level);
};


} // namespace rack

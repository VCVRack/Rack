#include "GVerbWidget.hpp"

#include <sys/stat.h>

#if ARCH_WIN
	#include <windows.h>
	#include <direct.h>
	#define mkdir(_dir, _perms) _mkdir(_dir)
#else
	#include <dlfcn.h>
#endif

using namespace std;

Plugin *plugin;

void init(Plugin *p) {
	plugin = p;
	p->slug = TOSTRING(SLUG);
	p->version = TOSTRING(VERSION);

	// Add all Models defined throughout the plugin
	p->addModel(modelGVerbModule);
	p->addModel(modelAudioInterface16);
	p->addModel(modelCV0to10Module);
	p->addModel(modelCVS0to10Module);
	p->addModel(modelCV5to5Module);
	p->addModel(modelCVMmtModule);
	p->addModel(modelCVTglModule);
	p->addModel(modelPianoRollModule);
	//p->addModel(modelSongRollModule);
	p->addModel(modelDuckModule);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}

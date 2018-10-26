#include "global_pre.hpp"
#include "plugin.hpp"
#include "Core/Core.hpp"
#include "app.hpp"
#include "asset.hpp"
#include "util/request.hpp"
#include "osdialog.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

#ifdef YAC_POSIX
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h> // for MAXPATHLEN
#include <fcntl.h>
#include <dirent.h>
#else
#include "util/dirent_win32/dirent.h"
#endif // YAC_POSIX

#include <thread>
#include <stdexcept>

#include "global.hpp"
#include "global_ui.hpp"

#ifndef USE_VST2
#define ZIP_STATIC
#include <zip.h>
#endif // USE_VST2
#include <jansson.h>

#if ARCH_WIN
	#include <windows.h>
	#include <direct.h>
	#define mkdir(_dir, _perms) _mkdir(_dir)
#else
	#include <dlfcn.h>
#endif


namespace rack {

extern Plugin *pluginGetPlugin (std::string pluginSlug);

#ifdef USE_VST2
#ifndef RACK_PLUGIN
#ifndef SKIP_STATIC_MODULES
extern "C" {
extern void init_plugin_21kHz                (rack::Plugin *p);
extern void init_plugin_AmalgamatedHarmonics (rack::Plugin *p);
extern void init_plugin_Alikins              (rack::Plugin *p);
extern void init_plugin_alto777_LFSR         (rack::Plugin *p);
extern void init_plugin_AS                   (rack::Plugin *p);
extern void init_plugin_AudibleInstruments   (rack::Plugin *p);
extern void init_plugin_Autodafe             (rack::Plugin *p);
extern void init_plugin_BaconMusic           (rack::Plugin *p);
extern void init_plugin_Befaco               (rack::Plugin *p);
extern void init_plugin_Bidoo                (rack::Plugin *p);
extern void init_plugin_Bogaudio             (rack::Plugin *p);
// extern void init_plugin_bsp                  (rack::Plugin *p);  // contains GPLv3 code from Ob-Xd (Obxd_VCF)
// extern void init_plugin_BOKONTEPByteBeatMachine (rack::Plugin *p);  // unstable
extern void init_plugin_CastleRocktronics    (rack::Plugin *p);
extern void init_plugin_cf                   (rack::Plugin *p);
extern void init_plugin_com_soundchasing_stochasm (rack::Plugin *p);
extern void init_plugin_computerscare        (rack::Plugin *p);
//extern void init_plugin_dBiz                 (rack::Plugin *p);  // now a DLL (13Jul2018)
extern void init_plugin_DHE_Modules          (rack::Plugin *p);
extern void init_plugin_DrumKit              (rack::Plugin *p);
extern void init_plugin_ErraticInstruments   (rack::Plugin *p);
extern void init_plugin_ESeries              (rack::Plugin *p);
extern void init_plugin_FrankBussFormula     (rack::Plugin *p);
extern void init_plugin_FrozenWasteland      (rack::Plugin *p);
extern void init_plugin_Fundamental          (rack::Plugin *p);
extern void init_plugin_Geodesics            (rack::Plugin *p);
extern void init_plugin_Gratrix              (rack::Plugin *p);
extern void init_plugin_HetrickCV            (rack::Plugin *p);
extern void init_plugin_huaba                (rack::Plugin *p);
extern void init_plugin_ImpromptuModular     (rack::Plugin *p);
extern void init_plugin_JE                   (rack::Plugin *p);
extern void init_plugin_JW_Modules           (rack::Plugin *p);
extern void init_plugin_Koralfx              (rack::Plugin *p);
extern void init_plugin_LindenbergResearch   (rack::Plugin *p);
extern void init_plugin_LOGinstruments       (rack::Plugin *p);
extern void init_plugin_mental               (rack::Plugin *p);
extern void init_plugin_ML_modules           (rack::Plugin *p);
extern void init_plugin_moDllz               (rack::Plugin *p);
extern void init_plugin_modular80            (rack::Plugin *p);
extern void init_plugin_mscHack              (rack::Plugin *p);
extern void init_plugin_mtsch_plugins        (rack::Plugin *p);
extern void init_plugin_NauModular           (rack::Plugin *p);
extern void init_plugin_Nohmad               (rack::Plugin *p);
extern void init_plugin_Ohmer                (rack::Plugin *p);
// extern void init_plugin_ParableInstruments   (rack::Plugin *p); // alternative "Clouds" module (crashes)
extern void init_plugin_PG_Instruments       (rack::Plugin *p);
extern void init_plugin_PvC                  (rack::Plugin *p);
extern void init_plugin_Qwelk                (rack::Plugin *p);
extern void init_plugin_RJModules            (rack::Plugin *p);
extern void init_plugin_SerialRacker         (rack::Plugin *p);
extern void init_plugin_SonusModular         (rack::Plugin *p);
extern void init_plugin_Southpole            (rack::Plugin *p);
extern void init_plugin_Southpole_parasites  (rack::Plugin *p);
extern void init_plugin_squinkylabs_plug1    (rack::Plugin *p);
extern void init_plugin_SubmarineFree        (rack::Plugin *p);
extern void init_plugin_SynthKit             (rack::Plugin *p);
extern void init_plugin_Template             (rack::Plugin *p);
extern void init_plugin_TheXOR               (rack::Plugin *p);
extern void init_plugin_trowaSoft            (rack::Plugin *p);
extern void init_plugin_unless_modules       (rack::Plugin *p);
extern void init_plugin_Valley               (rack::Plugin *p);
// extern void init_plugin_VultModules          (rack::Plugin *p);
}

static void vst2_load_static_rack_plugin(const char *_name, InitCallback _initCallback) {

   std::string path = assetStaticPlugin(_name);
   
	// Construct and initialize Plugin instance
	Plugin *plugin = new Plugin();
	plugin->path = path;
	plugin->handle = NULL;
	_initCallback(plugin);

#if 0
	// Reject plugin if slug already exists
	Plugin *oldPlugin = pluginGetPlugin(plugin->slug);
	if (oldPlugin) {
		warn("Plugin \"%s\" is already loaded, not attempting to load it again", plugin->slug.c_str());
		// TODO
		// Fix memory leak with `plugin` here
		return false;
	}
#endif

	// Add plugin to list
	global->plugin.gPlugins.push_back(plugin);
	info("vcvrack: Loaded static plugin %s %s", plugin->slug.c_str(), plugin->version.c_str());
}
#endif // SKIP_STATIC_MODULES

void vst2_load_static_rack_plugins(void) {
#ifndef SKIP_STATIC_MODULES
   printf("[dbg] vst2_load_static_rack_plugins: ENTER\n");
#if 1
   vst2_load_static_rack_plugin("21kHz",                &init_plugin_21kHz);
   vst2_load_static_rack_plugin("AmalgamatedHarmonics", &init_plugin_AmalgamatedHarmonics);
   vst2_load_static_rack_plugin("Alikins",              &init_plugin_Alikins);
   vst2_load_static_rack_plugin("alto777_LFSR",         &init_plugin_alto777_LFSR);
   vst2_load_static_rack_plugin("AS",                   &init_plugin_AS);
   vst2_load_static_rack_plugin("AudibleInstruments",   &init_plugin_AudibleInstruments);
   vst2_load_static_rack_plugin("Autodafe",             &init_plugin_Autodafe);
   vst2_load_static_rack_plugin("BaconMusic",           &init_plugin_BaconMusic);
   vst2_load_static_rack_plugin("Befaco",               &init_plugin_Befaco);
   vst2_load_static_rack_plugin("Bidoo",                &init_plugin_Bidoo);
   vst2_load_static_rack_plugin("Bogaudio",             &init_plugin_Bogaudio);
   // vst2_load_static_rack_plugin("bsp",                  &init_plugin_bsp);  // contains GPLv3 code from Ob-Xd (Obxd_VCF)
   // vst2_load_static_rack_plugin("BOKONTEPByteBeatMachine",   &init_plugin_BOKONTEPByteBeatMachine);
   vst2_load_static_rack_plugin("CastleRocktronics",    &init_plugin_CastleRocktronics);
   vst2_load_static_rack_plugin("cf",                   &init_plugin_cf);
   vst2_load_static_rack_plugin("com_soundchasing_stochasm", &init_plugin_com_soundchasing_stochasm);
   vst2_load_static_rack_plugin("computerscare",        &init_plugin_computerscare);
   // vst2_load_static_rack_plugin("dBiz",                 &init_plugin_dBiz);  // now a DLL (13Jul2018)
   vst2_load_static_rack_plugin("DHE-Modules",          &init_plugin_DHE_Modules);
   vst2_load_static_rack_plugin("DrumKit",              &init_plugin_DrumKit);
   vst2_load_static_rack_plugin("ErraticInstruments",   &init_plugin_ErraticInstruments);
   vst2_load_static_rack_plugin("ESeries",              &init_plugin_ESeries);
   vst2_load_static_rack_plugin("FrankBussFormula",     &init_plugin_FrankBussFormula);
   vst2_load_static_rack_plugin("FrozenWasteland",      &init_plugin_FrozenWasteland);
   vst2_load_static_rack_plugin("Fundamental",          &init_plugin_Fundamental);
   vst2_load_static_rack_plugin("Geodesics",            &init_plugin_Geodesics);
   vst2_load_static_rack_plugin("Gratrix",              &init_plugin_Gratrix);
   vst2_load_static_rack_plugin("HetrickCV",            &init_plugin_HetrickCV);
   vst2_load_static_rack_plugin("huaba",                &init_plugin_huaba);
   vst2_load_static_rack_plugin("ImpromptuModular",     &init_plugin_ImpromptuModular);
   vst2_load_static_rack_plugin("JE",                   &init_plugin_JE);
   vst2_load_static_rack_plugin("JW_Modules",           &init_plugin_JW_Modules);
   vst2_load_static_rack_plugin("Koralfx-Modules",      &init_plugin_Koralfx);
   vst2_load_static_rack_plugin("LindenbergResearch",   &init_plugin_LindenbergResearch);
   vst2_load_static_rack_plugin("LOGinstruments",       &init_plugin_LOGinstruments);
   vst2_load_static_rack_plugin("mental",               &init_plugin_mental);
   vst2_load_static_rack_plugin("ML_modules",           &init_plugin_ML_modules);
   vst2_load_static_rack_plugin("moDllz",               &init_plugin_moDllz);
   vst2_load_static_rack_plugin("modular80",            &init_plugin_modular80);
   vst2_load_static_rack_plugin("mscHack",              &init_plugin_mscHack);
   vst2_load_static_rack_plugin("mtsch_plugins",        &init_plugin_mtsch_plugins);
   vst2_load_static_rack_plugin("NauModular",           &init_plugin_NauModular);
   vst2_load_static_rack_plugin("Nohmad",               &init_plugin_Nohmad);
   vst2_load_static_rack_plugin("Ohmer",                &init_plugin_Ohmer);
   // vst2_load_static_rack_plugin("ParableInstruments",   &init_plugin_ParableInstruments);
   vst2_load_static_rack_plugin("PG_Instruments",       &init_plugin_PG_Instruments);
   vst2_load_static_rack_plugin("PvC",                  &init_plugin_PvC);
   vst2_load_static_rack_plugin("Qwelk",                &init_plugin_Qwelk);
   vst2_load_static_rack_plugin("RJModules",            &init_plugin_RJModules);
   vst2_load_static_rack_plugin("SerialRacker",         &init_plugin_SerialRacker);
   vst2_load_static_rack_plugin("SonusModular",         &init_plugin_SonusModular);
   vst2_load_static_rack_plugin("Southpole",            &init_plugin_Southpole);
   vst2_load_static_rack_plugin("Southpole_parasites",  &init_plugin_Southpole_parasites);
   vst2_load_static_rack_plugin("squinkylabs-plug1",    &init_plugin_squinkylabs_plug1);
   vst2_load_static_rack_plugin("SubmarineFree",        &init_plugin_SubmarineFree);
   vst2_load_static_rack_plugin("SynthKit",             &init_plugin_SynthKit);
   vst2_load_static_rack_plugin("Template",             &init_plugin_Template);
   vst2_load_static_rack_plugin("TheXOR",               &init_plugin_TheXOR);
   vst2_load_static_rack_plugin("trowaSoft",            &init_plugin_trowaSoft);
   vst2_load_static_rack_plugin("unless_modules",       &init_plugin_unless_modules);
   vst2_load_static_rack_plugin("Valley",               &init_plugin_Valley);
   // vst2_load_static_rack_plugin("VultModules",          &init_plugin_VultModules);
#else
   vst2_load_static_rack_plugin("Template",             &init_plugin_Template);
#endif
   printf("[dbg] vst2_load_static_rack_plugins: LEAVE\n");
#endif // SKIP_STATIC_MODULES
}
#endif // RACK_PLUGIN
#endif // USE_VST2
}

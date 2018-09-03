#pragma once


#include <map>
#include <list>
#include <vector>
#include <string>
#include <mutex>
#include <thread>

#include "util/common.hpp"  // VIPMutex


#ifdef USE_VST2
class VSTPluginWrapper;
#endif // USE_VST2


namespace rack {

struct Module;
struct Wire;
struct KeyboardDriver;
struct MidiDriver;
struct Plugin;

#ifdef USE_VST2
struct VSTMidiInputDevice;

#define VST2_MAX_UNIQUE_PARAM_IDS  (9999)
#endif // USE_VST2

//
// the structure fields reflect the original file locations
//  (e.g. 'window' was originally located in 'window.cpp')
//

struct VST2QueuedParam {
   int   unique_id;
   float value;
   bool  b_normalized;
};

struct Global {
   bool gPaused;
   /** Plugins should not manipulate other modules or wires unless that is the entire purpose of the module.
    Your plugin needs to have a clear purpose for manipulating other modules and wires and must be done with a good UX.
   */
   std::vector<Module*> gModules;
   std::vector<Wire*> gWires;
   bool gPowerMeter;

   struct {
      bool running;
      float sampleRate;
      float sampleTime;
      
      std::mutex mutex;
      std::thread thread;
      VIPMutex vipMutex;
      
      // Parameter interpolation
      Module *smoothModule;
      int smoothParamId;
      float smoothValue;

   } engine;

   struct {
      std::string globalDir;
      std::string localDir;
   } asset;

   struct {
      KeyboardDriver *driver;
   } keyboard;

   struct {
      std::vector<int> driverIds;
      std::map<int, MidiDriver*> drivers;
   } midi;

   struct {
      std::list<Plugin*> gPlugins;
      std::string gToken;

      bool isDownloading;
      float downloadProgress;
      std::string downloadName;
      std::string loginStatus;
   } plugin;

   struct {
      bool gSkipAutosaveOnLaunch;
   } settings;

   struct {
      FILE *logFile;
      std::chrono::high_resolution_clock::time_point startTime;
   } logger;

   struct {
      uint64_t xoroshiro128plus_state[2] = {};
   } random;

   // struct {
   // } plugins;

#ifdef USE_VST2
   struct {
      int last_seen_instance_count;
      const char *program_dir;

      float *const*inputs;
      float **outputs;
      unsigned int frame_idx;
      unsigned int last_seen_num_frames;

      VSTPluginWrapper *wrapper;
      VSTMidiInputDevice *midi_device;

      int next_unique_param_base_id;
      std::vector<VST2QueuedParam> queued_params;

      bool b_patch_loading;
   } vst2;
#endif // USE_VST2

   void init(void) {
      gPaused = false;
      gPowerMeter = false;

      engine.running = false;
      engine.sampleRate = 44100.0f;
      engine.smoothModule = NULL;

      keyboard.driver = NULL;

      plugin.isDownloading = false;
      plugin.downloadProgress = 0.0;

      settings.gSkipAutosaveOnLaunch = false;

      logger.logFile = NULL;

      random.xoroshiro128plus_state[0] = 0;
      random.xoroshiro128plus_state[1] = 0;

#ifdef USE_VST2
      vst2.midi_device = NULL;
      vst2.next_unique_param_base_id = 1;
      vst2.b_patch_loading = false;
#endif
   } 

};


} // namespace rack

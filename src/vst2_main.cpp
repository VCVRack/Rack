#ifdef USE_VST2
/// vst2_main.cpp
///
/// (c) 2018 bsp. very loosely based on pongasoft's "hello, world" example plugin.
///
///   Licensed under the Apache License, Version 2.0 (the "License");
///   you may not use this file except in compliance with the License.
///   You may obtain a copy of the License at
///
///       http://www.apache.org/licenses/LICENSE-2.0
///
///   Unless required by applicable law or agreed to in writing, software
///   distributed under the License is distributed on an "AS IS" BASIS,
///   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
///   See the License for the specific language governing permissions and
///   limitations under the License.
///
/// created: 25Jun2018
/// changed: 26Jun2018, 27Jun2018, 29Jun2018, 01Jul2018, 02Jul2018, 06Jul2018, 13Jul2018
///          26Jul2018, 04Aug2018, 05Aug2018
///
///
///

// #define DEBUG_PRINT_EVENTS  defined
// #define DEBUG_PRINT_PARAMS  defined

#define NUM_INPUTS    (   8)  // must match AudioInterface.cpp:AUDIO_INPUTS
#define NUM_OUTPUTS   (   8)  // must match AudioInterface.cpp:AUDIO_OUTPUTS

// (note) causes reason to shut down when console is freed (when plugin is deleted)
//#define USE_CONSOLE  defined

#undef RACK_HOST

#include <aeffect.h>
#include <aeffectx.h>
#include <stdio.h>

#include "../dep/yac/yac.h"
#include "../dep/yac/yac_host.cpp"
YAC_Host *yac_host;  // not actually used, just to satisfy the linker

#include "global_pre.hpp"
#include "global.hpp"
#include "global_ui.hpp"

#define EDITWIN_X 0
#define EDITWIN_Y 0
#define EDITWIN_W 1200
#define EDITWIN_H 800

extern int  vst2_init (int argc, char* argv[]);
extern void vst2_exit (void);
namespace rack {
extern void vst2_editor_redraw (void);
}
extern void vst2_set_samplerate (sF32 _rate);
extern void vst2_engine_process (float *const*_in, float **_out, unsigned int _numFrames);
extern void vst2_process_midi_input_event (sU8 _a, sU8 _b, sU8 _c);
extern void vst2_queue_param (int uniqueParamId, float normValue);
extern void vst2_handle_queued_params (void);
extern float vst2_get_param (int uniqueParamId);
extern void  vst2_get_param_name (int uniqueParamId, char *s, int sMaxLen);
extern void vst2_set_shared_plugin_tls_globals (void);

namespace rack {
   extern void settingsLoad(std::string filename, bool bWindowSizeOnly);
}


#include "../include/window.hpp"
#include "../dep/include/osdialog.h"
#include "../include/app.hpp"

// using namespace rack;
// extern void rack::windowRun(void);

#if defined(_WIN32) || defined(_WIN64)
#define HAVE_WINDOWS defined

#define WIN32_LEAN_AND_MEAN defined
#include <windows.h>
#include <xmmintrin.h>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;


// Windows:
#define VST_EXPORT  extern "C" __declspec(dllexport)


struct PluginMutex {
   CRITICAL_SECTION handle; 

   PluginMutex(void) {
      ::InitializeCriticalSection( &handle ); 
   }

   ~PluginMutex() {
      ::DeleteCriticalSection( &handle ); 
   }

   void lock(void) {
      ::EnterCriticalSection(&handle); 
   }

   void unlock(void) {
      ::LeaveCriticalSection(&handle); 
   }
};

#else

// MacOSX, Linux:
#define HAVE_UNIX defined

#define VST_EXPORT extern

#include <pthread.h> 
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

//static pthread_mutex_t loc_pthread_mutex_t_init = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static pthread_mutex_t loc_pthread_mutex_t_init = PTHREAD_MUTEX_INITIALIZER;

struct PluginMutex {
   pthread_mutex_t handle;

   PluginMutex(void) {
      ::memcpy((void*)&handle, (const void*)&loc_pthread_mutex_t_init, sizeof(pthread_mutex_t));
   }

   ~PluginMutex() {
   }

   void lock(void) {
		::pthread_mutex_lock(&handle); 
   }

   void unlock(void) {
		::pthread_mutex_unlock(&handle); 
   }
};

#endif // _WIN32||_WIN64


// // extern "C" {
// // extern void glfwSetInstance(void *_glfw);
// // }



class PluginString : public YAC_String {
public:
   static const sUI QUOT2 =(sUI)(1<<26); // \'\'
   static const sUI STRFLQMASK  = (QUOT | UTAG1 | QUOT2);

   void           safeFreeChars      (void);
   sSI          _realloc             (sSI _numChars);
   sSI           lastIndexOf         (sChar _c, sUI _start) const;
   void          getDirName          (PluginString *_r) const;
   void          replace             (sChar _c, sChar _o);
};

void PluginString::safeFreeChars(void) {
   if(bflags & PluginString::DEL)
   {
      // if(!(bflags & PluginString::LA))
      {
         Dyacfreechars(chars);
      }
   }
}

sSI PluginString::_realloc(sSI _numBytes) { 

   // Force alloc if a very big string is about to shrink a lot or there is simply not enough space available
   if( ((buflen >= 1024) && ( (((sUI)_numBytes)<<3) < buflen )) || 
       (NULL == chars) || 
       (buflen < ((sUI)_numBytes))
       ) // xxx (!chars) hack added 180702 
   {
      if(NULL != chars) 
      { 
         sUI l = length; 

         if(((sUI)_numBytes) < l)
         {
            l = _numBytes;
         }

         sU8 *nc = Dyacallocchars(_numBytes + 1);
         sUI i = 0;

         for(; i<l; i++)
         {
            nc[i] = chars[i];
         }

         nc[i] = 0; 
 
         safeFreeChars();
		   buflen = (_numBytes + 1);
         bflags = PluginString::DEL | (bflags & PluginString::STRFLQMASK); // keep old stringflags
         length = i + 1;
         chars  = nc;
         key    = YAC_LOSTKEY;

         return YAC_TRUE;
      } 
      else 
      {
		   return PluginString::alloc(_numBytes + 1);
      }
   } 
	else 
	{ 
      key = YAC_LOSTKEY; // new 010208

		return YAC_TRUE;
	} 
}

sSI PluginString::lastIndexOf(sChar _c, sUI _start) const {
   sSI li = -1;

   if(NULL != chars)
   {
      sUI i = _start;

      for(; i<length; i++)
      {
         if(chars[i] == ((sChar)_c))
         {
            li = i;
         }
      }
   }

   return li;
}

void PluginString::replace(sChar _c, sChar _o) {
   if(NULL != chars)
   {
      for(sUI i = 0; i < length; i++)
      {
         if(chars[i] == _c)
            chars[i] = _o;
      }
   }
}

void PluginString::getDirName(PluginString *_r) const {
   sSI idxSlash     = lastIndexOf('/', 0);
   sSI idxBackSlash = lastIndexOf('\\', 0);
   sSI idxDrive     = lastIndexOf(':', 0);
   sSI idx = -1;

   if(idxSlash > idxBackSlash)
   {
      idx = idxSlash;
   }
   else
   {
      idx = idxBackSlash;
   }

   if(idxDrive > idx)
   {
      idx = idxDrive;
   }

   if(-1 != idx)
   {
      _r->_realloc(idx + 2);
      _r->length = idx + 2;

      sSI i;
      for(i=0; i<=idx; i++)
      {
         _r->chars[i] = chars[i];
      }

      _r->chars[i++] = 0;
      _r->key = YAC_LOSTKEY;
   }
   else
   {
      _r->empty();
   }
}

#define MAX_FLOATARRAYALLOCSIZE (1024*1024*64)

class PluginFloatArray : public YAC_FloatArray {
public:
   sSI  alloc          (sSI _maxelements);
};

sSI PluginFloatArray::alloc(sSI _max_elements) {
   if(((sUI)_max_elements)>MAX_FLOATARRAYALLOCSIZE)  
   {
      printf("[---] FloatArray::insane array size (maxelements=%08x)\n", _max_elements);
      return 0;
   }
   if(own_data) 
   {
      if(elements)  
      { 
         delete [] elements; 
         elements = NULL;
      } 
   }
   if(_max_elements)
   {
      elements = new(std::nothrow) sF32[_max_elements];
      if(elements)
      {
         max_elements = _max_elements;
         num_elements = 0; 
         own_data     = 1;
         return 1;
      }
   }
   num_elements = 0;
   max_elements = 0;
   return 0;
}



/*
 * I find the naming a bit confusing so I decided to use more meaningful names instead.
 */

/**
 * The VSTHostCallback is a function pointer so that the plugin can communicate with the host (not used in this small example)
 */
typedef audioMasterCallback VSTHostCallback;

/**
 * The VSTPlugin structure (AEffect) contains information about the plugin (like version, number of inputs, ...) and
 * callbacks so that the host can call the plugin to do its work. The primary callback will be `processReplacing` for
 * single precision (float) sample processing (or `processDoubleReplacing` for double precision (double)).
 */
typedef AEffect VSTPlugin;


// Since the host is expecting a very specific API we need to make sure it has C linkage (not C++)
extern "C" {

/*
 * This is the main entry point to the VST plugin.
 *
 * The host (DAW like Maschine, Ableton Live, Reason, ...) will look for this function with this exact API.
 *
 * It is the equivalent to `int main(int argc, char *argv[])` for a C executable.
 *
 * @param vstHostCallback is a callback so that the plugin can communicate with the host (not used in this small example)
 * @return a pointer to the AEffect structure
 */
VST_EXPORT VSTPlugin *VSTPluginMain (VSTHostCallback vstHostCallback);

// note this looks like this without the type aliases (and is obviously 100% equivalent)
// extern AEffect *VSTPluginMain(audioMasterCallback audioMaster);

}

/*
 * Constant for the version of the plugin. For example 1100 for version 1.1.0.0
 */
const VstInt32 PLUGIN_VERSION = 1000;


/**
 * Encapsulates the plugin as a C++ class. It will keep both the host callback and the structure required by the
 * host (VSTPlugin). This class will be stored in the `VSTPlugin.object` field (circular reference) so that it can
 * be accessed when the host calls the plugin back (for example in `processDoubleReplacing`).
 */
class VSTPluginWrapper {
public:
   static const uint32_t MIN_SAMPLE_RATE = 8192u;
   static const uint32_t MAX_SAMPLE_RATE = 384000u;
   static const uint32_t MIN_BLOCK_SIZE  = 64u;
   static const uint32_t MAX_BLOCK_SIZE  = 65536u;

public:
   rack::Global rack_global;
   rack::GlobalUI rack_global_ui;

protected:
   PluginString dllname;
   PluginString cwd;

   float    sample_rate;   // e.g. 44100.0
   uint32_t block_size;    // e.g. 64

   PluginMutex mtx_audio;
public:
   PluginMutex mtx_mididev;

public:

   bool b_open;
   bool b_processing;  // true=generate output, false=suspended

   ERect editor_rect;
   sBool b_editor_open;

   char *last_program_chunk_str;

   static sSI instance_count;
   sSI instance_id;

   sF32 tmp_input_buffers[NUM_INPUTS * MAX_BLOCK_SIZE];

public:
   VSTPluginWrapper(VSTHostCallback vstHostCallback,
                    VstInt32 vendorUniqueID,
                    VstInt32 vendorVersion,
                    VstInt32 numParams,
                    VstInt32 numPrograms,
                    VstInt32 numInputs,
                    VstInt32 numOutputs
                    );

   ~VSTPluginWrapper();

   VSTPlugin *getVSTPlugin(void) {
      return &_vstPlugin;
   }

   void setGlobals(void) {
      rack::global = &rack_global;
      rack::global_ui = &rack_global_ui;
   }

   sSI openEffect(void) {

      printf("xxx vstrack_plugin::openEffect\n");

      // (todo) use mutex 
      instance_id = instance_count;
      printf("xxx vstrack_plugin::openEffect: instance_id=%d\n", instance_id);

      rack_global.vst2.wrapper = this;

#ifdef USE_CONSOLE
      AllocConsole();
      freopen("CON", "w", stdout);
      freopen("CON", "w", stderr);
      freopen("CON", "r", stdin); // Note: "r", not "w".
#endif // USE_CONSOLE

      setGlobals();
      rack_global.init();
      rack_global_ui.init();
      rack::global->vst2.last_seen_instance_count = instance_count;

      char oldCWD[1024];
      char dllnameraw[1024];
      ::GetCurrentDirectory(1024, (LPSTR) oldCWD);
      // ::GetModuleFileNameA(NULL, dllnameraw, 1024); // returns executable name (not the dll pathname)
      GetModuleFileNameA((HINSTANCE)&__ImageBase, dllnameraw, 1024);
      dllname.visit(dllnameraw);
      dllname.getDirName(&cwd);
      rack::global->vst2.program_dir = (const char*)cwd.chars;
      printf("xxx vstrack_plugin::openEffect: cd to \"%s\"\n", (const char*)cwd.chars);
      // // ::SetCurrentDirectory("f:/vst_64bit/vstrack_plugin");
      ::SetCurrentDirectory((const char*)cwd.chars);
      printf("xxx vstrack_plugin::openEffect: cwd change done\n");
      // cwd.replace('\\', '/');

      int argc = 1;
      char *argv[1];
      //argv[0] = (char*)cwd.chars;
      argv[0] = (char*)dllnameraw;
      printf("xxx vstrack_plugin::openEffect: dllname=\"%s\"\n", argv[0]);
      (void)vst2_init(argc, argv);
      printf("xxx vstrack_plugin::openEffect: vst2_init() done\n");

      vst2_set_shared_plugin_tls_globals();

      printf("xxx vstrack_plugin::openEffect: restore cwd=\"%s\"\n", oldCWD);      
      ::SetCurrentDirectory(oldCWD);

      setSampleRate(sample_rate);

      b_open = true;
      b_editor_open = false;

      printf("xxx vstrack_plugin::openEffect: LEAVE\n");
      return 1;
   }

   void setWindowSize(int _width, int _height) {
      if(_width < 640)
         _width = 640;
      if(_height < 480)
         _height = 480;

      editor_rect.right  = EDITWIN_X + _width;
      editor_rect.bottom = EDITWIN_Y + _height;
   }

   void openEditor(void *_hwnd) {
      printf("xxx vstrack_plugin: openEditor() parentHWND=%p\n", _hwnd);
      setGlobals();
      (void)lglw_window_open(rack_global_ui.window.lglw,
                             _hwnd,
                             0/*x*/, 0/*y*/,
                             (editor_rect.right - editor_rect.left),
                             (editor_rect.bottom - editor_rect.top)
                             );
      b_editor_open = true;
   }

   void closeEditor(void) {
      printf("xxx vstrack_plugin: closeEditor() b_editor_open=%d\n", b_editor_open);
      if(b_editor_open)
      {
         setGlobals();
         lglw_window_close(rack_global_ui.window.lglw);
         b_editor_open = false;
      }      
   }

   void closeEffect(void) {

      closeEditor();

      // (todo) use mutex
      printf("xxx vstrack_plugin::closeEffect: last_program_chunk_str=%p\n", last_program_chunk_str);
      if(NULL != last_program_chunk_str)
      {
         ::free(last_program_chunk_str);
         last_program_chunk_str = NULL;
      }

      printf("xxx vstrack_plugin::closeEffect: b_open=%d\n", b_open);

      if(b_open)
      {
         b_open = false;

         setGlobals();
         rack::global->vst2.last_seen_instance_count = instance_count;

         printf("xxx vstrack_plugin: call vst2_exit()\n");

         vst2_exit();

         printf("xxx vstrack_plugin: vst2_exit() done\n");

#ifdef USE_CONSOLE
         // FreeConsole();
#endif // USE_CONSOLE
      }

   }

   void lockAudio(void) {
      mtx_audio.lock();
   }

   void unlockAudio(void) {
      mtx_audio.unlock();
   }

   VstInt32 getNumInputs(void) const {
      return _vstPlugin.numInputs;
   }

   VstInt32 getNumOutputs(void) const {
      return _vstPlugin.numOutputs;
   }

   bool setSampleRate(float _rate) {
      bool r = false;

      if((_rate >= float(MIN_SAMPLE_RATE)) && (_rate <= float(MAX_SAMPLE_RATE)))
      {
         setGlobals();
         lockAudio();
         sample_rate = _rate;
         vst2_set_samplerate(sample_rate);
         unlockAudio();
         r = true;
      }

      return r;
   }

   bool setBlockSize(uint32_t _blockSize) {
      bool r = false;

      if((_blockSize >= MIN_BLOCK_SIZE) && (_blockSize <= MAX_BLOCK_SIZE))
      {
         lockAudio();
         block_size = _blockSize;
         unlockAudio();
         r = true;
      }

      return r;
   }

   void setEnableProcessingActive(bool _bEnable) {
      lockAudio();
      b_processing = _bEnable;

      unlockAudio();
   }

   sUI getBankChunk(uint8_t **_addr) {
      return 0;
   }

   sUI getProgramChunk(uint8_t**_addr) {
      setGlobals();
      if(NULL != last_program_chunk_str)
      {
         ::free(last_program_chunk_str);
      }
      last_program_chunk_str = rack::global_ui->app.gRackWidget->savePatchToString();
      if(NULL != last_program_chunk_str)
      {
         *_addr = (uint8_t*)last_program_chunk_str;
         return (sUI)strlen(last_program_chunk_str) + 1/*ASCIIZ*/;
      }
      return 0;
   }

   bool setBankChunk(size_t _size, uint8_t *_addr) {
      bool r = false;
      return r;
   }

   bool setProgramChunk(size_t _size, uint8_t *_addr) {
      setGlobals();
      lockAudio();
#if 0
      printf("xxx vstrack_plugin:setProgramChunk: size=%u\n", _size);
#endif
      lglw_glcontext_push(rack::global_ui->window.lglw);
      bool r = rack::global_ui->app.gRackWidget->loadPatchFromString((const char*)_addr);
      rack::global_ui->ui.gScene->step();  // w/o this the patch is bypassed
      lglw_glcontext_pop(rack::global_ui->window.lglw);
      printf("xxx vstrack_plugin:setProgramChunk: r=%d\n", r);
      unlockAudio();
      return r;
   }

#ifdef HAVE_WINDOWS
   void sleepMillisecs(uint32_t _num) {
      ::Sleep((DWORD)_num);
   }
#elif defined(HAVE_UNIX)
   void sleepMillisecs(uint32_t _num) {
      ::usleep(1000u * _num);
   }
#endif

   const volatile float *getNextInputChannelChunk(void) {
      volatile float *r = NULL;

      return r;
   }

   volatile float *lockNextOutputChannelChunk(void) {
      volatile float *r = NULL;

      return r;
   }

   void handleUIParam(int uniqueParamId, float normValue) {
      if(NULL != _vstHostCallback)
         (void)_vstHostCallback(&_vstPlugin, audioMasterAutomate, uniqueParamId, 0/*value*/, NULL/*ptr*/, normValue/*opt*/);
   }

   void getTimingInfo(int *_retPlaying, float *_retBPM, float *_retSongPosPPQ) {
      *_retPlaying = 0;

      if(NULL != _vstHostCallback)
      {
         VstIntPtr result = _vstHostCallback(&_vstPlugin, audioMasterGetTime, 0, 0/*value*/, NULL/*ptr*/, 0.0f/*opt*/);
         if(NULL != result)
         {
            const struct VstTimeInfo *timeInfo = (const struct VstTimeInfo *)result;

            *_retPlaying = (0 != (timeInfo->flags & kVstTransportPlaying));

            if(0 != (timeInfo->flags & kVstTempoValid))
            {
               *_retBPM = float(timeInfo->tempo);
            }

            if(0 != (timeInfo->flags & kVstPpqPosValid))
            {
               *_retSongPosPPQ = (float)timeInfo->ppqPos;
            }
         }
      }
   }

private:
   // the host callback (a function pointer)
   VSTHostCallback _vstHostCallback;

   // the actual structure required by the host
   VSTPlugin _vstPlugin;
};

sSI VSTPluginWrapper::instance_count = 0;



/*******************************************
 * Callbacks: Host -> Plugin
 *
 * Defined here because they are used in the rest of the code later
 */

/**
 * This is the callback that will be called to process the samples in the case of single precision. This is where the
 * meat of the logic happens!
 *
 * @param vstPlugin the object returned by VSTPluginMain
 * @param inputs an array of array of input samples. You read from it. First dimension is for inputs, second dimension is for samples: inputs[numInputs][sampleFrames]
 * @param outputs an array of array of output samples. You write to it. First dimension is for outputs, second dimension is for samples: outputs[numOuputs][sampleFrames]
 * @param sampleFrames the number of samples (second dimension in both arrays)
 */
void VSTPluginProcessReplacingFloat32(VSTPlugin *vstPlugin,
                                      float    **_inputs,
                                      float    **outputs,
                                      VstInt32   sampleFrames
                                      ) {
   if(sUI(sampleFrames) > VSTPluginWrapper::MAX_BLOCK_SIZE)
      return;  // should not be reachable

   // we can get a hold to our C++ class since we stored it in the `object` field (see constructor)
   VSTPluginWrapper *wrapper = static_cast<VSTPluginWrapper *>(vstPlugin->object);
   // printf("xxx vstrack_plugin: VSTPluginProcessReplacingFloat32: ENTER\n");
   
   wrapper->lockAudio();
   wrapper->setGlobals();
   vst2_set_shared_plugin_tls_globals();
   // // rack::global->engine.vipMutex.lock();
   rack::global->engine.mutex.lock();
   rack::global->vst2.last_seen_num_frames = sUI(sampleFrames);
   vst2_handle_queued_params();

   //printf("xxx vstrack_plugin: VSTPluginProcessReplacingFloat32: lockAudio done\n");

   //printf("xxx vstrack_plugin: VSTPluginProcessReplacingFloat32: wrapper=%p\n", wrapper);

   sUI chIdx;

   //  (note) Cubase (tested with 9.5.30) uses the same buffer(s) for both input&output
   //           => back up the inputs before clearing the outputs
   sF32 *inputs[NUM_INPUTS];
   sUI k = 0u;
   for(chIdx = 0u; chIdx < NUM_INPUTS; chIdx++)
   {
      inputs[chIdx] = &wrapper->tmp_input_buffers[k];
      ::memcpy((void*)inputs[chIdx], _inputs[chIdx], sizeof(sF32)*sampleFrames);
      k += sampleFrames;
   }

   // Clear output buffers
   //  (note) AudioInterface instances accumulate samples in the output buffer
   for(chIdx = 0u; chIdx < NUM_OUTPUTS; chIdx++)
   {
      ::memset((void*)outputs[chIdx], 0, sizeof(sF32)*sampleFrames);
   }

   if(1 && wrapper->b_processing)
   {

#ifdef HAVE_WINDOWS
      _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
#endif // HAVE_WINDOWS

      vst2_engine_process(inputs, outputs, sampleFrames);
   }

   // // rack::global->engine.vipMutex.unlock();
   rack::global->engine.mutex.unlock();
   wrapper->unlockAudio();

   //printf("xxx vstrack_plugin: VSTPluginProcessReplacingFloat32: LEAVE\n");
   // // glfwSetInstance(NULL); // xxxx test TLS (=> not working in mingw64!)
}



/**
 * This is the plugin called by the host to communicate with the plugin, mainly to request information (like the
 * vendor string, the plugin category...) or communicate state/changes (like open/close, frame rate...)
 *
 * @param vstPlugin the object returned by VSTPluginMain
 * @param opCode defined in aeffect.h/AEffectOpcodes and which continues in aeffectx.h/AEffectXOpcodes for a grand
 *        total of 79 of them! Only a few of them are implemented in this small plugin.
 * @param index depend on the opcode
 * @param value depend on the opcode
 * @param ptr depend on the opcode
 * @param opt depend on the opcode
 * @return depend on the opcode (0 is ok when you don't implement an opcode...)
 */
VstIntPtr VSTPluginDispatcher(VSTPlugin *vstPlugin,
                              VstInt32   opCode,
                              VstInt32   index,
                              VstIntPtr  value,
                              void      *ptr,
                              float      opt
                              ) {
   // printf("vstrack_plugin: called VSTPluginDispatcher(%d)\n", opCode);

   VstIntPtr r = 0;

   // we can get a hold to our C++ class since we stored it in the `object` field (see constructor)
   VSTPluginWrapper *wrapper = static_cast<VSTPluginWrapper *>(vstPlugin->object);

   // see aeffect.h/AEffectOpcodes and aeffectx.h/AEffectXOpcodes for details on all of them
   switch(opCode)
   {
      case effGetPlugCategory:
         // request for the category of the plugin: in this case it is an effect since it is modifying the input (as opposed
         // to generating sound)
#ifdef VST2_EFFECT
         return kPlugCategEffect;
#else
         return kPlugCategSynth;
#endif // VST2_EFFECT

      case effOpen:
         // called by the host after it has obtained the effect instance (but _not_ during plugin scans)
         //  (note) any heavy-lifting init code should go here
         ::printf("vstrack_plugin<dispatcher>: effOpen\n");
         r = wrapper->openEffect();
         break;

      case effClose:
         // called by the host when the plugin was called... time to reclaim memory!
         wrapper->closeEffect();
         // (note) hosts usually call effStopProcess before effClose
         delete wrapper;
         break;

      case effSetProgram:
         r = 1;
         break;

      case effGetProgram:
         r = 0;
         break;

      case effGetVendorString:
         // request for the vendor string (usually used in the UI for plugin grouping)
         ::strncpy(static_cast<char *>(ptr), "bsp", kVstMaxVendorStrLen);
         r = 1;
         break;

      case effGetVendorVersion:
         // request for the version
         return PLUGIN_VERSION;

      case effGetEffectName:
#ifdef VST2_EFFECT
         ::strncpy((char*)ptr, "VeeSeeVST Rack 0.6.1", kVstMaxEffectNameLen);
#else
         ::strncpy((char*)ptr, "VeeSeeVST Rack 0.6.1 I", kVstMaxEffectNameLen);
#endif // VST2_EFFECT
         r = 1;
         break;

      case effGetProductString:
#ifdef VST2_EFFECT
         ::strncpy((char*)ptr, "VeeSeeVST Rack 0.6.1 VST2 Plugin v0.4", kVstMaxProductStrLen);
#else
         ::strncpy((char*)ptr, "VeeSeeVST Rack 0.6.1 I VST2 Plugin v0.4", kVstMaxProductStrLen);
#endif // VST2_EFFECT
         r = 1;
         break;

      case effGetNumMidiInputChannels:
         r = 16;
         break;

      case effGetNumMidiOutputChannels:
         r = 0;
         break;

      case effGetInputProperties:
         {
            VstPinProperties *pin = (VstPinProperties*)ptr;
            ::snprintf(pin->label, kVstMaxLabelLen, "Input #%d", index);
            pin->flags           = kVstPinIsActive | ((0 == (index & 1)) ? kVstPinIsStereo : 0);
            pin->arrangementType = ((0 == (index & 1)) ? kSpeakerArrStereo : kSpeakerArrMono);
            ::snprintf(pin->shortLabel, kVstMaxShortLabelLen, "in%d", index);
            memset((void*)pin->future, 0, 48);
            r = 1;
         }
         break;

      case effGetOutputProperties:
         {
            VstPinProperties *pin = (VstPinProperties*)ptr;
            ::snprintf(pin->label, kVstMaxLabelLen, "Output #%d", index);
            pin->flags           = kVstPinIsActive | ((0 == (index & 1)) ? kVstPinIsStereo : 0);
            pin->arrangementType = ((0 == (index & 1)) ? kSpeakerArrStereo : kSpeakerArrMono);
            ::snprintf(pin->shortLabel, kVstMaxShortLabelLen, "out%d", index);
            memset((void*)pin->future, 0, 48);
            r = 1;
         }
         break;

      case effSetSampleRate:
         r = wrapper->setSampleRate(opt) ? 1 : 0;
         break;

      case effSetBlockSize:
         r = wrapper->setBlockSize(uint32_t(value)) ? 1 : 0;
         break;

      case effCanDo:
         // ptr:
         // "sendVstEvents"
         // "sendVstMidiEvent"
         // "sendVstTimeInfo"
         // "receiveVstEvents"
         // "receiveVstMidiEvent"
         // "receiveVstTimeInfo"
         // "offline"
         // "plugAsChannelInsert"
         // "plugAsSend"
         // "mixDryWet"
         // "noRealTime"
         // "multipass"
         // "metapass"
         // "1in1out"
         // "1in2out"
         // "2in1out"
         // "2in2out"
         // "2in4out"
         // "4in2out"
         // "4in4out"
         // "4in8out"
         // "8in4out"
         // "8in8out"
         // "midiProgramNames"
         // "conformsToWindowRules"
         if(!strcmp((char*)ptr, "receiveVstEvents"))
            r = 1;
         else
            r = 0;
         break;

      case effGetProgramName:
         ::snprintf((char*)ptr, kVstMaxProgNameLen, "default");
         r = 1;
         break;

      case effSetProgramName:
         r = 1;
         break;

      case effGetProgramNameIndexed:
         ::sprintf((char*)ptr, "default");
         r = 1;
         break;

      case effGetParamName:
      case effGetParamLabel:
         // kVstMaxParamStrLen(8), much longer in other plugins
         // printf("xxx vstrack_plugin: effGetParamName: ptr=%p\n", ptr);
         wrapper->setGlobals();
         vst2_get_param_name(index, (char*)ptr, kVstMaxParamStrLen);
         r = 1;
         break;

      case effGetParameterProperties:
         r = 0;
         break;

      case effGetChunk:
         // Query bank (index=0) or program (index=1) state
         //  value: 0
         //    ptr: buffer address
         //      r: buffer size
         printf("xxx effGetChunk index=%d ptr=%p\n", index, ptr);
         // // if(0 == index)
         // // {
         // //    r = wrapper->getBankChunk((uint8_t**)ptr);
         // // }
         // // else
         // // {
            r = wrapper->getProgramChunk((uint8_t**)ptr);
         // // }
         break;

      case effSetChunk:
         // Restore bank (index=0) or program (index=1) state
         //  value: buffer size
         //    ptr: buffer address
         //      r: 1
         printf("xxx effSetChunk index=%d size=%lld ptr=%p\n", index, value, ptr);
         // // if(0 == index)
         // // {
         // //    r = wrapper->setBankChunk(size_t(value), (uint8_t*)ptr) ? 1 : 0;
         // // }
         // // else
         // // {
            r = wrapper->setProgramChunk(size_t(value), (uint8_t*)ptr) ? 1 : 0;
         // // }
         break;

      case effShellGetNextPlugin:
         // For shell plugins (e.g. Waves), returns next sub-plugin UID (or 0)
         //  (note) plugin uses audioMasterCurrentId while it's being instantiated to query the currently selected sub-plugin
         //          if the host returns 0, it will then call effShellGetNextPlugin to enumerate the sub-plugins
         //  ptr: effect name string ptr (filled out by the plugin)
         r = 0;
         break;

      case effMainsChanged:
         // value = 0=suspend, 1=resume
         wrapper->setEnableProcessingActive((value > 0) ? true : false);
         r = 1;
         break;

      case effStartProcess:
         wrapper->setEnableProcessingActive(true);
         r = 1;
         break;

      case effStopProcess:
         wrapper->setEnableProcessingActive(false);
         r = 1;
         break;

      case effProcessEvents:
         // ptr: VstEvents*
         {
            VstEvents *events = (VstEvents*)ptr;
            //printf("vstrack_plugin:effProcessEvents: recvd %d events", events->numEvents);
            VstEvent**evAddr = &events->events[0];

            if(events->numEvents > 0)
            {
               wrapper->setGlobals();
               wrapper->mtx_mididev.lock();
            
               for(uint32_t evIdx = 0u; evIdx < uint32_t(events->numEvents); evIdx++, evAddr++)
               {
                  VstEvent *ev = *evAddr;

                  if(NULL != ev)  // paranoia
                  {
#ifdef DEBUG_PRINT_EVENTS
                     printf("vstrack_plugin:effProcessEvents: ev[%u].byteSize    = %u\n", evIdx, uint32_t(ev->byteSize));  // sizeof(VstMidiEvent) = 32
                     printf("vstrack_plugin:effProcessEvents: ev[%u].deltaFrames = %u\n", evIdx, uint32_t(ev->deltaFrames));
#endif // DEBUG_PRINT_EVENTS

                     switch(ev->type)
                     {
                        default:
                           //case kVstAudioType:      // deprecated
                           //case kVstVideoType:      // deprecated
                           //case kVstParameterType:  // deprecated
                           //case kVstTriggerType:    // deprecated
                           break;

                        case kVstMidiType:
                           // (note) ev->data stores the actual payload (up to 16 bytes)
                           // (note) e.g. 0x90 0x30 0x7F for a C-4 note-on on channel 1 with velocity 127
                           // (note) don't forget to use a mutex (lockAudio(), unlockAudio()) when modifying the audio processor state!
                        {
                           VstMidiEvent *mev = (VstMidiEvent *)ev;
#ifdef DEBUG_PRINT_EVENTS
                           printf("vstrack_plugin:effProcessEvents<midi>: ev[%u].noteLength      = %u\n", evIdx, uint32_t(mev->noteLength));  // #frames
                           printf("vstrack_plugin:effProcessEvents<midi>: ev[%u].noteOffset      = %u\n", evIdx, uint32_t(mev->noteOffset));  // #frames
                           printf("vstrack_plugin:effProcessEvents<midi>: ev[%u].midiData        = %02x %02x %02x %02x\n", evIdx, uint8_t(mev->midiData[0]), uint8_t(mev->midiData[1]), uint8_t(mev->midiData[2]), uint8_t(mev->midiData[3]));
                           printf("vstrack_plugin:effProcessEvents<midi>: ev[%u].detune          = %d\n", evIdx, mev->detune); // -64..63
                           printf("vstrack_plugin:effProcessEvents<midi>: ev[%u].noteOffVelocity = %d\n", evIdx, mev->noteOffVelocity); // 0..127
#endif // DEBUG_PRINT_EVENTS
                           vst2_process_midi_input_event(mev->midiData[0],
                                                         mev->midiData[1],
                                                         mev->midiData[2]
                                                         );
                        }
                        break;

                        case kVstSysExType:
                        {
                           VstMidiSysexEvent *xev = (VstMidiSysexEvent*)ev;
#ifdef DEBUG_PRINT_EVENTS
                           printf("vstrack_plugin:effProcessEvents<syx>: ev[%u].dumpBytes = %u\n", evIdx, uint32_t(xev->dumpBytes));  // size
                           printf("vstrack_plugin:effProcessEvents<syx>: ev[%u].sysexDump = %p\n", evIdx, xev->sysexDump);            // buffer addr
#endif // DEBUG_PRINT_EVENTS

                           // (note) don't forget to use a mutex (lockAudio(), unlockAudio()) when modifying the audio processor state!
                        }
                        break;
                     }
                  } // if ev
               } // loop events

               wrapper->mtx_mididev.unlock();
            } // if events
         }
         break;

      case effGetTailSize: // 52
         break;
#if 1
      //case effIdle:
      case 53:
         // Periodic idle call (from UI thread), e.g. at 20ms intervals (depending on host)
         //  (note) deprecated in vst2.4 (but some plugins still rely on this)
         r = 1;
         break;
#endif

      case effEditIdle:
         wrapper->setGlobals();
         if(lglw_window_is_visible(rack::global_ui->window.lglw))
         {
            vst2_set_shared_plugin_tls_globals();

            // Save DAW GL context and bind our own
            lglw_glcontext_push(rack::global_ui->window.lglw);

            rack::vst2_editor_redraw();

            // Restore the DAW's GL context
            lglw_glcontext_pop(rack::global_ui->window.lglw);
         }
         break;

      case effEditGetRect:
         // Query editor window geometry
         // ptr: ERect* (on Windows)
         if(NULL != ptr) // yeah, this should never be NULL
         {
            // ...
            *(void**)ptr = (void*) &wrapper->editor_rect;
            r = 1;
         }
         else
         {
            r = 0;
         }
         break;

#if 0
      case effEditTop:
         // deprecated in vst2.4
         r = 0;
         break;
#endif

      case effEditOpen:
         // Show editor window
         // ptr: native window handle (hWnd on Windows)
         wrapper->openEditor(ptr);
         r = 1;
         break;

      case effEditClose:
         // Close editor window
         wrapper->closeEditor();
         r = 1;
         break;

      default:
         // ignoring all other opcodes
         printf("vstrack_plugin:dispatcher: unhandled opCode %d [ignored] \n", opCode);
         break;

   }

   return r;
}


/**
 * Set parameter setting
 */
void VSTPluginSetParameter(VSTPlugin *vstPlugin,
                           VstInt32   index,
                           float      parameter
                           ) {
#ifdef DEBUG_PRINT_PARAMS
   printf("vstrack_plugin: called VSTPluginSetParameter(%d, %f)\n", index, parameter);
#endif // DEBUG_PRINT_PARAMS

   // we can get a hold to our C++ class since we stored it in the `object` field (see constructor)
   VSTPluginWrapper *wrapper = static_cast<VSTPluginWrapper *>(vstPlugin->object);

   wrapper->lockAudio();
   wrapper->setGlobals();
   vst2_queue_param(index, parameter);
   wrapper->unlockAudio();
}


/**
 * Query parameter
 */
float VSTPluginGetParameter(VSTPlugin *vstPlugin,
                            VstInt32   index
                            ) {
#ifdef DEBUG_PRINT_PARAMS
   printf("vstrack_plugin: called VSTPluginGetParameter(%d)\n", index);
#endif // DEBUG_PRINT_PARAMS
   // we can get a hold to our C++ class since we stored it in the `object` field (see constructor)
   VSTPluginWrapper *wrapper = static_cast<VSTPluginWrapper *>(vstPlugin->object);

   wrapper->lockAudio();  // don't query a param while the module is deleted
   wrapper->setGlobals();
   float r = vst2_get_param(index);
   wrapper->unlockAudio();

   return r;
}


/**
 * Main constructor for our C++ class
 */
VSTPluginWrapper::VSTPluginWrapper(audioMasterCallback vstHostCallback,
                                   VstInt32 vendorUniqueID,
                                   VstInt32 vendorVersion,
                                   VstInt32 numParams,
                                   VstInt32 numPrograms,
                                   VstInt32 numInputs,
                                   VstInt32 numOutputs
                                   ) : _vstHostCallback(vstHostCallback)
{
   instance_count++;

   // Make sure that the memory is properly initialized
   memset(&_vstPlugin, 0, sizeof(_vstPlugin));

   // this field must be set with this constant...
   _vstPlugin.magic = kEffectMagic;

   // storing this object into the VSTPlugin so that it can be retrieved when called back (see callbacks for use)
   _vstPlugin.object = this;

   // specifying that we handle both single and NOT double precision (there are other flags see aeffect.h/VstAEffectFlags)
   _vstPlugin.flags = 
#ifndef VST2_EFFECT
      effFlagsIsSynth                  |
#endif
      effFlagsCanReplacing             | 
      // (effFlagsCanDoubleReplacing & 0) |
      effFlagsProgramChunks            |
      effFlagsHasEditor                ;

   // initializing the plugin with the various values
   _vstPlugin.uniqueID    = vendorUniqueID;
   _vstPlugin.version     = vendorVersion;
   _vstPlugin.numParams   = numParams;
   _vstPlugin.numPrograms = numPrograms;
   _vstPlugin.numInputs   = numInputs;
   _vstPlugin.numOutputs  = numOutputs;

   // setting the callbacks to the previously defined functions
   _vstPlugin.dispatcher             = &VSTPluginDispatcher;
   _vstPlugin.getParameter           = &VSTPluginGetParameter;
   _vstPlugin.setParameter           = &VSTPluginSetParameter;
   _vstPlugin.processReplacing       = &VSTPluginProcessReplacingFloat32;
   _vstPlugin.processDoubleReplacing = NULL;//&VSTPluginProcessReplacingFloat64;

   // report latency
   _vstPlugin.initialDelay = 0;

   sample_rate  = 44100.0f;
   block_size   = 64u;
   b_processing = true;

   last_program_chunk_str = NULL;

   b_open = false;
   b_editor_open = false;

   editor_rect.left   = EDITWIN_X;
   editor_rect.top    = EDITWIN_Y;
   editor_rect.right  = EDITWIN_X + EDITWIN_W;
   editor_rect.bottom = EDITWIN_Y + EDITWIN_H;
}

/**
 * Destructor called when the plugin is closed (see VSTPluginDispatcher with effClose opCode). In this very simply plugin
 * there is nothing to do but in general the memory that gets allocated MUST be freed here otherwise there might be a
 * memory leak which may end up slowing down and/or crashing the host
 */
VSTPluginWrapper::~VSTPluginWrapper() {
   closeEffect();
   instance_count--;
}


void vst2_lock_midi_device() {
   rack::global->vst2.wrapper->mtx_mididev.lock();
}

void vst2_unlock_midi_device() {
   rack::global->vst2.wrapper->mtx_mididev.unlock();
}

// void vst2_handle_queued_set_program_chunk(void) {
//    (void)rack::global->vst2.wrapper->handleSetQueuedProgramChunk();
// }

void vst2_handle_ui_param(int uniqueParamId, float normValue) {
   // Called by engineSetParam()
   rack::global->vst2.wrapper->handleUIParam(uniqueParamId, normValue);
}

void vst2_get_timing_info(int *_retPlaying, float *_retBPM, float *_retSongPosPPQ) {
   // updates the requested fields when query was successful
   rack::global->vst2.wrapper->getTimingInfo(_retPlaying, _retBPM, _retSongPosPPQ);
}

void vst2_set_globals(void *_wrapper) {
   VSTPluginWrapper *wrapper = (VSTPluginWrapper *)_wrapper;
   wrapper->setGlobals();
}

void vst2_window_size_set(int _width, int _height) {
   rack::global->vst2.wrapper->setWindowSize(_width, _height);
}


/**
 * Implementation of the main entry point of the plugin
 */
VST_EXPORT VSTPlugin *VSTPluginMain(VSTHostCallback vstHostCallback) {
   printf("vstrack_plugin: called VSTPluginMain... \n");

   // simply create our plugin C++ class
   VSTPluginWrapper *plugin =
      new VSTPluginWrapper(vstHostCallback,
                           // registered with Steinberg (http://service.steinberg.de/databases/plugin.nsf/plugIn?openForm)
#ifdef VST2_EFFECT
                           CCONST('g', 'v', 'g', 'y'),
#else
                           CCONST('v', '5', 'k', 'v'),
#endif
                           PLUGIN_VERSION, // version
                           VST2_MAX_UNIQUE_PARAM_IDS,    // num params
                           0,    // no programs
                           NUM_INPUTS,
                           NUM_OUTPUTS
                           );

   // return the plugin per the contract of the API
   return plugin->getVSTPlugin();
}
#endif // USE_VST2

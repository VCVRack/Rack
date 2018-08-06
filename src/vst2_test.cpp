//#ifdef USE_VST2
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
///
///
///

// #define DEBUG_PRINT_EVENTS  defined
// #define DEBUG_PRINT_PARAMS  defined

#define NUM_INPUTS    (   2)  // must match AudioInterface.cpp:AUDIO_INPUTS
#define NUM_OUTPUTS   (   2)  // must match AudioInterface.cpp:AUDIO_OUTPUTS

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

extern int  vst2_init (int argc, char* argv[]);
extern void vst2_exit (void);
extern void vst2_editor_create (void);
extern void vst2_editor_loop (void);
extern void vst2_editor_destroy (void);
extern void vst2_set_samplerate (sF32 _rate);
extern void vst2_engine_process (float *const*_in, float **_out, unsigned int _numFrames);
extern void vst2_process_midi_input_event (sU8 _a, sU8 _b, sU8 _c);
extern void vst2_queue_param (int uniqueParamId, float normValue);
extern void vst2_handle_queued_params (void);
extern float vst2_get_param (int uniqueParamId);
extern void  vst2_get_param_name (int uniqueParamId, char *s, int sMaxLen);
extern void vst2_set_shared_plugin_tls_globals (void);


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

extern "C" extern HWND g_glfw_vst2_parent_hwnd;  // read by modified version of GLFW (see glfw/src/win32_window.c)
extern "C" extern HWND __hack__glfwGetHWND (GLFWwindow *window);


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

void vst2_lock_midi_device() {
}

void vst2_unlock_midi_device() {
}

void vst2_handle_queued_set_program_chunk(void) {
}

void vst2_handle_ui_param(int uniqueParamId, float normValue) {
}

void vst2_get_timing_info(int *_retPlaying, float *_retBPM, float *_retSongPosPPQ) {
}

void vst2_maximize_reparented_window(void) {
}

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
VST_EXPORT VSTPlugin *VSTPluginMain(VSTHostCallback vstHostCallback);

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
class VSTPluginWrapper
{
public:
  VSTPluginWrapper(VSTHostCallback vstHostCallback,
                   VstInt32 vendorUniqueID,
                   VstInt32 vendorVersion,
                   VstInt32 numParams,
                   VstInt32 numPrograms,
                   VstInt32 numInputs,
                   VstInt32 numOutputs);

  ~VSTPluginWrapper();

  inline VSTPlugin *getVSTPlugin()
  {
    return &_vstPlugin;
  }

  inline VstInt32 getNumInputs() const
  {
    return _vstPlugin.numInputs;
  }

  inline VstInt32 getNumOutputs() const
  {
    return _vstPlugin.numOutputs;
  }

private:
  // the host callback (a function pointer)
  VSTHostCallback _vstHostCallback;

  // the actual structure required by the host
  VSTPlugin _vstPlugin;
};

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
void VSTPluginProcessSamplesFloat32(VSTPlugin *vstPlugin, float **inputs, float **outputs, VstInt32 sampleFrames)
{
  // we can get a hold to our C++ class since we stored it in the `object` field (see constructor)
  VSTPluginWrapper *wrapper = static_cast<VSTPluginWrapper *>(vstPlugin->object);

  // code speaks for itself: for each input (2 when stereo input), iterating over every sample and writing the
  // result in the outputs array after multiplying by 0.5 (which result in a 3dB attenuation of the sound)
  for(int i = 0; i < wrapper->getNumInputs(); i++)
  {
    auto inputSamples = inputs[i];
    auto outputSamples = outputs[i];
    for(int j = 0; j < sampleFrames; j++)
    {
      outputSamples[j] = inputSamples[j] * 0.5f;
    }
  }
}

/**
 * This is the callback that will be called to process the samples in the case of double precision. This is where the
 * meat of the logic happens!
 *
 * @param vstPlugin the object returned by VSTPluginMain
 * @param inputs an array of array of input samples. You read from it. First dimension is for inputs, second dimension is for samples: inputs[numInputs][sampleFrames]
 * @param outputs an array of array of output samples. You write to it. First dimension is for outputs, second dimension is for samples: outputs[numOuputs][sampleFrames]
 * @param sampleFrames the number of samples (second dimension in both arrays)
 */
void VSTPluginProcessSamplesFloat64(VSTPlugin *vstPlugin, double **inputs, double **outputs, VstInt32 sampleFrames)
{
  // we can get a hold to our C++ class since we stored it in the `object` field (see constructor)
  VSTPluginWrapper *wrapper = static_cast<VSTPluginWrapper *>(vstPlugin->object);

  // code speaks for itself: for each input (2 when stereo input), iterating over every sample and writing the
  // result in the outputs array after multiplying by 0.5 (which result in a 3dB attenuation of the sound)
  for(int i = 0; i < wrapper->getNumInputs(); i++)
  {
    auto inputSamples = inputs[i];
    auto outputSamples = outputs[i];
    for(int j = 0; j < sampleFrames; j++)
    {
      outputSamples[j] = inputSamples[j] * 0.5;
    }
  }
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
VstIntPtr VSTPluginDispatcher(VSTPlugin *vstPlugin, VstInt32 opCode, VstInt32 index, VstIntPtr value, void *ptr, float opt)
{
  printf("called VSTPluginDispatcher(%d)\n", opCode);

  VstIntPtr v = 0;

  // we can get a hold to our C++ class since we stored it in the `object` field (see constructor)
  VSTPluginWrapper *wrapper = static_cast<VSTPluginWrapper *>(vstPlugin->object);
  // see aeffect.h/AEffectOpcodes and aeffectx.h/AEffectXOpcodes for details on all of them
  switch(opCode)
  {
    // request for the category of the plugin: in this case it is an effect since it is modifying the input (as opposed
    // to generating sound)
    case effGetPlugCategory:
      return kPlugCategEffect;

    // called by the host when the plugin was called... time to reclaim memory!
    case effClose:
      delete wrapper;
      break;

    // request for the vendor string (usually used in the UI for plugin grouping)
    case effGetVendorString:
      strncpy(static_cast<char *>(ptr), "testsoft", kVstMaxVendorStrLen);
      v = 1;
      break;

    // request for the version
    case effGetVendorVersion:
      return PLUGIN_VERSION;

    // ignoring all other opcodes
    default:
      printf("Unknown opCode %d [ignored] \n", opCode);
      break;
  }

  return v;
}

/**
 * Used for parameter setting (not used by this plugin)
 */
void VSTPluginSetParameter(VSTPlugin *vstPlugin, VstInt32 index, float parameter)
{
  printf("called VSTPluginSetParameter(%d, %f)\n", index, parameter);
  // we can get a hold to our C++ class since we stored it in the `object` field (see constructor)
  VSTPluginWrapper *wrapper = static_cast<VSTPluginWrapper *>(vstPlugin->object);
}

/**
 * Used for parameter (not used by this plugin)
 */
float VSTPluginGetParameter(VSTPlugin *vstPlugin, VstInt32 index)
{
  printf("called VSTPluginGetParameter(%d)\n", index);
  // we can get a hold to our C++ class since we stored it in the `object` field (see constructor)
  VSTPluginWrapper *wrapper = static_cast<VSTPluginWrapper *>(vstPlugin->object);
  return 0;
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
                                   VstInt32 numOutputs) :
  _vstHostCallback(vstHostCallback)
{
  // Make sure that the memory is properly initialized
  memset(&_vstPlugin, 0, sizeof(_vstPlugin));

  // this field must be set with this constant...
  _vstPlugin.magic = kEffectMagic;

  // storing this object into the VSTPlugin so that it can be retrieved when called back (see callbacks for use)
  _vstPlugin.object = this;

  // specifying that we handle both single and double precision (there are other flags see aeffect.h/VstAEffectFlags)
  _vstPlugin.flags = effFlagsCanReplacing | effFlagsCanDoubleReplacing;

  // initializing the plugin with the various values
  _vstPlugin.uniqueID = vendorUniqueID;
  _vstPlugin.version = vendorVersion;
  _vstPlugin.numParams = numParams;
  _vstPlugin.numPrograms = numPrograms;
  _vstPlugin.numInputs = numInputs;
  _vstPlugin.numOutputs = numOutputs;

  // setting the callbacks to the previously defined functions
  _vstPlugin.dispatcher = VSTPluginDispatcher;
  _vstPlugin.getParameter = VSTPluginGetParameter;
  _vstPlugin.setParameter = VSTPluginSetParameter;
  _vstPlugin.processReplacing = VSTPluginProcessSamplesFloat32;
  _vstPlugin.processDoubleReplacing = VSTPluginProcessSamplesFloat64;

}

/**
 * Destructor called when the plugin is closed (see VSTPluginDispatcher with effClose opCode). In this very simply plugin
 * there is nothing to do but in general the memory that gets allocated MUST be freed here otherwise there might be a
 * memory leak which may end up slowing down and/or crashing the host
 */
VSTPluginWrapper::~VSTPluginWrapper()
{
}

/**
 * Implementation of the main entry point of the plugin
 */
VST_EXPORT VSTPlugin *VSTPluginMain(VSTHostCallback vstHostCallback)
{
  printf("called VSTPluginMain... \n");

  // simply create our plugin C++ class
  VSTPluginWrapper *plugin =
    new VSTPluginWrapper(vstHostCallback,
                         CCONST('u', 's', 'b', 'n'), // registered with Steinberg (http://service.steinberg.de/databases/plugin.nsf/plugIn?openForm)
                         PLUGIN_VERSION, // version
                         0,    // no params
                         0,    // no programs
                         2,    // 2 inputs
                         2);   // 2 outputs

  // return the plugin per the contract of the API
  return plugin->getVSTPlugin();
}

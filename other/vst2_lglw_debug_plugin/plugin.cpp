
#define USE_LGLW defined

#include <aeffect.h>
#include <aeffectx.h>
#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)
  #include <windows.h>
  #include <wingdi.h>
  #define VST_EXPORT  extern "C" __declspec(dllexport)
#else
  #include <GL/gl.h>
  #include <X11/Xlib.h>
  #include <X11/Xutil.h>
  #include <X11/Xos.h>
  #define VST_EXPORT extern
  #include <dlfcn.h>
#endif

#include "lglw.h"

#define EDITWIN_X 20
#define EDITWIN_Y 20
#define EDITWIN_W 640
#define EDITWIN_H 480

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
VST_EXPORT VSTPlugin *VSTPluginMain(VSTHostCallback vstHostCallback);

// note this looks like this without the type aliases (and is obviously 100% equivalent)
// extern AEffect *VSTPluginMain(audioMasterCallback audioMaster);

}

/*
 * Constant for the version of the plugin. For example 1100 for version 1.1.0.0
 */
const VstInt32 PLUGIN_VERSION = 1000;

// extern "C" LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

#ifdef USE_LGLW
void loc_mouse_cbk(lglw_t _lglw, int32_t _x, int32_t _y, uint32_t _buttonState, uint32_t _changedButtonState) {
   printf("vstgltest: lglw_mouse_cbk: lglw=%p p=(%d; %d) bt=0x%08x changedBt=0x%08x\n", _lglw, _x, _y, _buttonState, _changedButtonState);

   if(LGLW_IS_MOUSE_LBUTTON_DOWN())
   {
      // lglw_mouse_grab(_lglw, LGLW_MOUSE_GRAB_CAPTURE);
      lglw_mouse_grab(_lglw, LGLW_MOUSE_GRAB_WARP);
   }
   else if(LGLW_IS_MOUSE_LBUTTON_UP())
   {
      lglw_mouse_ungrab(_lglw);
   }
}

void loc_focus_cbk(lglw_t _lglw, uint32_t _focusState, uint32_t _changedFocusState) {
   printf("vstgltest: lglw_focus_cbk: lglw=%p focusState=0x%08x changedFocusState=0x%08x\n", _lglw, _focusState, _changedFocusState);
}

lglw_bool_t loc_keyboard_cbk(lglw_t _lglw, uint32_t _vkey, uint32_t _kmod, lglw_bool_t _bPressed) {
   printf("vstgltest: lglw_keyboard_cbk: lglw=%p vkey=0x%08x (\'%c\') kmod=0x%08x bPressed=%d\n", _lglw, _vkey, _vkey, _kmod, _bPressed);
   return LGLW_FALSE;
}

void loc_timer_cbk(lglw_t _lglw) {
   printf("vstgltest: lglw_timer_cbk: tick\n");
}
#endif // USE_LGLW

/**
 * Encapsulates the plugin as a C++ class. It will keep both the host callback and the structure required by the
 * host (VSTPlugin). This class will be stored in the `VSTPlugin.object` field (circular reference) so that it can
 * be accessed when the host calls the plugin back (for example in `processDoubleReplacing`).
 */
class VSTPluginWrapper
{
public:
   ERect editor_rect;

#ifdef USE_LGLW
   lglw_t lglw;
#endif // USE_LGLW

   float clear_color = 0.0f;

   static VSTPluginWrapper *window_to_wrapper;

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

   int openEffect(void) {
      printf("vstgltest: openEffect()\n");
      return 1;
   }

   void closeEffect(void) {
      closeEditor();
   }

   void openEditor(void *_hwnd) {

#ifdef USE_LGLW      
      (void)lglw_window_open(lglw, _hwnd, 0/*x*/, 0/*y*/, EDITWIN_W, EDITWIN_H);

      lglw_mouse_callback_set(lglw, &loc_mouse_cbk);
      lglw_focus_callback_set(lglw, &loc_focus_cbk);
      lglw_keyboard_callback_set(lglw, &loc_keyboard_cbk);
      lglw_timer_callback_set(lglw, &loc_timer_cbk);

      lglw_timer_start(lglw, 200);
#endif // USE_LGLW

      window_to_wrapper = this;
   }

   void closeEditor(void) {
      if(NULL != window_to_wrapper)
      {
#ifdef USE_LGLW
         lglw_window_close(lglw);
#endif // USE_LGLW

         window_to_wrapper = NULL;
      }
   }

   static VSTPluginWrapper *FindWrapperByWindow(Window hwnd) {
      return window_to_wrapper;
   }

   void redrawWindow(void) {
#if 1
#ifdef USE_LGLW
      // Save host GL context
      lglw_glcontext_push(lglw);

      // Draw something
      ::glClearColor(0.0, 1.0, clear_color, 1.0);
      clear_color += 0.05f;
      if(clear_color >= 1.0f)
         clear_color -= 1.0f;
      ::glClear(GL_COLOR_BUFFER_BIT);

      ::glFlush();
      lglw_swap_buffers(lglw);

      // Restore host GL context
      lglw_glcontext_pop(lglw);
#endif // USE_LGLW
#endif
   }

private:
   // the host callback (a function pointer)
   VSTHostCallback _vstHostCallback;

   // the actual structure required by the host
   VSTPlugin _vstPlugin;
};

VSTPluginWrapper *VSTPluginWrapper::window_to_wrapper = NULL;


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
extern "C" {
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
extern "C" {
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
extern "C" {
VstIntPtr VSTPluginDispatcher(VSTPlugin *vstPlugin, VstInt32 opCode, VstInt32 index, VstIntPtr value, void *ptr, float opt)
{
   // printf("vstgltest: called VSTPluginDispatcher(%d)\n", opCode);

   VstIntPtr r = 0;

   // we can get a hold to our C++ class since we stored it in the `object` field (see constructor)
   VSTPluginWrapper *wrapper = static_cast<VSTPluginWrapper *>(vstPlugin->object);
   // see aeffect.h/AEffectOpcodes and aeffectx.h/AEffectXOpcodes for details on all of them
   switch(opCode)
   {
      default:
         printf("vstgltest: unhandled VSTPluginDispatcher opcode=%d\n", opCode);
         break;

      case effGetVstVersion: /*58*/
         r = 0;
         break;

      // request for the category of the plugin: in this case it is an effect since it is modifying the input (as opposed
      // to generating sound)
      case effGetPlugCategory:
         // return kPlugCategEffect;
         return kPlugCategSynth;

      case effOpen:
         // called by the host after it has obtained the effect instance (but _not_ during plugin scans)
         //  (note) any heavy-lifting init code should go here
         printf("vstgltest: effOpen\n");
         r = wrapper->openEffect();
         break;

         // called by the host when the plugin was called... time to reclaim memory!
      case effClose:
         printf("vstgltest: effClose\n");
         wrapper->closeEffect();
         delete wrapper;
         break;

      case effGetEffectName:
         ::strncpy((char*)ptr, "VST GL Test", kVstMaxEffectNameLen);
         r = 1;
         break;

      case effGetProductString:
         ::strncpy((char*)ptr, "VST GL Test ProdStr", kVstMaxProductStrLen);
         r = 1;
         break;

         // request for the vendor string (usually used in the UI for plugin grouping)
      case effGetVendorString:
         strncpy(static_cast<char *>(ptr), "bsp", kVstMaxVendorStrLen);
         r = 1;
         break;

         // request for the version
      case effGetVendorVersion:
         return PLUGIN_VERSION;

      case effGetNumMidiInputChannels:
         r = 16;
         break;

      case effGetNumMidiOutputChannels:
         r = 0;
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
         else if(!strcmp((char*)ptr, "receiveVstMidiEvent"))  // (note) required by Jeskola Buzz
            r = 1;
         else if(!strcmp((char*)ptr, "noRealTime"))
            r = 1;
         else
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
         printf("vstgltest: effSetSampleRate(%f)\n", opt);
         r = 1;////wrapper->setSampleRate(opt) ? 1 : 0;
         break;

      case effSetBlockSize:
         printf("vstgltest: effSetBlockSize(%u)\n", uint32_t(value));
         r = 1;////wrapper->setBlockSize(uint32_t(value)) ? 1 : 0;
         break;

      case effMainsChanged:
         printf("vstgltest: effMainsChanged(%d)\n", value);
         // value = 0=suspend, 1=resume
         // wrapper->setEnableProcessingActive((value > 0) ? true : false);
         r = 1;
         break;

      case effSetProgram:
         r = 1;
         break;

      case effGetProgram:
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
         strncpy(static_cast<char *>(ptr), "myparam", kVstMaxParamStrLen);
         r = 1;
         break;

      case effCanBeAutomated:
         // fix Propellerhead Reason VST parameter support
         r = 1;
         break;

      case effStartProcess:
         r = 1;
         break;

      case effStopProcess:
         r = 1;
         break;

      case effEditIdle:
         printf("vstgltest: redraw window\n");
         // (void)::RedrawWindow(wrapper->hwnd, NULL, NULL, RDW_INTERNALPAINT);
         //(void)::UpdateWindow(wrapper->hwnd);
#ifdef USE_LGLW
         if(lglw_window_is_visible(wrapper->lglw))
         {
            wrapper->redrawWindow();
         }
#endif // USE_LGLW
         break;

      case effEditGetRect:
         // Query editor window geometry
         // ptr: ERect* (on Windows)
         if(NULL != ptr) // yeah, this should never be NULL
         {
            // ...
            wrapper->editor_rect.left   = EDITWIN_X;
            wrapper->editor_rect.top    = EDITWIN_Y;
            wrapper->editor_rect.right  = EDITWIN_X + EDITWIN_W;
            wrapper->editor_rect.bottom = EDITWIN_Y + EDITWIN_H;
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
         // Hide editor window
         wrapper->closeEditor();
         r = 1;
         break;

  }

  return r;
}
}

/**
 * Used for parameter setting (not used by this plugin)
 */
extern "C" {
void VSTPluginSetParameter(VSTPlugin *vstPlugin, VstInt32 index, float parameter)
{
  printf("vstgltest: VSTPluginSetParameter(%d, %f)\n", index, parameter);
  // we can get a hold to our C++ class since we stored it in the `object` field (see constructor)
  VSTPluginWrapper *wrapper = static_cast<VSTPluginWrapper *>(vstPlugin->object);
}
}

/**
 * Used for parameter (not used by this plugin)
 */
extern "C" {
float VSTPluginGetParameter(VSTPlugin *vstPlugin, VstInt32 index)
{
  printf("vstgltest: VSTPluginGetParameter(%d)\n", index);
  // we can get a hold to our C++ class since we stored it in the `object` field (see constructor)
  VSTPluginWrapper *wrapper = static_cast<VSTPluginWrapper *>(vstPlugin->object);
  return 0;
}
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
   _vstPlugin.flags = 
      effFlagsIsSynth | 
      effFlagsCanReplacing | 
      effFlagsCanDoubleReplacing | 
      effFlagsHasEditor
      ;

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

#ifdef USE_LGLW
   printf("vstgltest: calling lglw_init()\n");

   lglw = lglw_init(EDITWIN_W, EDITWIN_H);

   printf("vstgltest: lglw_init() returned lglw=%p\n", lglw);
#endif // USE_LGLW
}

/**
 * Destructor called when the plugin is closed (see VSTPluginDispatcher with effClose opCode). In this very simply plugin
 * there is nothing to do but in general the memory that gets allocated MUST be freed here otherwise there might be a
 * memory leak which may end up slowing down and/or crashing the host
 */
VSTPluginWrapper::~VSTPluginWrapper() {
#ifdef USE_LGLW
   lglw_exit(lglw);
#endif // USE_LGLW
}

/**
 * Implementation of the main entry point of the plugin
 */
VST_EXPORT VSTPlugin *VSTPluginMain(VSTHostCallback vstHostCallback)
{
   printf("vstgltest: entering VSTPluginMain... \n");
   {
      FILE *fh = fopen("/tmp/debug_lglw.txt", "w");
      fprintf(fh, "hello\n");
      fflush(fh);
      fclose(fh);
   }

   {
      Dl_info dlInfo;
      char dllnameraw[1024];
      char *dllnamerawp = dllnameraw;
      char oldCWD[1024];
      getcwd(oldCWD, 1024);
      ::dladdr((void*)VSTPluginMain, &dlInfo);
      if('/' != dlInfo.dli_fname[0])
      {
         // (note) 'dli_fname' can be a relative path (e.g. when loaded from vst2_debug_host)
         sprintf(dllnameraw, "%s/%s", oldCWD, dlInfo.dli_fname);
      }
      else
      {
         // Absolute path (e.g. when loaded from Renoise host)
         dllnamerawp = (char*)dlInfo.dli_fname;
      }
      printf("vstgltest: dllname=\"%s\"\n", dllnamerawp);
   }

   // simply create our plugin C++ class
   VSTPluginWrapper *plugin =
      new VSTPluginWrapper(vstHostCallback,
                           //CCONST('u', 's', 'a', '§'), // registered with Steinberg (http://service.steinberg.de/databases/plugin.nsf/plugIn?openForm)
                           CCONST('t', 'e', 's', 't'), // unregistered
                           PLUGIN_VERSION, // version
                           0,    // no params
                           1,    // no programs
                           0,    // 2 inputs
                           2);   // 2 outputs

   // return the plugin per the contract of the API
   return plugin->getVSTPlugin();
}

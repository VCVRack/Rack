// vst_eureka_standalone_test.cpp : Defines the entry point for the console application.
//

#define DLL_PATH "../../vst2_bin/veeseevstrack_effect.dll"
#define SO_PATH  "../../vst2_bin/veeseevstrack_effect.so"


#include <yac.h>

#ifdef YAC_WIN32
#include "stdafx.h"
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <aeffect.h>
#include <aeffectx.h>

typedef AEffect* (*PluginEntryProc) (audioMasterCallback audioMaster);


static VstIntPtr VSTCALLBACK HostCallback(AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt) {
   static VstInt32 lastOpcode = -1;
   static VstIntPtr lastTimeMask = ~0;
   VstIntPtr result = 0;

   lastOpcode = opcode;

   (void)lastOpcode;
   (void)lastTimeMask;

   return result;
}

float *inputBuffers[48];
float *outputBuffers[48];

void open_and_close(void) {
#ifdef YAC_WIN32
   HINSTANCE dllHandle = ::LoadLibraryA(DLL_PATH);
#else
	void *dllHandle = ::dlopen(SO_PATH, RTLD_NOW);
	if(NULL == dllHandle)
   {
		printf("Failed to load library %s: %s", SO_PATH, dlerror());
		return;
	}
#endif

   for(int i = 0; i < 48; i++)
   {
      inputBuffers[i] = new float[4096];
      outputBuffers[i] = new float[4096];
   }

   if(NULL != dllHandle)
   {
#ifdef YAC_WIN32
      PluginEntryProc mainProc = (PluginEntryProc) ::GetProcAddress((HMODULE)dllHandle, "VSTPluginMain");
      if(NULL == mainProc)
      {
         mainProc = (PluginEntryProc) ::GetProcAddress((HMODULE)dllHandle, "main");
      }
#else
      PluginEntryProc mainProc = (PluginEntryProc) ::dlsym(dllHandle, "VSTPluginMain");
      if(NULL == mainProc)
      {
         mainProc = (PluginEntryProc) ::dlsym(dllHandle, "main");
      }
#endif

      if(NULL != mainProc)
      {
         AEffect *effect;
         printf("xxx calling mainProc\n");
         effect = mainProc(HostCallback);
         printf("xxx mainProc returned effect=%p\n", effect);

         if(NULL != effect)
         {
            printf("xxx calling effect->dispatcher<effOpen>\n");
            effect->dispatcher(effect, effOpen, 0, 0, NULL, 0.0f);
            printf("xxx effect->dispatcher<effOpen> returned\n");
            VstIntPtr ip = effect->dispatcher(effect, effEditOpen, 0, 0, NULL/*hWnd*/, 0.0f);
            (void)ip;
            printf("xxx call processreplacing\n");
            for(int i = 0; i < 1024; i++)
            {
               effect->processReplacing(effect, inputBuffers, outputBuffers, (VstInt32)64);
            }
            effect->dispatcher(effect, effEditClose, 0, 0, NULL, 0.0f);
            effect->dispatcher(effect, effClose, 0, 0, NULL, 0.0f);
         }
      }
      else
      {
         printf("[---] failed to find mainProc\n");
      }

#ifdef YAC_WIN32
      ::FreeLibrary(dllHandle);
#else
      ::dlclose(dllHandle);
#endif
   }

   for(int i = 0; i < 48; i++)
   {
      delete [] inputBuffers[i];
      delete [] outputBuffers[i];
   }
}

 int main() {
    for(;;)
    {
       open_and_close();
    }
   return 0;
}

// vst_eureka_standalone_test.cpp : Defines the entry point for the console application.
//

#define DLL_PATH "../../vst2_bin/veeseevstrack_effect.dll"
// #define SO_PATH  "../../vst2_bin/veeseevstrack_effect.so"
#define SO_PATH  "../vst2_lglw_debug_plugin/debug_lglw.so"


#include <yac.h>

#ifdef YAC_WIN32
#include "stdafx.h"
#include <windows.h>
#else
#include <dlfcn.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#endif

#include <aeffect.h>
#include <aeffectx.h>

typedef AEffect* (*PluginEntryProc) (audioMasterCallback audioMaster);

#ifndef YAC_WIN32
// https://github.com/Ardour/ardour/blob/master/gtk2_ardour/linux_vst_gui_support.cc
long getXWindowProperty(Display* display, Window window, Atom atom)
{
   long result = 0;
   int userSize;
   unsigned long bytes;
   unsigned long userCount;
   unsigned char *data;
   Atom userType;
   // LXVST_xerror = false;

   /*Use our own Xerror handler while we're in here - in an
   attempt to stop the brain dead default Xerror behaviour of
   qutting the entire application because of e.g. an invalid
   window ID*/

   // XErrorHandler olderrorhandler = XSetErrorHandler(TempErrorHandler);

   XGetWindowProperty(display,
                  window,
                  atom,
                  0,
                  2,
                  false,
                  AnyPropertyType,
                  &userType,
                  &userSize,
                  &userCount,
                  &bytes,
                  &data);

   if(userCount == 1)
      result = *(long*)data;

   // XSetErrorHandler(olderrorhandler);

   /*Hopefully this will return zero if the property is not set*/

   return result;
}
#endif


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

#ifndef YAC_WIN32
   Display *d;
   Window w;
   int s;

   d = XOpenDisplay(NULL);
   s = DefaultScreen(d);
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

#ifndef YAC_WIN32
            w = XCreateSimpleWindow(d, RootWindow(d, s), 10, 10, 100, 100, 1,
                                    BlackPixel(d, s), WhitePixel(d, s));
            XSelectInput(d, w, ExposureMask | KeyPressMask);
            XMapRaised(d, w);
            XFlush(d);
#endif

            printf("xxx calling effect->dispatcher<effOpen>\n");
            effect->dispatcher(effect, effOpen, 0, 0, NULL, 0.0f);
            printf("xxx effect->dispatcher<effOpen> returned\n");
#ifdef YAC_WIN32
            VstIntPtr ip = effect->dispatcher(effect, effEditOpen, 0, 0, NULL/*hWnd*/, 0.0f);
#else
            VstIntPtr ip = effect->dispatcher(effect, effEditOpen, 0, 0, (void*)(w)/*hWnd*/, 0.0f);
#endif
            (void)ip;
            sleep(2);

#ifndef YAC_WIN32
            long result = getXWindowProperty(d, w, XInternAtom(d, "_XEventProc", false));
            if(result == 0)
            {
               printf("xxx no XEventProc found\n");
            }
            else
            {
               printf("xxx XEventProc found... calling\n");
               void (* eventProc) (void * event);
               eventProc = (void (*) (void* event))result;
               eventProc(NULL);
            }
#endif

            printf("xxx calling effect->dispatcher<effEditIdle>\n");
            effect->dispatcher(effect, effEditIdle, 0, 0, NULL, 0.0f);
            sleep(1);
            printf("xxx calling effect->dispatcher<effEditIdle>\n");
            effect->dispatcher(effect, effEditIdle, 0, 0, NULL, 0.0f);
            sleep(1);
            printf("xxx calling effect->dispatcher<effEditClose>\n");
            effect->dispatcher(effect, effEditClose, 0, 0, NULL, 0.0f);
            sleep(1);
            printf("xxx calling effect->dispatcher<effEditOpen> again\n");
#ifdef YAC_WIN32
            effect->dispatcher(effect, effEditOpen, 0, 0, NULL/*hWnd*/, 0.0f);
#else
            effect->dispatcher(effect, effEditOpen, 0, 0, (void*)(w)/*hWnd*/, 0.0f);
#endif
            sleep(1);
            printf("xxx calling effect->dispatcher<effEditIdle>\n");
            effect->dispatcher(effect, effEditIdle, 0, 0, NULL, 0.0f);
            sleep(1);
            printf("xxx call processreplacing\n");
            for(int i = 0; i < 1024; i++)
            {
               effect->processReplacing(effect, inputBuffers, outputBuffers, (VstInt32)64);
            }
            printf("xxx calling effect->dispatcher<effEditClose>\n");
            effect->dispatcher(effect, effEditClose, 0, 0, NULL, 0.0f);
            sleep(1);
            printf("xxx calling effect->dispatcher<effClose>\n");
            effect->dispatcher(effect, effClose, 0, 0, NULL, 0.0f);
            sleep(1);

#ifndef YAC_WIN32
            XDestroyWindow(d, w);
#endif
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

#ifndef YAC_WIN32
   XCloseDisplay(d);
#endif
}

 int main() {
    for(int i = 0; i < 5; i++)
    {
       open_and_close();
    }
   return 0;
}

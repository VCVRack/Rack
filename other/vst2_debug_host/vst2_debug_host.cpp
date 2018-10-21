// vst_eureka_standalone_test.cpp : Defines the entry point for the console application.
//

#define DLL_PATH "../../vst2_bin/veeseevstrack_effect.dll"

// #define SO_PATH  "../../vst2_bin/veeseevstrack_effect.so"
// #define SO_PATH  "../vst2_lglw_debug_plugin/debug_lglw.so"
// #define SO_PATH  "/usr/local/lib/vst/debug_lglw.so"
#define SO_PATH  "../../vst2_bin/debug_lglw.so"

// #define SO_PATH  "/home/bsp/.vst/DiscoveryPro68DemoLinux/64-bit/DiscoveryPro64.so"
// #define SO_PATH  "/home/bsp/.vst/AcidBoxDEMO-Linux/AcidBoxDEMOVST-x64.so"
// #define SO_PATH "/home/bsp/.vst/DigitsLinux_2_1/DigitsVST_64.so"


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
void *getXWindowProperty(Display* display, Window window, Atom atom)
{
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

   printf("xxx getXWindowProperty: window=%lu\n", window);

   XGetWindowProperty(display,
                      window,
                      atom,
                      0/*offset*/,
                      2/*length*/,
                      false/*delete*/,
                      AnyPropertyType,
                      &userType/*actual_type_return*/,
                      &userSize/*actual_format_return*/,
                      &userCount/*nitems_return*/,
                      &bytes/*bytes_after_return / partial reads*/,
                      &data);

   union {
      long l[2];
      void *any;
   } uptr;
   uptr.any = 0;

   printf("xxx getXWindowProperty: userSize=%d userCount=%lu bytes=%lu data=%p\n", userSize, userCount, bytes, data);

   if(NULL != data)
   {
      if(userCount == 1)
      {
         // 32-bit
         uptr.l[0] = *(long*)data;
         uptr.l[1] = 0;
      }
      else if(2 == userCount)
      {
         // 64-bit
         uptr.l[0] = ((long*)data)[0];
         uptr.l[1] = ((long*)data)[1];
      }

      XFree(data);
   }

   // XSetErrorHandler(olderrorhandler);

   /*Hopefully this will return zero if the property is not set*/
   printf("xxx getXWindowProperty: return callback addr=%p\n", uptr.any);

   return uptr.any;
}
#endif


static VstIntPtr VSTCALLBACK HostCallback(AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt) {
   static VstInt32 lastOpcode = -1;
   static VstIntPtr lastTimeMask = ~0;
   VstIntPtr result = 0;

   lastOpcode = opcode;

   (void)lastOpcode;
   (void)lastTimeMask;

   switch(opcode)
   {
      default:
         printf("xxx debug_host: HostCallback: unhandled opcode=%d index=%d value=%ld ptr=%p opt=%f\n", opcode, index, value, ptr, opt);
         break;

      case audioMasterVersion:
         result = 2400;
         break;
   }

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
            ERect *rectp = 0;
            ERect rect;
            effect->dispatcher(effect, effEditGetRect, 0, 0, (void*)&rectp, 0.0f);
            if(NULL != rectp)
            {
               rect = *rectp;
            }
            else
            {
               rect.top  = 0;
               rect.left = 0;
               rect.right = 640;
               rect.bottom = 480;
            }
            printf("xxx effEditGetRect returned left=%d top=%d right=%d bottom=%d\n", rect.left, rect.top, rect.right, rect.bottom);

#ifndef YAC_WIN32
            w = XCreateSimpleWindow(d, RootWindow(d, s), rect.left, rect.top, (rect.right - rect.left), (rect.bottom - rect.top), 1,
                                    BlackPixel(d, s), WhitePixel(d, s)
                                    );
            XSelectInput(d, w, ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | PointerMotionMask | ButtonMotionMask | FocusChangeMask);
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
            void *result = getXWindowProperty(d, w, XInternAtom(d, "_XEventProc", false));
            if(result == 0)
            {
               printf("xxx no XEventProc found\n");
            }
            else
            {
               void (* eventProc) (void * event);
               int evIdx = 0;
               eventProc = (void (*) (void* event))result;
               printf("xxx XEventProc found\n");
               for(;;)
               {
                  XEvent xev;
                  XNextEvent(d, &xev);
                  printf("xxx call XEventProc[%d]\n", evIdx++);
                  eventProc(&xev);
               }
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

      printf("xxx debug_host: closing library\n");

#ifdef YAC_WIN32
      ::FreeLibrary(dllHandle);
#else
      ::dlclose(dllHandle);
#endif
      printf("xxx debug_host: library closed\n");
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
   printf("xxx debug_host: exiting\n");
   return 0;
}

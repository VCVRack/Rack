// vst_eureka_standalone_test.cpp : Defines the entry point for the console application.
//

#define DLL_PATH "../../vst2_bin/veeseevstrack_effect.dll"

#define SO_PATH  "../../vst2_bin/veeseevstrack_effect.so"
// #define SO_PATH  "../vst2_lglw_debug_plugin/debug_lglw.so"
// #define SO_PATH  "/usr/local/lib/vst/debug_lglw.so"
// #define SO_PATH  "../../vst2_bin/debug_lglw.so"

// #define SO_PATH  "/home/bsp/.vst/DiscoveryPro68DemoLinux/64-bit/DiscoveryPro64.so"
// #define SO_PATH  "/home/bsp/.vst/AcidBoxDEMO-Linux/AcidBoxDEMOVST-x64.so"
// #define SO_PATH "/home/bsp/.vst/DigitsLinux_2_1/DigitsVST_64.so"
// #define SO_PATH "/usr/lib/lxvst/helm.so"
// #define SO_PATH "/home/bsp/.vst/oxevst134/oxevst64.so"  // crashes VirtualBox (!)
// #define SO_PATH "/home/bsp/.vst/tunefish-v4.2.0-linux64-vst24.tar/Tunefish4.so"  // does not load (GLIBC error)
// #define SO_PATH "/home/bsp/.vst/zyn-fusion/ZynAddSubFX.so"

#define Dprintf_verbose if(1);else printf
// #define Dprintf_verbose if(0);else printf

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
static void *loc_getProperty(Display *_display, Window _window, const char *_name) {
   int userSize;
   unsigned long bytes;
   unsigned long userCount;
   unsigned char *data;
   Atom userType;
   Atom atom = XInternAtom(_display, _name, False);

   // (note) 64bit properties need to be read with two XGetWindowProperty() calls.
   //         When using just one call and setting the 'length' to 2, the upper 32bit (second array element) will be 0xFFFFffff.
   XGetWindowProperty(_display,
                      _window,
                      atom,
                      0/*offset*/,
                      1/*length*/,
                      False/*delete*/,
                      AnyPropertyType,
                      &userType/*actual_type_return*/,
                      &userSize/*actual_format_return*/,
                      &userCount/*nitems_return*/,
                      &bytes/*bytes_after_return / partial reads*/,
                      &data);

   union {
      uint32_t ui[2];
      void *any;
   } uptr;
   uptr.any = 0;

   Dprintf_verbose("xxx debug_host: loc_getProperty: LOWER userSize=%d userCount=%lu bytes=%lu data=%p\n", userSize, userCount, bytes, data);

   if(NULL != data)
   {
      if(userCount >= 1)
      {
         // lower 32-bit
         uptr.ui[0] = *(long*)data;
         uptr.ui[1] = 0;

         Dprintf_verbose("xxx     lower=0x%08x\n", uptr.ui[0]);

         XFree(data);

         XGetWindowProperty(_display,
                            _window,
                            atom,
                            1/*offset*/,
                            1/*length*/,
                            False/*delete*/,
                            AnyPropertyType,
                            &userType/*actual_type_return*/,
                            &userSize/*actual_format_return*/,
                            &userCount/*nitems_return*/,
                            &bytes/*bytes_after_return / partial reads*/,
                            &data);

         // printf("xxx debug_host: loc_getProperty: UPPER userSize=%d userCount=%lu bytes=%lu data=%p\n", userSize, userCount, bytes, data);
         if(NULL != data)
         {
            // upper 32-bit
            uptr.ui[1] = *(long*)data;
            Dprintf_verbose("xxx     upper=0x%08x\n", uptr.ui[1]);
            XFree(data);
         }
      }
   }

   Dprintf_verbose("xxx debug_host: loc_getProperty: return value=%p\n", uptr.any);

   return uptr.any;
}
#endif  // !YAC_WIN32


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
            printf("xxx calling effect->dispatcher<effOpen>\n");
            effect->dispatcher(effect, effOpen, 0, 0, NULL, 0.0f);
            printf("xxx effect->dispatcher<effOpen> returned\n");

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

            // see <https://stackoverflow.com/questions/1157364/intercept-wm-delete-window-on-x11>
            Atom wm_delete_window;
            {
               wm_delete_window = XInternAtom(d, "WM_DELETE_WINDOW", False);
               XSetWMProtocols(d, w, &wm_delete_window, 1);
            }

            XMapRaised(d, w);
            XFlush(d);
#endif

#if 0
            // Event test (=> keyboard events are working)
            {
               int evIdx = 0;
               for(;;)
               {
                  XEvent xev;
                  XNextEvent(d, &xev);
                  printf("xxx XNextEvent[%d]\n", evIdx++);
               }
            }
#endif

#ifdef YAC_WIN32
            VstIntPtr ip = effect->dispatcher(effect, effEditOpen, 0, 0, NULL/*hWnd*/, 0.0f);
#else
            VstIntPtr ip = effect->dispatcher(effect, effEditOpen, 0, 0, (void*)(w)/*hWnd*/, 0.0f);
            effect->dispatcher(effect, effEditIdle, 0, 0, NULL, 0.0f); // test feedback loop
#endif
            (void)ip;
            sleep(2);

#ifndef YAC_WIN32
            void *result = loc_getProperty(d, w, "_XEventProc");
            if(result == 0)
            {
               printf("xxx no XEventProc found, running effEditIdle instead\n");
               bool bRunning = true;
               while(bRunning)
               {
                  XEvent xev;
                  int queued = XPending(d);
                  // printf("xxx =====================================================\n");
                  // printf("xxx checking host queue before effEditIdle (events: %i)\n", queued);
                  while(queued)
                  {
                     XNextEvent(d, &xev);
                     // printf("xxx event type: %i\n", xev.type);
                     queued--;
                  }

                  // printf("xxx calling effect->dispatcher<effEditIdle>\n");
                  effect->dispatcher(effect, effEditIdle, 0, 0, NULL, 0.0f);

                  queued = XPending(d);
                  // printf("xxx checking host queue after effEditIdle (events: %i)\n", queued);
                  while((queued > 0) && bRunning)
                  {
                     XNextEvent(d, &xev);

                     // printf("xxx debug_host: xev.type=%d\n", xev.type);

                     if(ClientMessage == xev.type)
                     {
                        printf("xxx debug_host: ClientMessage\n");
                        if((Atom)xev.xclient.data.l[0] == wm_delete_window)
                        {
                           printf("xxx debug_host: ClientMessage<wm_delete_window>\n");
                           bRunning = false;
                        }
                     }

                     // if(MotionNotify != xev.type)
                     //    printf("xxx event type: %i\n", xev.type);

                     queued--;
                  }

                  // sleep(1); // can be helpful when viewing print statements
               }
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
                  Dprintf_verbose("xxx call XEventProc[%d]\n", evIdx++);
                  eventProc(&xev);

                  // Dprintf_verbose("xxx calling effect->dispatcher<effEditIdle>\n");
                  // effect->dispatcher(effect, effEditIdle, 0, 0, NULL, 0.0f);
               }
            }
#endif

            sleep(1);
            printf("xxx calling effect->dispatcher<effEditClose>\n");
            effect->dispatcher(effect, effEditClose, 0, 0, NULL, 0.0f);

#if 0
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
#endif

            printf("xxx call processreplacing\n");
            for(int i = 0; i < 1024; i++)
            {
               effect->processReplacing(effect, inputBuffers, outputBuffers, (VstInt32)64);
            }

#if 0
            printf("xxx calling effect->dispatcher<effEditClose>\n");
            effect->dispatcher(effect, effEditClose, 0, 0, NULL, 0.0f);
            sleep(1);
#endif
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
   for(int i = 0; i < 2; i++)
   {
      open_and_close();
   }
   printf("xxx debug_host: exiting\n");
   return 0;
}

/* ----
 * ---- file   : lglw_linux.c  (**stub**)
 * ---- author : bsp
 * ---- legal  : Distributed under terms of the MIT LICENSE (MIT).
 * ----
 * ---- Permission is hereby granted, free of charge, to any person obtaining a copy
 * ---- of this software and associated documentation files (the "Software"), to deal
 * ---- in the Software without restriction, including without limitation the rights
 * ---- to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * ---- copies of the Software, and to permit persons to whom the Software is
 * ---- furnished to do so, subject to the following conditions:
 * ----
 * ---- The above copyright notice and this permission notice shall be included in
 * ---- all copies or substantial portions of the Software.
 * ----
 * ---- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * ---- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * ---- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * ---- AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * ---- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * ---- OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * ---- THE SOFTWARE.
 * ----
 * ---- info   : This is part of the "lglw" package.
 * ----
 * ---- created: 04Aug2018
 * ---- changed: 05Aug2018, 06Aug2018, 07Aug2018, 08Aug2018, 09Aug2018, 18Aug2018, 10Oct2018
 * ----          16Oct2018, 19May2019
 * ----
 * ----
 */

// #define USE_XEVENTPROC defined

#include "lglw.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>

#include <GL/gl.h>
#include <GL/glx.h>

#ifdef ARCH_X64
#include <sys/mman.h>
#include <unistd.h>
#endif // ARCH_X64


#define LOG_FXN  printf
// #define LOG_FXN  lglw_log

//
// Regular log entry (low frequency)
//
#define Dlog if(1);else LOG_FXN

//
// Verbose log entry
//
#define Dlog_v if(1);else LOG_FXN

//
// Very-verbose log entry
//
#define Dlog_vv if(1);else LOG_FXN

//
// Very-very-verbose log entry
//
#define Dlog_vvv if(1);else LOG_FXN

// ---------------------------------------------------------------------------- macros and defines
#define LGLW(a) lglw_int_t *lglw = ((lglw_int_t*)(a))

#define LGLW_DEFAULT_HIDDEN_W  (800)
#define LGLW_DEFAULT_HIDDEN_H  (600)

#define LGLW_MOUSE_TOUCH_LMB_TIMEOUT   (250u)
#define LGLW_MOUSE_TOUCH_RMB_TIMEOUT   (500u)
#define LGLW_MOUSE_TOUCH_RMB_STATE_IDLE  (0u)
#define LGLW_MOUSE_TOUCH_RMB_STATE_LMB   (1u)
#define LGLW_MOUSE_TOUCH_RMB_STATE_WAIT  (2u)
#define LGLW_MOUSE_TOUCH_RMB_STATE_RMB   (3u)
#define LGLW_MOUSE_TOUCH_RMB_MOVE_THRESHOLD  (7u)

#define sABS(x) (((x)>0)?(x):-(x))


// ---------------------------------------------------------------------------- structs and typedefs
typedef struct lglw_int_s {
   void        *user_data;    // arbitrary user data
   Display     *xdsp;
   XVisualInfo *vi;
   Colormap     cmap;
   Window       parent_xwnd;  // created by host

   struct {
      lglw_vec2i_t size;
      Window       xwnd;
   } hidden;

   struct {
      lglw_vec2i_t size;
      Window       xwnd;
      lglw_bool_t  mapped;
      int32_t      swap_interval;
      lglw_bool_t  b_owner;
   } win;

   GLXContext   ctx;

   struct {
      GLXContext   ctx;
      GLXDrawable  drw;
   } prev;

   struct {
      uint32_t            kmod_state;  // See LGLW_KMOD_xxx
      lglw_keyboard_fxn_t cbk;
   } keyboard;

   struct {
      lglw_vec2i_t     p;  // last seen mouse position
      uint32_t         button_state;
      lglw_mouse_fxn_t cbk;
      struct {
         uint32_t         mode;
         lglw_vec2i_t     p;  // grab-start mouse position
         lglw_bool_t      b_queue_warp;
         lglw_vec2i_t     last_p;
      } grab;
      struct {
         lglw_bool_t      b_enable;
         lglw_bool_t      b_update_queued;
         lglw_bool_t      b_syn_rmb;
         uint32_t         syn_rmb_hold_state;  // see LGLW_MOUSE_TOUCH_RMB_STATE_xxx
         uint32_t         hold_start_ms;
         lglw_vec2i_t     hold_start_p;
      } touch;
   } mouse;

   struct {
      uint32_t         state;
      lglw_focus_fxn_t cbk;
   } focus;

   struct {
      lglw_bool_t      b_running;
      lglw_timer_fxn_t cbk;
      struct timeval   tv_start;
      uint32_t         interval_ms;
      uint32_t         last_ms;
   } timer;

   struct {
      uint32_t numChars;
      char *data;
   } clipboard;

   struct {
      lglw_dropfiles_fxn_t cbk;
   } dropfiles;

   struct {
      lglw_redraw_fxn_t cbk;
   } redraw;

} lglw_int_t;


// ---------------------------------------------------------------------------- module fxn fwd decls
static lglw_bool_t loc_create_hidden_window (lglw_int_t *lglw, int32_t _w, int32_t _h);
static void loc_destroy_hidden_window(lglw_int_t *lglw);

static lglw_bool_t loc_handle_key (lglw_int_t *lglw, lglw_bool_t _bPressed, uint32_t _vkey);
// static lglw_bool_t loc_touchkeyboard_get_rect (RECT *rect);
// static lglw_bool_t loc_touchkeyboard_is_visible (void);
extern lglw_bool_t lglw_int_touchkeyboard_toggle (void);

static void loc_handle_mouseleave (lglw_int_t *lglw);
static void loc_handle_mouseenter (lglw_int_t *lglw);
static void loc_handle_mousebutton (lglw_int_t *lglw, lglw_bool_t _bPressed, uint32_t _button);
static void loc_handle_mousemotion (lglw_int_t *lglw);
static void loc_handle_queued_mouse_warp (lglw_int_t *lglw);

static void loc_enable_dropfiles (lglw_int_t *lglw, lglw_bool_t _bEnable);

static void loc_eventProc (XEvent *xev, lglw_int_t *lglw);
static void loc_XEventProc (void *_xevent);
static void loc_setProperty (Display *_display, Window _window, const char *_name, void *_value);
static void *loc_getProperty (Display *_display, Window _window, const char *_name);
static void loc_setEventProc (Display *display, Window window);

static void loc_millisec_init (lglw_int_t *lglw);
static uint32_t loc_millisec_delta (lglw_int_t *lglw);  // return millisec since init()
static void loc_process_timer (lglw_int_t *lglw);


// ---------------------------------------------------------------------------- lglw_millisec_init
static void loc_millisec_init (lglw_int_t *lglw) {
   gettimeofday(&lglw->timer.tv_start, 0);
}

// ---------------------------------------------------------------------------- lglw_millisec_delta
static uint32_t loc_millisec_delta (lglw_int_t *lglw) {
   struct timeval c; gettimeofday(&c, 0);
   const struct timeval*s =  &lglw->timer.tv_start;
   return (uint32_t) ( (c.tv_sec-s->tv_sec)*1000 + (c.tv_usec-s->tv_usec)/1000 );
}


// TODO: remove and/or improve debug logging for a debug build
// ---------------------------------------------------------------------------- lglw_log
static FILE *logfile;

void lglw_log(const char *logData, ...) {
   static char buf[16*1024]; 
   va_list va; 
   va_start(va, logData); 
   vsprintf(buf, logData, va);
   va_end(va); 
   printf(buf); 
   //fprintf(logfile, logData);
   fputs(buf, logfile);
   fflush(logfile);
   // printf(logData);
}


// TODO: remove, or maybe not in some specific use cases
// ---------------------------------------------------------------------------- xerror_log
static int xerror_handler(Display *display, XErrorEvent *error) {
   char error_text[1024];
   XGetErrorText(display, error->error_code, error_text, 1024);
   Dlog("XERROR (%d): %s, %d, %d\n", error->error_code, error_text, error->request_code, error->minor_code);
   return 0;
}


// ---------------------------------------------------------------------------- lglw_init
lglw_t lglw_init(int32_t _w, int32_t _h) {
   lglw_int_t *lglw = malloc(sizeof(lglw_int_t));

   // TODO: remove/improve
   logfile = fopen("/tmp/lglw_log.txt", "w");
   XSetErrorHandler(xerror_handler);
   XInitThreads();  // fix GL crash, see <https://forum.juce.com/t/linux-vst-opengl-crash-because-xinitthreads-not-called/22821>

   loc_millisec_init(lglw);

   if(NULL != lglw)
   {
      memset(lglw, 0, sizeof(lglw_int_t));

      Dlog("lglw:lglw_init: 1\n");
      if(_w <= 16)
         _w = LGLW_DEFAULT_HIDDEN_W;

      if(_h <= 16)
         _h = LGLW_DEFAULT_HIDDEN_H;

      Dlog("lglw:lglw_init: 2\n");
      if(!loc_create_hidden_window(lglw, _w, _h))
      {
         free(lglw);
         lglw = NULL;
      }
      Dlog("lglw:lglw_init: 3\n");
   }

   Dlog("lglw:lglw_init: EXIT\n");

   return lglw;
}


// ---------------------------------------------------------------------------- lglw_exit
void lglw_exit(lglw_t _lglw) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      Dlog("lglw:lglw_exit: 1\n");

      loc_destroy_hidden_window(lglw);

      Dlog("lglw:lglw_exit: 2\n");

      fclose(logfile);
      free(lglw);
   }
}


// ---------------------------------------------------------------------------- lglw_userdata_set
void lglw_userdata_set(lglw_t _lglw, void *_userData) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      Dlog_vvv("lglw:lglw_userdata_set: ptr=%p\n", _userData);
      lglw->user_data = _userData;
   }
}

// ---------------------------------------------------------------------------- lglw_userdata_get
void *lglw_userdata_get(lglw_t _lglw) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      Dlog_vvv("lglw:lglw_userdata_get: return ptr=%p\n", lglw->user_data);
      return lglw->user_data;
   }

   return NULL;
}

// ---------------------------------------------------------------------------- lglw_userdata_get
void loc_create_gl(lglw_int_t *lglw) {
   lglw->ctx = glXCreateContext(lglw->xdsp, lglw->vi, None, True);
}

// ---------------------------------------------------------------------------- lglw_userdata_get
void loc_destroy_gl(lglw_int_t *lglw) {
   if(NULL != lglw->xdsp && NULL != lglw->ctx)
   {
      glXMakeCurrent(lglw->xdsp, None, NULL);
      glXDestroyContext(lglw->xdsp, lglw->ctx);
   }
}


// ---------------------------------------------------------------------------- loc_create_hidden_window
static lglw_bool_t loc_create_hidden_window(lglw_int_t *lglw, int32_t _w, int32_t _h) {

   // TODO: compare to 'WindowClass' from Windows implementation

   Dlog_v("lglw:loc_create_hidden_window: 1\n");
   XSetWindowAttributes swa;
   int attrib[] = { GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 24, None };
   int screen;

   Dlog_v("lglw:loc_create_hidden_window: 2\n");
   lglw->xdsp = XOpenDisplay(NULL);
   screen = DefaultScreen(lglw->xdsp);

   Dlog_v("lglw:loc_create_hidden_window: 3\n");
   lglw->vi = glXChooseVisual(lglw->xdsp, screen, attrib);

   Dlog_v("lglw:loc_create_hidden_window: 4\n");
   if(NULL == lglw->vi)
   {
      Dlog("[---] lglw: failed to find GLX Visual for hidden window\n");
      return LGLW_FALSE;
   }

   Dlog_v("lglw:loc_create_hidden_window: 5\n");
   loc_create_gl(lglw);

   if(NULL == lglw->ctx)
   {
      Dlog("lglw: FAILED to create context!!!\n");
   }

   Dlog_v("lglw:loc_create_hidden_window: 6\n");
   if(NULL == lglw->ctx)
   {
      Dlog("[---] lglw: failed to create GLX Context for hidden window\n");
      return LGLW_FALSE;
   }

   Dlog_v("lglw:loc_create_hidden_window: 7\n");
   lglw->cmap = XCreateColormap(lglw->xdsp, RootWindow(lglw->xdsp, lglw->vi->screen),
                                lglw->vi->visual, AllocNone);

   Dlog_v("lglw:loc_create_hidden_window: 8\n");
   swa.border_pixel = 0;
   swa.colormap = lglw->cmap;
   lglw->hidden.xwnd = XCreateWindow(lglw->xdsp, DefaultRootWindow(lglw->xdsp),
                                     0, 0, LGLW_DEFAULT_HIDDEN_W, LGLW_DEFAULT_HIDDEN_H, 0, CopyFromParent, InputOutput,
                                     lglw->vi->visual, CWBorderPixel | CWColormap, &swa);

   Dlog_v("lglw:loc_create_hidden_window: 9\n");
   XSetStandardProperties(lglw->xdsp, lglw->hidden.xwnd, "LGLW_hidden", "LGLW_hidden", None, NULL, 0, NULL);
   XSync(lglw->xdsp, False);

   Dlog_v("lglw:loc_create_hidden_window: EXIT\n");
   lglw->hidden.size.x = _w;
   lglw->hidden.size.y = _h;

   return LGLW_TRUE;
}


// ---------------------------------------------------------------------------- loc_destroy_hidden_window
static void loc_destroy_hidden_window(lglw_int_t *lglw) {
   Dlog_v("lglw:loc_destroy_hidden_window: 1\n");
   loc_destroy_gl(lglw);
   Dlog_v("lglw:loc_destroy_hidden_window: 2\n");
   if(NULL != lglw->xdsp && 0 != lglw->hidden.xwnd) XDestroyWindow(lglw->xdsp, lglw->hidden.xwnd);
   Dlog_v("lglw:loc_destroy_hidden_window: 3\n");
   if(NULL != lglw->xdsp && 0 != lglw->cmap) XFreeColormap(lglw->xdsp, lglw->cmap);
   Dlog_v("lglw:loc_destroy_hidden_window: 4\n");
   if(NULL != lglw->vi) XFree(lglw->vi);

   Dlog_v("lglw:loc_destroy_hidden_window: 5\n");
   XSync(lglw->xdsp, False);
   if(NULL != lglw->xdsp) XCloseDisplay(lglw->xdsp);
}


// ---------------------------------------------------------------------------- loc_setEventProc
// https://www.kvraudio.com/forum/viewtopic.php?t=387924
// https://github.com/Ardour/ardour/blob/master/gtk2_ardour/linux_vst_gui_support.cc
// https://discourse.ardour.org/t/overtonedsp-plugins/90115/22
// https://github.com/amsynth/amsynth/blob/4a87798e650c6d71d70274a961c9b8d98fc6da7e/src/amsynth_vst.cpp
// https://github.com/rsenn/eXT2/blob/7f00a09561ded8175ffed2f4912dad74e466a1c7/vstplugins/vstgui/vstgui.cpp
// https://github.com/COx2/DistortionFilter/blob/c6a34fb56b503a6e95bf0975e00f438bbf4ff52a/juce/modules/juce_audio_processors/format_types/juce_VSTPluginFormat.cpp

// Very simple function to test _XEventProc is properly called
static void loc_eventProc(XEvent *xev, lglw_int_t *lglw) {

   Dlog_vvv("lglw:loc_eventProc: type=%d serial=%lu send_event=%d lglw=%p\n", xev->xany.type, xev->xany.serial, xev->xany.send_event, lglw);

   loc_process_timer(lglw);

   if(NULL != lglw)
   {
      lglw_bool_t eventHandled = LGLW_FALSE;

      switch(xev->type)
      {
         default:
            Dlog("lglw:loc_eventProc: unhandled X11 event type=%d\n", xev->type);
            eventHandled = LGLW_FALSE;
            break;

         case Expose:
            Dlog_vvv("lglw:loc_eventProc: xev Expose\n");
            loc_handle_queued_mouse_warp(lglw);
            eventHandled = LGLW_FALSE;
            if(NULL != lglw->redraw.cbk)
            {
               lglw->redraw.cbk(lglw);
               eventHandled = LGLW_TRUE;
            }
            break;

            // TODO: Should FocusIn/Out be treated like WM_CAPTURECHANGED and reset the grab state?

         case FocusIn:
            Dlog_v("lglw:loc_eventProc: xev FocusIn\n");
            eventHandled = LGLW_FALSE;
            break;

         case FocusOut:
            Dlog_v("lglw:loc_eventProc: xev FocusOut\n");
            eventHandled = LGLW_FALSE;
            break;

         case EnterNotify:
            // Dlog_v("lglw:loc_eventProc: xev XEnterWindowEvent\n");
            ; // empty statement
            XEnterWindowEvent *wenter = (XEnterWindowEvent*)xev;
            Dlog_v("lglw:loc_eventProc: xev EnterNotify: mode:%i, detail:%i, state:%d\n", wenter->mode, wenter->detail, wenter->state);
            lglw->mouse.p.x = wenter->x;
            lglw->mouse.p.y = wenter->y;
            loc_handle_mousemotion(lglw);

            // EnterNotify messages can be pseudo-motion events (NotifyGrab, NotifyUngrab)
            // when buttons are pressed, which would trigger false focus changes
            // so, the callback is only sent when a normal entry happens
            if (wenter->mode == NotifyNormal)
            {
               loc_handle_mouseenter(lglw);
            }
            eventHandled = LGLW_TRUE;

            break;

         case LeaveNotify:
            // Dlog_v("lglw:loc_eventProc: xev XLeaveWindowEvent\n");
            ; // empty statement
            XLeaveWindowEvent *wexit = (XLeaveWindowEvent*)xev;
            Dlog_v("lglw:loc_eventProc: xev LeaveNotify: mode:%i, detail:%i, state:%d\n", wexit->mode, wexit->detail, wexit->state);

            // LeaveNotify messages can be pseudo-motion events (NotifyGrab, NotifyUngrab)
            // when buttons are pressed, which would trigger false focus changes
            // so, the callback is only sent when a normal entry happens
            if (wexit->mode == NotifyNormal)
            {
               loc_handle_mouseleave(lglw);
            }
            eventHandled = LGLW_TRUE;

            break;

         case MotionNotify:
            Dlog_vvv("lglw:loc_eventProc: xev MotionNotify\n");
            ; // empty statement
            XMotionEvent *motion = (XMotionEvent*)xev;

            if(LGLW_MOUSE_GRAB_WARP == lglw->mouse.grab.mode)
            {
               lglw->mouse.grab.b_queue_warp = LGLW_TRUE;

               lglw->mouse.p.x += (motion->x - lglw->mouse.grab.last_p.x);
               lglw->mouse.p.y += (motion->y - lglw->mouse.grab.last_p.y);

               lglw->mouse.grab.last_p.x = motion->x;
               lglw->mouse.grab.last_p.y = motion->y;
            }
            else
            {
               lglw->mouse.p.x = motion->x;
               lglw->mouse.p.y = motion->y;
            }

            loc_handle_mousemotion(lglw);
            eventHandled = LGLW_TRUE;

            break;

         case KeyPress:
            Dlog("lglw:loc_eventProc: xev KeyPress\n");
            XKeyPressedEvent *keyPress = (XKeyPressedEvent*)xev;

            eventHandled = LGLW_FALSE;
            KeySym xkp = XLookupKeysym(keyPress, 0);
            switch(xkp)
            {
               default:
                  Dlog("lglw:loc_eventProc: xev KeyPress: %x or %lu\n", keyPress->keycode, xkp);
                  if(0u != (lglw->keyboard.kmod_state & LGLW_KMOD_SHIFT))
                  {
                     KeySym xkpl;
                     KeySym xkpu;
                     XConvertCase(xkp, &xkpl, &xkpu);
                     eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, xkpu);
                  }
                  else
                  {
                     eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, xkp);
                  }
                  break;

               case NoSymbol:
                  Dlog("lglw:loc_eventProc: xev UNKNOWN KeyPress: %x\n", keyPress->keycode);
                  break;

               case XK_Left:
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_LEFT);
                  break;

               case XK_Right:
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_RIGHT);
                  break;

               case XK_Up:
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_UP);
                  break;

               case XK_Down:
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_DOWN);
                  break;

               case XK_Insert:
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_INSERT);
                  break;

               case XK_Delete:
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_DELETE);
                  break;

               case XK_Home:
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_HOME);
                  break;

               case XK_End:
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_END);
                  break;

               case XK_Prior:
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_PAGEUP);
                  break;

               case XK_Next:
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_PAGEDOWN);
                  break;

               case XK_F1:
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_F1);
                  break;

               case XK_F2:
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_F2);
                  break;

               case XK_F3:
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_F3);
                  break;

               case XK_F4:
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_F4);
                  break;

               case XK_F5:
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_F5);
                  break;

               case XK_F6:
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_F6);
                  break;

               case XK_F7:
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_F7);
                  break;

               case XK_F8:
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_F8);
                  break;

               case XK_F9:
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_F9);
                  break;

               case XK_F10:
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_F10);
                  break;

               case XK_F11:
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_F11);
                  break;

               case XK_F12:
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_F12);
                  break;

               case XK_BackSpace:
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_BACKSPACE);
                  break;

               case XK_Tab:
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_TAB);
                  break;

               case XK_Return:
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_RETURN);
                  break;

               case XK_Escape:
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_ESCAPE);
                  break;

               case XK_Shift_L:
                  lglw->keyboard.kmod_state |= LGLW_KMOD_LSHIFT;
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_LSHIFT);
                  eventHandled = LGLW_FALSE;
                  break;

               case XK_Shift_R:
                  lglw->keyboard.kmod_state |= LGLW_KMOD_RSHIFT;
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_RSHIFT);
                  eventHandled = LGLW_FALSE;
                  break;

               case XK_Control_L:
                  lglw->keyboard.kmod_state |= LGLW_KMOD_LCTRL;
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_KMOD_LCTRL);
                  eventHandled = LGLW_FALSE;
                  break;

               case XK_Control_R:
                  lglw->keyboard.kmod_state |= LGLW_KMOD_RCTRL;
                  eventHandled = loc_handle_key(lglw, LGLW_TRUE/*bPressed*/, LGLW_KMOD_RCTRL);
                  eventHandled = LGLW_FALSE;
                  break;
            }

            break;

         case KeyRelease:
            Dlog("lglw:loc_eventProc: xev KeyRelease\n");
            XKeyReleasedEvent *keyRelease = (XKeyReleasedEvent*)xev;

            eventHandled = LGLW_FALSE;
            KeySym xkr = XLookupKeysym(keyRelease, 0);
            switch(xkr)
            {
               default:
                  Dlog("lglw:loc_eventProc: xev KeyRelease: %x or %lu\n", keyRelease->keycode, xkr);
                  if(0u != (lglw->keyboard.kmod_state & LGLW_KMOD_SHIFT))
                  {
                     KeySym xkrl;
                     KeySym xkru;
                     XConvertCase(xkr, &xkrl, &xkru);
                     eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, xkru);
                  }
                  else
                  {
                     eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, xkr);
                  }
                  break;

               case NoSymbol:
                  Dlog("lglw:loc_eventProc: xev UNKNOWN KeyRelease: %x\n", keyRelease->keycode);
                  break;

               case XK_Left:
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_LEFT);
                  break;

               case XK_Right:
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_RIGHT);
                  break;

               case XK_Up:
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_UP);
                  break;

               case XK_Down:
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_DOWN);
                  break;

               case XK_Insert:
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_INSERT);
                  break;

               case XK_Delete:
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_DELETE);
                  break;

               case XK_Home:
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_HOME);
                  break;

               case XK_End:
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_END);
                  break;

               case XK_Prior:
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_PAGEUP);
                  break;

               case XK_Next:
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_PAGEDOWN);
                  break;

               case XK_F1:
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_F1);
                  break;

               case XK_F2:
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_F2);
                  break;

               case XK_F3:
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_F3);
                  break;

               case XK_F4:
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_F4);
                  break;

               case XK_F5:
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_F5);
                  break;

               case XK_F6:
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_F6);
                  break;

               case XK_F7:
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_F7);
                  break;

               case XK_F8:
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_F8);
                  break;

               case XK_F9:
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_F9);
                  break;

               case XK_F10:
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_F10);
                  break;

               case XK_F11:
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_F11);
                  break;

               case XK_F12:
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_F12);
                  break;

               case XK_BackSpace:
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_BACKSPACE);
                  break;

               case XK_Tab:
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_TAB);
                  break;

               case XK_Return:
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_RETURN);
                  break;

               case XK_Escape:
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_ESCAPE);
                  break;

               case XK_Shift_L:
                  lglw->keyboard.kmod_state &= ~LGLW_KMOD_LSHIFT;
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_LSHIFT);
                  eventHandled = LGLW_FALSE;
                  break;

               case XK_Shift_R:
                  lglw->keyboard.kmod_state &= ~LGLW_KMOD_RSHIFT;
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_RSHIFT);
                  eventHandled = LGLW_FALSE;
                  break;

               case XK_Control_L:
                  lglw->keyboard.kmod_state &= ~LGLW_KMOD_LCTRL;
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_LCTRL);
                  eventHandled = LGLW_FALSE;
                  break;

               case XK_Control_R:
                  lglw->keyboard.kmod_state &= ~LGLW_KMOD_RCTRL;
                  eventHandled = loc_handle_key(lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_RCTRL);
                  eventHandled = LGLW_FALSE;
                  break;
            }

            break;

         case ButtonPress:
            Dlog("lglw:loc_eventProc: xev ButtonPress\n");
            XButtonPressedEvent *btnPress = (XButtonPressedEvent*)xev;
            lglw->mouse.p.x = btnPress->x;
            lglw->mouse.p.y = btnPress->y;

            if(0u == (lglw->focus.state & LGLW_FOCUS_MOUSE))
            {
               loc_handle_mouseenter(lglw);
            }

            switch(btnPress->button)
            {
               default:
                  Dlog("lglw:loc_eventProc: xev ButtonPress unhandled button: %i\n", btnPress->button);
                  eventHandled = LGLW_FALSE;
                  break;
               case Button1:
                  loc_handle_mousebutton(lglw, LGLW_TRUE/*bPressed*/, LGLW_MOUSE_LBUTTON);
                  eventHandled = LGLW_TRUE;
                  break;
               case Button2:
                  loc_handle_mousebutton(lglw, LGLW_TRUE/*bPressed*/, LGLW_MOUSE_MBUTTON);
                  eventHandled = LGLW_TRUE;
                  break;
               case Button3:
                  loc_handle_mousebutton(lglw, LGLW_TRUE/*bPressed*/, LGLW_MOUSE_RBUTTON);
                  eventHandled = LGLW_TRUE;
                  break;
               case Button4:
                  loc_handle_mousebutton(lglw, LGLW_TRUE/*bPressed*/, LGLW_MOUSE_WHEELUP);
                  eventHandled = LGLW_TRUE;
                  break;
               case Button5:
                  loc_handle_mousebutton(lglw, LGLW_TRUE/*bPressed*/, LGLW_MOUSE_WHEELDOWN);
                  eventHandled = LGLW_TRUE;
                  break;
            }
            break;

         case ButtonRelease:
            Dlog("lglw:loc_eventProc: xev ButtonRelease\n");
            XButtonReleasedEvent *btnRelease = (XButtonReleasedEvent*)xev;
            lglw->mouse.p.x = btnRelease->x;
            lglw->mouse.p.y = btnRelease->y;
            switch(btnRelease->button)
            {
               default:
                  Dlog("lglw:loc_eventProc: xev ButtonRelease unhandled button: %i\n", btnRelease->button);
                  eventHandled = LGLW_FALSE;
                  break;
               case Button1:
                  loc_handle_mousebutton(lglw, LGLW_FALSE/*bPressed*/, LGLW_MOUSE_LBUTTON);
                  eventHandled = LGLW_TRUE;
                  break;
               case Button2:
                  loc_handle_mousebutton(lglw, LGLW_FALSE/*bPressed*/, LGLW_MOUSE_MBUTTON);
                  eventHandled = LGLW_TRUE;
                  break;
               case Button3:
                  loc_handle_mousebutton(lglw, LGLW_FALSE/*bPressed*/, LGLW_MOUSE_RBUTTON);
                  eventHandled = LGLW_TRUE;
                  break;
               case Button4:
                  loc_handle_mousebutton(lglw, LGLW_FALSE/*bPressed*/, LGLW_MOUSE_WHEELUP);
                  eventHandled = LGLW_TRUE;
                  break;
               case Button5:
                  loc_handle_mousebutton(lglw, LGLW_FALSE/*bPressed*/, LGLW_MOUSE_WHEELDOWN);
                  eventHandled = LGLW_TRUE;
                  break;
            }
            break;

         case SelectionClear:
            Dlog("lglw:loc_eventProc: xev SelectionClear\n");
            lglw->clipboard.numChars = 0;
            free(lglw->clipboard.data);
            eventHandled = LGLW_TRUE;
            break;

         case SelectionRequest:
            Dlog("lglw:loc_eventProc: xev SelectionRequest\n");
            XSelectionRequestEvent *cbReq = (XSelectionRequestEvent*)xev;
            XSelectionEvent cbRes;

            Atom utf8 = XInternAtom(lglw->xdsp, "UTF8_STRING", False);

            cbRes.type = SelectionNotify;
            cbRes.requestor = cbReq->requestor;
            cbRes.selection = cbReq->selection;
            cbRes.target = cbReq->target;
            cbRes.time = cbReq->time;

            if(cbReq->target == utf8)
            {
               XChangeProperty(lglw->xdsp, cbReq->requestor, cbReq->property, utf8, 8/*format*/, PropModeReplace,
                               (unsigned char *)lglw->clipboard.data, lglw->clipboard.numChars);

               cbRes.property = cbReq->property;
            }
            else
            {
               cbRes.property = None;
            }

            XSendEvent(lglw->xdsp, cbReq->requestor, True, NoEventMask, (XEvent *)&cbRes);
            eventHandled = LGLW_TRUE;

            break;
      }

      if(LGLW_FALSE == eventHandled)
      {
         if(0 == lglw->parent_xwnd)
         {
            Dlog("lglw:loc_eventProc: no parent window to send events to\n");
            XSendEvent(lglw->xdsp, InputFocus, True/*propgate*/, NoEventMask, xev);
         }
         else
         {
            Dlog("lglw:loc_eventProc: sending event %i to parent\n", xev->type);
            XSendEvent(lglw->xdsp, lglw->parent_xwnd, True/*propgate*/, NoEventMask, xev);
         }
      }
   }
}

static void loc_XEventProc(void *_xevent) {
   XEvent *xev = (XEvent*)_xevent;

   Dlog_vvv("lglw:loc_XEventProc: ENTER\n");
   // Dlog_vvv("lglw: XEventProc, xev=%p\n", xev);

   if(NULL != xev)
   {
      LGLW(loc_getProperty(xev->xany.display, xev->xany.window, "_lglw"));  // get instance pointer
      Dlog_vvv("lglw:loc_XEventProc: xev=%p lglw=%p\n", xev, lglw);

      loc_eventProc(xev, lglw);
   }

   Dlog_vvv("lglw:loc_XEventProc: LEAVE\n");
}

static void loc_setProperty(Display *_display, Window _window, const char *_name, void *_value) {
   size_t data = (size_t)_value;
   long temp[2];

   // Split the 64 bit pointer into a little-endian long array
   temp[0] = (long)(data & 0xffffffffUL);
   temp[1] = (long)(data >> 32L);

   Dlog_v("lglw:loc_setProperty: name=\"%s\" value=%p temp[0]=%08x temp[1]=%08x\n", _name, _value, (uint32_t)temp[0], (uint32_t)temp[1]);

   Atom atom = XInternAtom(_display, _name, False/*only_if_exists*/);

   // (note) what's quite weird here is that we're writing an array of 32bit values, yet the element format must be 64bit (long)
   XChangeProperty(_display, _window,
                   atom/*property*/,
                   atom/*type*/,
                   32/*format*/,
                   PropModeReplace/*mode*/,
                   (unsigned char*)temp/*data*/,
                   2/*nelements*/
                   );
}

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

   Dlog_vvv("lglw:loc_getProperty: LOWER userSize=%d userCount=%lu bytes=%lu data=%p\n", userSize, userCount, bytes, data);

   if(NULL != data)
   {
      if(userCount >= 1)
      {
         // lower 32-bit
         uptr.ui[0] = *(long*)data;
         uptr.ui[1] = 0;

         Dlog_vvv("lglw:loc_getProperty:    lower=0x%08x\n", uptr.ui[0]);

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

         Dlog_vvv("lglw:loc_getProperty: UPPER userSize=%d userCount=%lu bytes=%lu data=%p\n", userSize, userCount, bytes, data);
         if(NULL != data)
         {
            // upper 32-bit
            uptr.ui[1] = *(long*)data;
            Dlog_vvv("lglw:loc_getProperty:    upper=0x%08x\n", uptr.ui[1]);
            XFree(data);
         }
      }
   }

   Dlog_vvv("lglw:loc_getProperty: return value=%p\n", uptr.any);

   return uptr.any;
}


#ifdef ARCH_X64
#if 0
// Pulled from the Renoise 64-bit callback example
// Unsure what data was supposed to be, but swapping it to a function name did not work
// This does nothing, no event proc found
static void loc_setEventProc (Display *display, Window window) {
   size_t data = (size_t)loc_eventProc;
   long temp[2];

   Dlog("lglw: setEventProc (2*32bit). window=%lu loc_eventProc=%p\n", window, &loc_eventProc);

   // Split the 64 bit pointer into a little-endian unsigned int array
   temp[0] = (uint32_t)(data & 0xffffffffUL);
   temp[1] = (uint32_t)(data >> 32L);

   Atom atom = XInternAtom(display, "_XEventProc", False);
   XChangeProperty(display, window,
                   atom/*property*/,
                   atom/*type*/,
                   32/*format*/,
                   PropModeReplace/*mode*/,
                   (unsigned char*)temp/*data*/,
                   2/*nelements*/
                   );
}
#else
// GPL code pulled from the amsynth example <https://github.com/amsynth/amsynth/blob/4a87798e650c6d71d70274a961c9b8d98fc6da7e/src/amsynth_vst.cpp>
// Simply swapped out the function names, crashes Ardour in the same was as the others
static void loc_setEventProc (Display *display, Window window) {
   //
   // JUCE calls XGetWindowProperty with long_length = 1 which means it only fetches the lower 32 bits of the address.
   // Therefore we need to ensure we return an address in the lower 32-bits of address space.
   //

   // based on mach_override
   static const unsigned char kJumpInstructions[] = {
         0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00
   };
   static const int kJumpAddress = 6;

   static char *ptr = 0;
   if (!ptr) {
      ptr = (char *)mmap(0,
                         getpagesize()/*PAGE_SIZE*/,
                         PROT_READ | PROT_WRITE | PROT_EXEC,
                         MAP_ANONYMOUS | MAP_PRIVATE | MAP_32BIT,
                         0, 0);
      if (ptr == MAP_FAILED) {
         perror("mmap");
         ptr = 0;
         return;
      } else {
         memcpy(ptr, kJumpInstructions, sizeof(kJumpInstructions));
         *((uint64_t *)(ptr + kJumpAddress)) = (uint64_t)(&loc_XEventProc);
         msync(ptr, sizeof(kJumpInstructions), MS_INVALIDATE);
         Dlog("lglw: 64bit trampoline installed\n");
      }
   }

   long temp[2] = {(uint32_t)(((size_t)ptr)&0xFFFFfffful), 0};
   Atom atom = XInternAtom(display, "_XEventProc", False);
   XChangeProperty(display, window,
                   atom/*property*/,
                   atom/*type*/,
                   32/*format*/,
                   PropModeReplace/*mode*/,
                   (unsigned char *)temp/*data*/,
                   2/*nelements*/
                   );
}
#endif
#else
// Pulled from the eXT2 example
static void loc_setEventProc (Display *display, Window window) {
   void* data = (void*)&loc_XEventProc; // swapped the function name here

   // (note) 32-bit only
   Atom atom = XInternAtom(display, "_XEventProc", False);
   XChangeProperty(display, window,
                   atom/*property*/,
                   atom/*type*/,
                   32/*format*/,
                   PropModeReplace/*mode*/,
                   (unsigned char*)&data/*data*/,
                   1/*nelements*/
                   );
}
#endif // ARCH_X64


// ---------------------------------------------------------------------------- lglw_window_open
lglw_bool_t lglw_window_open (lglw_t _lglw, void *_parentHWNDOrNull, int32_t _x, int32_t _y, int32_t _w, int32_t _h) {
   lglw_bool_t r = LGLW_FALSE;
   LGLW(_lglw);

   if(NULL != lglw)
   {
      Dlog_v("lglw:lglw_window_open: 1, %p, %i p=(%d; %d) s=(%d; %d)\n", (Window)_parentHWNDOrNull, (Window)_parentHWNDOrNull, _x, _y, _w, _h);
      lglw->parent_xwnd = (0 == _parentHWNDOrNull) ? DefaultRootWindow(lglw->xdsp) : (Window)_parentHWNDOrNull;

      Dlog_v("lglw:lglw_window_open: 2 lglw=%p\n", lglw);
      if(_w <= 16)
         _w = lglw->hidden.size.x;

      Dlog_v("lglw:lglw_window_open: 3\n");
      if(_h <= 16)
         _h = lglw->hidden.size.y;

      // TODO: compare to 'WindowClass' from Windows implementation

      Dlog_v("lglw:lglw_window_open: 4\n");
      XSetWindowAttributes swa;
      XSync(lglw->xdsp, False);

      Dlog_v("lglw:lglw_window_open: 5\n");
      swa.border_pixel = 0;
      swa.colormap = lglw->cmap;
      swa.event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | PointerMotionMask | ButtonMotionMask | ExposureMask | FocusChangeMask; // NoEventMask to bubble-up to parent
      lglw->win.xwnd = XCreateWindow(lglw->xdsp/*display*/,
                                     DefaultRootWindow(lglw->xdsp)/*parent. see Cameron's comment below.*/,
                                     0/*x*/,
                                     0/*y*/,
                                     _w/*width*/,
                                     _h/*height*/,
                                     0/*border_width*/,
                                     CopyFromParent/*depth*/,
                                     InputOutput/*class*/,
                                     lglw->vi->visual,
                                     CWBorderPixel | CWColormap | CWEventMask/*value_mask*/,
                                     &swa/*attributes*/
                                     );

      Dlog_v("lglw:lglw_window_open: 6\n");
      XSetStandardProperties(lglw->xdsp/*display*/,
                             lglw->win.xwnd/*window*/,
                             "LGLW"/*window_name*/,
                             "LGLW"/*icon_name*/,
                             None/*icon_pixmap*/,
                             NULL/*argv*/,
                             0/*argc*/,
                             NULL/*XSizeHints*/
                             );

      Dlog_v("lglw:lglw_window_open: 7\n");

#ifdef USE_XEVENTPROC
      loc_setEventProc(lglw->xdsp, lglw->win.xwnd);
#else
      {
         void *nowarn = &loc_setEventProc;
         (void)nowarn;
      }
#endif

      loc_setProperty(lglw->xdsp, lglw->win.xwnd, "_lglw", (void*)lglw);  // set instance pointer

      if(0 != _parentHWNDOrNull)
      {
#ifdef USE_XEVENTPROC
         loc_setEventProc(lglw->xdsp, lglw->parent_xwnd);
#endif // USE_XEVENTPROC
         loc_setProperty(lglw->xdsp, lglw->parent_xwnd, "_lglw", (void*)lglw);  // set instance pointer
      }

      // Some hosts only check and store the callback when the Window is reparented
      // Since creating the Window with a Parent may or may not do that, but the callback is not set,
      // ... it's created as a root window, the callback is set, and then it's reparented
      if (0 != _parentHWNDOrNull)
      {
         Dlog_v("lglw:lglw_window_open: 8\n");
         XReparentWindow(lglw->xdsp, lglw->win.xwnd, lglw->parent_xwnd, 0, 0);
      }

      lglw->win.b_owner = LGLW_TRUE;

      Dlog_v("lglw:lglw_window_open: 9\n");
      if(lglw->win.b_owner)
      {
         // // XMapRaised(lglw->xdsp, lglw->win.xwnd);
         XMapWindow(lglw->xdsp, lglw->win.xwnd);
      }

      XSync(lglw->xdsp, False);
      lglw->win.mapped = LGLW_TRUE;

      Dlog_v("lglw:lglw_window_open: 10\n");
      lglw->win.size.x = _w;
      lglw->win.size.y = _h;

      Dlog_v("lglw:lglw_window_open: 11\n");
      loc_enable_dropfiles(lglw, (NULL != lglw->dropfiles.cbk));

      Dlog_v("lglw:lglw_window_open: EXIT\n");

      r = LGLW_TRUE;
   }
   return r;
}


// ---------------------------------------------------------------------------- lglw_window_resize
lglw_bool_t lglw_window_resize (lglw_t _lglw, int32_t _w, int32_t _h) {
   lglw_bool_t r = LGLW_FALSE;
   LGLW(_lglw);

   if(NULL != lglw)
   {
      if(0 != lglw->hidden.xwnd)
      {
         XResizeWindow(lglw->xdsp, lglw->hidden.xwnd, _w, _h);

#ifdef LGLW_CONTEXT_ALLOW_USE_AFTER_FREE
         // (note) [bsp] destroying the GL context also destroys all GL objects attached to it
         //               (_if_ the GL driver is implemented correctly).
         //               IOW, if we destroy the context here, we'd have to re-create all the textures,
         //               buffer objects, framebuffer objects, shaders, .. before they can be used again.
         //               Apparently this works with certain GL drivers on Linux but it still is an application error.
         loc_destroy_gl(lglw);
         loc_create_gl(lglw);
#endif // LGLW_CONTEXT_ALLOW_USE_AFTER_FREE
      }

      if(0 != lglw->win.xwnd)
      {
         r = LGLW_TRUE;

         int deltaW = _w - lglw->win.size.x;
         int deltaH = _h - lglw->win.size.y;

         Dlog_v("Child Window Resize; old (%5i x %5i) new(%5i x %5i)\n", lglw->win.size.x, lglw->win.size.y, _w, _h);

         lglw->win.size.x = _w;
         lglw->win.size.y = _h;

         Window root, parent, *children = NULL;
         unsigned int num_children;

         if(!XQueryTree(lglw->xdsp, lglw->win.xwnd, &root, &parent, &children, &num_children))
            return r;

         if(children)
            XFree((char *)children);

         // Resize parent window (if any)
         if(0 != parent)
         {
            int x, y;
            unsigned int width, height;
            unsigned int border_width;
            unsigned int depth;

            if(!XGetGeometry(lglw->xdsp, lglw->win.xwnd, &root, &x, &y, &width, &height, &border_width, &depth))
               return r;

            Dlog_v("Parent Window Resize; old (%5i x %5i) new(%5i x %5i), delta (%5i x %5i)\n", width, height, width + deltaW, height + deltaH, deltaW, deltaH);

            XResizeWindow(lglw->xdsp, parent, width + deltaW, height + deltaH);
         }

         XResizeWindow(lglw->xdsp, lglw->win.xwnd, _w, _h);
         XRaiseWindow(lglw->xdsp, lglw->win.xwnd);
      }
   }

   return r;
}


// ---------------------------------------------------------------------------- lglw_window_close
void lglw_window_close (lglw_t _lglw) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      if(0 != lglw->win.xwnd)
      {
         Dlog_v("lglw:lglw_window_close: 1\n");
         lglw_timer_stop(_lglw);

         Dlog_v("lglw:lglw_window_close: 2\n");
         glXMakeCurrent(lglw->xdsp, None, NULL);

         Dlog_v("lglw:lglw_window_close: 3\n");
         if(lglw->win.b_owner)
         {
            XUnmapWindow(lglw->xdsp, lglw->win.xwnd);
            XDestroyWindow(lglw->xdsp, lglw->win.xwnd);
            lglw->win.b_owner = LGLW_FALSE;
         }
         XSync(lglw->xdsp, False);
         lglw->win.xwnd = 0;
         lglw->win.mapped = LGLW_FALSE;

         {
            XEvent xev;
            int queued = XPending(lglw->xdsp);
            Dlog_vvv("lglw:lglw_window_close: consume %d pending events\n", queued);
            while(queued)
            {
               XNextEvent(lglw->xdsp, &xev);
               queued--;
            }
         }
      }
   }
   Dlog_v("lglw:lglw_window_close: EXIT\n");
}


// ---------------------------------------------------------------------------- lglw_window_show
void lglw_window_show(lglw_t _lglw) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      Dlog_v("lglw:lglw_window_show: 1\n");
      XMapRaised(lglw->xdsp, lglw->win.xwnd);

      lglw->win.mapped = LGLW_TRUE;
   }
   Dlog_v("lglw:lglw_window_show: EXIT\n");
}


// ---------------------------------------------------------------------------- lglw_window_hide
void lglw_window_hide(lglw_t _lglw) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      Dlog_v("lglw:lglw_window_hide: 1\n");
      XUnmapWindow(lglw->xdsp, lglw->win.xwnd);

      lglw->win.mapped = LGLW_FALSE;
   }
   Dlog_v("lglw:lglw_window_hide: EXIT\n");
}


// ---------------------------------------------------------------------------- lglw_window_is_visible
lglw_bool_t lglw_window_is_visible(lglw_t _lglw) {
   lglw_bool_t r = LGLW_FALSE;
   LGLW(_lglw);

   // Dlog_vvv("lglw:lglw_window_is_visible: 1\n");
   if(NULL != lglw && 0 != lglw->win.xwnd)
   {
      // Dlog_vvv("lglw:lglw_window_is_visible: 2\n");
      r = lglw->win.mapped;
   }

   // Dlog_vvv("lglw:lglw_window_is_visible: EXIT\n");
   return r;
}


// ---------------------------------------------------------------------------- lglw_window_size_get
void lglw_window_size_get(lglw_t _lglw, int32_t *_retX, int32_t *_retY) {
   LGLW(_lglw);

   Dlog_vvv("lglw:lglw_window_size_get: 1\n");
   if(NULL != lglw)
   {
      if(0 != lglw->win.xwnd)
      {
         if(NULL != _retX)
            *_retX = lglw->win.size.x;

         if(NULL != _retY)
            *_retY = lglw->win.size.y;
      }
   }
   Dlog_vvv("lglw:lglw_window_size_get: EXIT\n");
}


// ---------------------------------------------------------------------------- lglw_redraw
void lglw_redraw(lglw_t _lglw) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      if(0 != lglw->win.xwnd)
      {
         Dlog_vvv("lglw:lglw_redraw: 1\n");
         XEvent xev;
         xev.xany.type       = Expose;
         xev.xany.serial     = 0;
         xev.xany.send_event = True;
         xev.xany.display    = lglw->xdsp;
         xev.xany.window     = lglw->win.xwnd;
         xev.xexpose.x      = 0;
         xev.xexpose.y      = 0;
         xev.xexpose.width  = lglw->win.size.x;
         xev.xexpose.height = lglw->win.size.y;
         xev.xexpose.count  = 0;
         XSendEvent(lglw->xdsp, lglw->win.xwnd,
                    True/*propagate*/,
                    ExposureMask/*event_mask*/,
                    &xev
                    );
         XFlush(lglw->xdsp);
      }
   }
}


// ---------------------------------------------------------------------------- lglw_redraw_callback_set
void lglw_redraw_callback_set(lglw_t _lglw, lglw_redraw_fxn_t _cbk) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      lglw->redraw.cbk = _cbk;
   }
}


// ---------------------------------------------------------------------------- lglw_glcontext_push
void lglw_glcontext_push(lglw_t _lglw) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      lglw->prev.drw = glXGetCurrentDrawable();
      lglw->prev.ctx = glXGetCurrentContext();

      Dlog_vvv("lglw:lglw_glcontext_push: win.xwnd=%p hidden.xwnd=%p ctx=%p\n",
               lglw->win.xwnd, lglw->hidden.xwnd, lglw->ctx);
      if(!glXMakeCurrent(lglw->xdsp, (0 == lglw->win.xwnd) ? lglw->hidden.xwnd : lglw->win.xwnd, lglw->ctx))
      {
         Dlog("[---] lglw_glcontext_push: glXMakeCurrent() failed. win.xwnd=%p hidden.xwnd=%p ctx=%p glGetError()=%d\n", lglw->win.xwnd, lglw->hidden.xwnd, lglw->ctx, glGetError());
      }
      // Dlog_vvv("lglw:lglw_glcontext_push: LEAVE\n");
   }
}


// ---------------------------------------------------------------------------- lglw_glcontext_rebind
void lglw_glcontext_rebind(lglw_t _lglw) {
   LGLW(_lglw);
   // printf("xxx lglw_glcontext_rebind\n");

   if(NULL != lglw)
   {
      Dlog_vvv("lglw:lglw_glcontext_rebind: win.xwnd=%p hidden.xwnd=%p ctx=%p\n",
               lglw->win.xwnd, lglw->hidden.xwnd, lglw->ctx);
      (void)glXMakeCurrent(lglw->xdsp, None, NULL);
      if(!glXMakeCurrent(lglw->xdsp, (0 == lglw->win.xwnd) ? lglw->hidden.xwnd : lglw->win.xwnd, lglw->ctx))
      {
         Dlog("[---] lglw_glcontext_rebind: glXMakeCurrent() failed. win.xwnd=%p hidden.xwnd=%p ctx=%p glGetError()=%d\n", lglw->win.xwnd, lglw->hidden.xwnd, lglw->ctx, glGetError());
      }
   }
}


// ---------------------------------------------------------------------------- lglw_glcontext_pop
void lglw_glcontext_pop(lglw_t _lglw) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      Dlog_vvv("lglw:lglw_glcontext_pop: prev.drw=%p prev.ctx=%p\n",
               lglw->prev.drw, lglw->prev.ctx);
      if(!glXMakeCurrent(lglw->xdsp, lglw->prev.drw, lglw->prev.ctx))
      {
         Dlog("[---] lglw_glcontext_pop: glXMakeCurrent() failed. prev.drw=%p ctx=%p glGetError()=%d\n", lglw->prev.drw, lglw->prev.ctx, glGetError());
      }
   }
}


// ---------------------------------------------------------------------------- lglw_swap_buffers
void lglw_swap_buffers(lglw_t _lglw) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      if(0 != lglw->win.xwnd)
      {
         Dlog_vvv("lglw:lglw_swap_buffers: 1\n");
         glXSwapBuffers(lglw->xdsp, lglw->win.xwnd);
      }
   }
}


// ---------------------------------------------------------------------------- lglw_swap_interval_set
typedef void (APIENTRY *PFNWGLEXTSWAPINTERVALPROC) (Display *, GLXDrawable, int);
void lglw_swap_interval_set(lglw_t _lglw, int32_t _ival) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      if(0 != lglw->win.xwnd)
      {
         Dlog_vv("lglw:lglw_swap_interval_set: 1\n");
         PFNWGLEXTSWAPINTERVALPROC glXSwapIntervalEXT;
         glXSwapIntervalEXT = (PFNWGLEXTSWAPINTERVALPROC) glXGetProcAddress((const GLubyte*)"glXSwapIntervalEXT");
         if(NULL != glXSwapIntervalEXT)
         {
            Dlog_vv("lglw:lglw_swap_interval_set: 2\n");
            glXSwapIntervalEXT(lglw->xdsp, lglw->win.xwnd, _ival);
            lglw->win.swap_interval = _ival;
         }
      }
   }
}


// ---------------------------------------------------------------------------- lglw_swap_interval_get
int32_t lglw_swap_interval_get(lglw_t _lglw) {
   LGLW(_lglw);
   int32_t r = 0;

   if(NULL != lglw)
   {
      r = lglw->win.swap_interval;
   }

   return r;
}


// ---------------------------------------------------------------------------- loc_handle_mouseleave
static void loc_handle_mouseleave(lglw_int_t *lglw) {
   lglw->focus.state &= ~LGLW_FOCUS_MOUSE;

   if(NULL != lglw->focus.cbk)
   {
      lglw->focus.cbk(lglw, lglw->focus.state, LGLW_FOCUS_MOUSE);
   }

   Dlog_vv("xxx lglw:loc_handle_mouseleave: LEAVE\n");
}


// ---------------------------------------------------------------------------- loc_handle_mouseenter
static void loc_handle_mouseenter(lglw_int_t *lglw) {

   lglw->focus.state |= LGLW_FOCUS_MOUSE;

   if(NULL != lglw->focus.cbk)
   {
      lglw->focus.cbk(lglw, lglw->focus.state, LGLW_FOCUS_MOUSE);
   }

   Dlog_vv("xxx lglw:loc_handle_mouseenter: LEAVE\n");
}


// ---------------------------------------------------------------------------- loc_handle_mousebutton
static void loc_handle_mousebutton(lglw_int_t *lglw, lglw_bool_t _bPressed, uint32_t _button) {
   if(_bPressed)
      lglw->mouse.button_state |= _button;
   else
      lglw->mouse.button_state &= ~_button;

   if(NULL != lglw->mouse.cbk)
   {
      lglw->mouse.cbk(lglw, lglw->mouse.p.x, lglw->mouse.p.y, lglw->mouse.button_state, _button);
   }
}


// ---------------------------------------------------------------------------- loc_handle_mousemotion
static void loc_handle_mousemotion(lglw_int_t *lglw) {

   if(NULL != lglw->mouse.cbk)
   {
      lglw->mouse.cbk(lglw, lglw->mouse.p.x, lglw->mouse.p.y, lglw->mouse.button_state, 0u/*changedbuttonstate*/);
   }
}


// ---------------------------------------------------------------------------- lglw_mouse_callback_set
void lglw_mouse_callback_set(lglw_t _lglw, lglw_mouse_fxn_t _cbk) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      lglw->mouse.cbk = _cbk;
   }
}


// ---------------------------------------------------------------------------- lglw_mouse_callback_set
void lglw_focus_callback_set(lglw_t _lglw, lglw_focus_fxn_t _cbk) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      lglw->focus.cbk = _cbk;
   }
}


// ---------------------------------------------------------------------------- loc_handle_key
static lglw_bool_t loc_handle_key(lglw_int_t *lglw, lglw_bool_t _bPressed, uint32_t _vkey) {
   lglw_bool_t r = LGLW_FALSE;

   if(NULL != lglw->keyboard.cbk)
   {
      r = lglw->keyboard.cbk(lglw, _vkey, lglw->keyboard.kmod_state, _bPressed);
   }

   return r;
}


// ---------------------------------------------------------------------------- lglw_keyboard_callback_set
void lglw_keyboard_callback_set(lglw_t _lglw, lglw_keyboard_fxn_t _cbk) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      lglw->keyboard.cbk = _cbk;
   }
}


// ---------------------------------------------------------------------------- lglw_keyboard_get_modifiers
uint32_t lglw_keyboard_get_modifiers(lglw_t _lglw) {
   uint32_t r = 0u;
   LGLW(_lglw);

   if(NULL != lglw)
   {
      r = lglw->keyboard.kmod_state;
   }

   return r;
}


// ---------------------------------------------------------------------------- lglw_keyboard_clear_modifiers
void lglw_keyboard_clear_modifiers(lglw_t _lglw) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      lglw->keyboard.kmod_state = 0;
   }
}


// ---------------------------------------------------------------------------- lglw_touchkeyboard_show
void lglw_touchkeyboard_show(lglw_t _lglw, lglw_bool_t _bEnable) {
   LGLW(_lglw);

   // (todo) implement me

   if(NULL != lglw)
   {
   }
}


// ---------------------------------------------------------------------------- lglw_mouse_get_buttons
uint32_t lglw_mouse_get_buttons(lglw_t _lglw) {
   uint32_t r = 0u;
   LGLW(_lglw);

   if(NULL != lglw)
   {
      r = lglw->mouse.button_state;
   }

   return r;
}


// ---------------------------------------------------------------------------- lglw_mouse_grab
void lglw_mouse_grab(lglw_t _lglw, uint32_t _grabMode) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      if(0 != lglw->win.xwnd)
      {
         if(!lglw->mouse.touch.b_enable)
         {
            if(lglw->mouse.grab.mode != _grabMode)
            {
               lglw_mouse_ungrab(_lglw);
            }

            int result;

            switch(_grabMode)
            {
               default:
               case LGLW_MOUSE_GRAB_NONE:
                  break;

               case LGLW_MOUSE_GRAB_CAPTURE:
                  result = XGrabPointer(lglw->xdsp, lglw->win.xwnd,
                                        True/*owner_events*/,
                                        NoEventMask/*event_mask*/,
                                        GrabModeAsync/*pointer_mode*/,
                                        GrabModeAsync/*keyboard_mode*/,
                                        lglw->win.xwnd/*confine_to*/,
                                        None/*cursor*/,
                                        CurrentTime/*time*/);
                  if(GrabSuccess != result)
                  {
                     Dlog("lglw: Grab Result: %i\n", result);
                  }
                  else
                  {
                     lglw->mouse.grab.mode = _grabMode;
                  }
                  break;

               case LGLW_MOUSE_GRAB_WARP:
                  result = XGrabPointer(lglw->xdsp, lglw->win.xwnd,
                                        True/*owner_events*/,
                                        NoEventMask/*event_mask*/,
                                        GrabModeAsync/*pointer_mode*/,
                                        GrabModeAsync/*keyboard_mode*/,
                                        lglw->win.xwnd/*confine_to*/,
                                        None/*cursor*/,
                                        CurrentTime/*time*/);
                  if(GrabSuccess != result)
                  {
                     Dlog("lglw: Grab Result: %i\n", result);
                  }
                  else
                  {
                     lglw_mouse_cursor_show(_lglw, LGLW_FALSE);
                     lglw->mouse.grab.p = lglw->mouse.p;
                     lglw->mouse.grab.last_p = lglw->mouse.p;
                     lglw->mouse.grab.mode = _grabMode;
                  }
                  break;
            }
         }
      }
   }
}


// ---------------------------------------------------------------------------- lglw_mouse_ungrab
void lglw_mouse_ungrab(lglw_t _lglw) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      if(0 != lglw->win.xwnd)
      {
         if(!lglw->mouse.touch.b_enable)
         {
            switch(lglw->mouse.grab.mode)
            {
               default:
               case LGLW_MOUSE_GRAB_NONE:
                  break;

               case LGLW_MOUSE_GRAB_CAPTURE:
                  XUngrabPointer(lglw->xdsp, CurrentTime);
                  lglw->mouse.grab.mode = LGLW_MOUSE_GRAB_NONE;
                  break;

               case LGLW_MOUSE_GRAB_WARP:
                  XUngrabPointer(lglw->xdsp, CurrentTime);
                  lglw->mouse.grab.mode = LGLW_MOUSE_GRAB_NONE;
                  lglw->mouse.grab.b_queue_warp = LGLW_TRUE;
                  lglw_mouse_cursor_show(_lglw, LGLW_TRUE);
                  break;
            }
         }
      }
   }
}


// ---------------------------------------------------------------------------- lglw_mouse_warp
void lglw_mouse_warp(lglw_t _lglw, int32_t _x, int32_t _y) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      if(0 != lglw->win.xwnd)
      {
         XWarpPointer(lglw->xdsp,
                      None/*src_w*/,
                      lglw->win.xwnd/*dest_w*/,
                      0/*src_x*/,
                      0/*src_y*/,
                      0/*src_width*/,
                      0/*src_height*/,
                      _x/*dest_x*/,
                      _y/*dest_y*/);
      }
   }
}


// ---------------------------------------------------------------------------- loc_handle_queued_mouse_warp
static void loc_handle_queued_mouse_warp(lglw_int_t *lglw) {
   if(lglw->mouse.grab.b_queue_warp)
   {
      lglw->mouse.grab.b_queue_warp = LGLW_FALSE;
      lglw_mouse_warp(lglw, lglw->mouse.grab.p.x, lglw->mouse.grab.p.y);
      lglw->mouse.grab.last_p = lglw->mouse.grab.p;
   }
}


// ---------------------------------------------------------------------------- lglw_mouse_cursor_show
void lglw_mouse_cursor_show (lglw_t _lglw, lglw_bool_t _bShow) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      if(LGLW_FALSE == _bShow)
      {
         Pixmap noPxm;
         Cursor noCursor;
         XColor black, dummy;
         static char pxmNoData[] = {0, 0, 0, 0, 0, 0, 0, 0};

         XAllocNamedColor(lglw->xdsp, lglw->cmap, "black", &black, &dummy);
         noPxm = XCreateBitmapFromData(lglw->xdsp, lglw->win.xwnd, pxmNoData, 8, 8);
         noCursor = XCreatePixmapCursor(lglw->xdsp, noPxm, noPxm, &black, &black, 0, 0);

         XDefineCursor(lglw->xdsp, lglw->win.xwnd, noCursor);
         XFreeCursor(lglw->xdsp, noCursor);
         if(noPxm != None)
         {
            XFreePixmap(lglw->xdsp, noPxm);
         }
      }
      else
      {
         XUndefineCursor(lglw->xdsp, lglw->win.xwnd);
      }
   }
}


// ---------------------------------------------------------------------------- lglw_timer_start
void lglw_timer_start(lglw_t _lglw, uint32_t _millisec) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      Dlog_v("lglw:lglw_timer_start: interval=%u\n", _millisec);
      lglw->timer.interval_ms = _millisec;
      lglw->timer.b_running   = LGLW_TRUE;
   }
}


// ---------------------------------------------------------------------------- lglw_timer_stop
void lglw_timer_stop(lglw_t _lglw) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      Dlog_v("lglw:lglw_timer_stop\n");
      lglw->timer.b_running = LGLW_FALSE;
   }
}


// ---------------------------------------------------------------------------- lglw_timer_callback_set
void lglw_timer_callback_set(lglw_t _lglw, lglw_timer_fxn_t _cbk) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      lglw->timer.cbk = _cbk;
   }
}


// ---------------------------------------------------------------------------- loc_process_timer()
static void loc_process_timer(lglw_int_t *lglw) {
   if(lglw->timer.b_running)
   {
      uint32_t ms = loc_millisec_delta(lglw);

      if( (ms - lglw->timer.last_ms) >= lglw->timer.interval_ms )
      {
         lglw->timer.last_ms = ms;

         if(NULL != lglw->timer.cbk)
         {
            Dlog_vvv("lglw: invoke timer callback\n");
            lglw->timer.cbk(lglw);
         }
      }
   }
}


// ---------------------------------------------------------------------------- lglw_time_get_millisec
uint32_t lglw_time_get_millisec(lglw_t _lglw) {
   uint32_t r = 0u;
   LGLW(_lglw);

   if(NULL != lglw)
   {
      r = loc_millisec_delta(lglw);
   }
   
   return r;
}


// ---------------------------------------------------------------------------- loc_enable_dropfiles
static void loc_enable_dropfiles(lglw_int_t *lglw, lglw_bool_t _bEnable) {

   // (todo) implement me
}


// ---------------------------------------------------------------------------- lglw_dropfiles_callback_set
void lglw_dropfiles_callback_set(lglw_t _lglw, lglw_dropfiles_fxn_t _cbk) {
   LGLW(_lglw);

   if(NULL != _lglw)
   {
      lglw->dropfiles.cbk = _cbk;

      loc_enable_dropfiles(lglw, (NULL != _cbk));
   }
}


// ---------------------------------------------------------------------------- lglw_touchinput_set
void lglw_touchinput_set(lglw_t _lglw, lglw_bool_t _bEnable) {
   LGLW(_lglw);

   if(NULL != _lglw)
   {
      lglw->mouse.touch.b_enable = _bEnable;
      lglw->mouse.touch.b_update_queued = LGLW_TRUE;
   }
}


// ---------------------------------------------------------------------------- lglw_touchinput_get
lglw_bool_t lglw_touchinput_get(lglw_t _lglw) {
   lglw_bool_t r = LGLW_FALSE;
   LGLW(_lglw);

   if(NULL != _lglw)
   {
      r = lglw->mouse.touch.b_enable;
   }

   return r;
}


// ---------------------------------------------------------------------------- lglw_clipboard_text_set
void lglw_clipboard_text_set(lglw_t _lglw, const uint32_t _numChars, const char *_text) {
   LGLW(_lglw);
   (void)_numChars;

   if(NULL != _text)
   {
      if(NULL != _lglw)
      {
         if(0 != lglw->win.xwnd)
         {
            uint32_t numChars = (0u == _numChars) ? ((uint32_t)strlen(_text)+1u) : _numChars;

            if(numChars > 0u)
            {
               lglw->clipboard.numChars = numChars;
               lglw->clipboard.data = malloc(numChars+1);

               uint32_t i;
               for(i = 0u; i < numChars; i++)
               {
                  lglw->clipboard.data[i] = _text[i];
               }
               lglw->clipboard.data[numChars - 1] = 0;

               Dlog("xxx lglw_clipboard_text_set(%i): %s\n", lglw->clipboard.numChars, lglw->clipboard.data);

               Atom clipboard = XInternAtom(lglw->xdsp, "CLIPBOARD", False);
               XSetSelectionOwner(lglw->xdsp, clipboard, lglw->win.xwnd, CurrentTime);
               XSync(lglw->xdsp, False);
            }
         }
      }
   }
}


// ---------------------------------------------------------------------------- loc_is_clipboard_event
static Bool loc_is_clipboard_event(Display *_display, XEvent *_xevent, XPointer _xarg) {
   return _xevent->type == SelectionNotify;
}


// ---------------------------------------------------------------------------- lglw_clipboard_text_get
void lglw_clipboard_text_get(lglw_t _lglw, uint32_t _maxChars, uint32_t *_retNumChars, char *_retText) {
   LGLW(_lglw);

   if(NULL != _retNumChars)
      *_retNumChars = 0u;

   if(NULL != _retText)
      *_retText = 0;

   if(_maxChars > 0u)
   {
      if(NULL != _lglw)
      {
         if(0 != lglw->win.xwnd)
         {
            Window owner;
            XEvent xev;
            XSelectionEvent *cbReq;
            Atom clipboard = XInternAtom(lglw->xdsp, "CLIPBOARD", False);
            Atom utf8 = XInternAtom(lglw->xdsp, "UTF8_STRING", False);
            Atom target = XInternAtom(lglw->xdsp, "_clipboard_result", False);

            owner = XGetSelectionOwner(lglw->xdsp, clipboard);
            if(owner == None)
            {
               Dlog("xxx lglw_clipboard_text_get: No Window can provide a clipboard result\n");
               return;
            }

            if(owner == lglw->win.xwnd)
            {
               Dlog("xxx lglw_clipboard_text_get: We are the owner of the clipboard, skip X interactions\n");

               uint32_t i = 0u;
               for(; i < _maxChars; i++)
               {
                  _retText[i] = lglw->clipboard.data[i];
                  if(0 == _retText[i])
                     break;
               }
               _retText[_maxChars - 1u] = 0;

               if(NULL != _retNumChars)
                  *_retNumChars = i;

               Dlog("xxx lglw_clipboard_text_get: (result on next line)\n%s\n", _retText);
               return;
            }

            XConvertSelection(lglw->xdsp, clipboard, utf8, target, lglw->win.xwnd, CurrentTime);
            XIfEvent(lglw->xdsp, &xev, &loc_is_clipboard_event, None);

            cbReq = (XSelectionEvent*)&xev;
            if(None == cbReq->property)
            {
               Dlog("xxx lglw_clipboard_text_get: Clipboard was not converted to UTF-8 string\n");
               return;
            }

            Atom returnType;
            int returnFormat;
            unsigned long size, returnSize, bytesLeft;
            unsigned char *propertyValue = NULL;

            XGetWindowProperty(lglw->xdsp, lglw->win.xwnd, target,
                               0/*offset*/,
                               0/*length*/,
                               False/*delete*/,
                               AnyPropertyType/*req_type*/,
                               &returnType/*actual_type_return*/,
                               &returnFormat/*actual_format_return*/,
                               &returnSize/*nitems_return*/,
                               &size/*bytes_after_return*/,
                               &propertyValue/*prop_return*/);
            XFree(propertyValue);

            if(utf8 != returnType)
            {
               Dlog("xxx lglw_clipboard_text_get: Clipboard result is not a UTF-8 string\n");
               return;
            }

            if(8u != returnFormat)
            {
               Dlog("xxx lglw_clipboard_text_get: Clipboard format is not a char array\n");
               return;
            }

            if(_maxChars < size)
               size = _maxChars;
            size = 1 + ((size - 1) / 4);

            // TODO: Even with the largest current use-case, multiple calls aren't necessary. do it anyway just in case
            XGetWindowProperty(lglw->xdsp, lglw->win.xwnd, target,
                               0/*offset*/,
                               size/*length*/,
                               True/*delete*/,
                               AnyPropertyType/*req_type*/,
                               &returnType/*actual_type_return*/,
                               &returnFormat/*actual_format_return*/,
                               &returnSize/*nitems_return*/,
                               &bytesLeft/*bytes_after_return*/,
                               &propertyValue/*prop_return*/);

            if(returnSize == 0)
            {
               Dlog("xxx lglw_clipboard_text_get: No Clipboard result after final request\n");
               return;
            }

            uint32_t i = 0u;
            for(; i < _maxChars; i++)
            {
               _retText[i] = propertyValue[i];
               if(0 == _retText[i])
                  break;
            }
            _retText[_maxChars - 1u] = 0;

            if(NULL != _retNumChars)
               *_retNumChars = i;

            Dlog("xxx lglw_clipboard_text_get: (result on next line)\n%s\n", _retText);
            XFree(propertyValue);
         }
      }
   }
}


// ---------------------------------------------------------------------------- lglw_events
void lglw_events(lglw_t _lglw) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      if(0 != lglw->win.xwnd)
      {
         XEvent xev;
         int queued = XPending(lglw->xdsp);
         if(queued > 0)
         {
            Dlog_vvv("lglw:lglw_events: (events: %i)\n", queued);
         }
         while(queued)
         {
            XNextEvent(lglw->xdsp, &xev);
            loc_eventProc(&xev, lglw);
            queued--;
         }

         loc_process_timer(lglw);
      }
   }
}

/* ----
 * ---- file   : lglw_windows.c
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
 * ---- changed: 05Aug2018, 06Aug2018, 07Aug2018, 08Aug2018, 09Aug2018, 18Aug2018
 * ----
 * ----
 */

#include "lglw.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <windows.h>
#include <windowsx.h>

// #define Dprintf if(0);else printf
#define Dprintf if(1);else printf

// ---------------------------------------------------------------------------- macros and defines
#define LGLW(a) lglw_int_t *lglw = ((lglw_int_t*)(a))

#define LGLW_HIDDEN_BASE_CLASS_NAME "hidden_LGLW"
#define LGLW_WIN_BASE_CLASS_NAME "LGLW_"

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
   void *user_data;  // arbitrary user data
   HWND  parent_hwnd;  // created by host

   struct {
      char         class_name[128];
      HWND         hwnd;
      HDC          hdc;
      lglw_vec2i_t size;
   } hidden;

   struct {
      char         class_name[128];
      HWND         hwnd;
      HDC          hdc;
      HHOOK        keyboard_ll_hhook;
      lglw_vec2i_t size;
      int32_t      swap_interval;
   } win;

   struct {
      HDC   hdc;
      HGLRC hglrc;
   } prev;

   HGLRC hglrc;

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
   } timer;

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

static LRESULT CALLBACK loc_WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static void loc_key_hook(lglw_int_t *lglw);
static void loc_key_unhook(lglw_int_t *lglw);
static LRESULT CALLBACK loc_LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
static lglw_bool_t loc_handle_key (lglw_int_t *lglw, lglw_bool_t _bPressed, uint32_t _vkey);
// static lglw_bool_t loc_touchkeyboard_get_rect (RECT *rect);
// static lglw_bool_t loc_touchkeyboard_is_visible (void);
extern lglw_bool_t lglw_int_touchkeyboard_toggle (void);

static void loc_handle_mouseleave (lglw_int_t *lglw);
static void loc_handle_mouseenter (lglw_int_t *lglw);
static void loc_handle_mousebutton (lglw_int_t *lglw, lglw_bool_t _bPressed, uint32_t _button);
static void loc_handle_mousemotion (lglw_int_t *lglw);
static void loc_handle_queued_mouse_warp (lglw_int_t *lglw);
static void loc_touchinput_update (lglw_int_t *lglw);

static void loc_enable_dropfiles (lglw_int_t *lglw, lglw_bool_t _bEnable);


// ---------------------------------------------------------------------------- module vars
static lglw_int_t *khook_lglw = NULL;  // currently key-hooked lglw instance (one at a time)


// ---------------------------------------------------------------------------- lglw_init
lglw_t lglw_init(int32_t _w, int32_t _h) {
   lglw_int_t *lglw = malloc(sizeof(lglw_int_t));

   if(NULL != lglw)
   {
      memset(lglw, 0, sizeof(lglw_int_t));

      if(_w <= 16)
         _w = LGLW_DEFAULT_HIDDEN_W;

      if(_h <= 16)
         _h = LGLW_DEFAULT_HIDDEN_H;

      if(!loc_create_hidden_window(lglw, _w, _h))
      {
         free(lglw);
         lglw = NULL;
      }
   }

   return lglw;
}


// ---------------------------------------------------------------------------- lglw_exit
void lglw_exit(lglw_t _lglw) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      loc_destroy_hidden_window(lglw);

      free(lglw);
   }
}


// ---------------------------------------------------------------------------- lglw_userdata_set
void lglw_userdata_set(lglw_t _lglw, void *_userData) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      lglw->user_data = _userData;
   }
}

// ---------------------------------------------------------------------------- lglw_userdata_get
void *lglw_userdata_get(lglw_t _lglw) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      return lglw->user_data;
   }

   return NULL;
}


// ---------------------------------------------------------------------------- loc_create_hidden_window
static lglw_bool_t loc_create_hidden_window(lglw_int_t *lglw, int32_t _w, int32_t _h) {

   sprintf(lglw->hidden.class_name, LGLW_HIDDEN_BASE_CLASS_NAME "%p", lglw);

   WNDCLASS wc;
   ZeroMemory(&wc, sizeof(wc));
   wc.style         = CS_OWNDC;
   wc.lpfnWndProc   = (WNDPROC) &loc_WndProc;
   wc.hInstance     = GetModuleHandle(NULL);
   wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
   wc.lpszClassName = lglw->hidden.class_name;

   if(!RegisterClass(&wc))
   {
      // something went terribly wrong
      printf("[---] lglw: failed to register hidden window class\n");
      return LGLW_FALSE;
   }

   DWORD dwExStyle = 0;
   DWORD dwStyle = 0;

   lglw->hidden.hwnd = CreateWindowEx(dwExStyle,
                                      lglw->hidden.class_name,
                                      "LGLW_hidden",
                                      dwStyle,
                                      0/*xpos*/, 0/*ypos*/,
                                      _w, _h,
                                      NULL/*parentHWND*/,
                                      NULL/*window menu*/,
                                      GetModuleHandle(NULL),
                                      NULL
                                      );

   lglw->hidden.size.x = _w;
   lglw->hidden.size.y = _h;

   PIXELFORMATDESCRIPTOR pfd = {
      sizeof(PIXELFORMATDESCRIPTOR),
      1,
      PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
      PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
      32,                   // Colordepth of the framebuffer.
      0, 0, 0, 0, 0, 0,
      0,
      0,
      0,
      0, 0, 0, 0,
      24,                   // Number of bits for the depthbuffer
      8,                    // Number of bits for the stencilbuffer
      0,                    // Number of Aux buffers in the framebuffer.
      PFD_MAIN_PLANE,
      0,
      0, 0, 0
   };

   lglw->hidden.hdc = GetDC(lglw->hidden.hwnd);
   Dprintf("xxx lglw: hidden hdc=%p\n", lglw->hidden.hdc);

   int pfmt = ChoosePixelFormat(lglw->hidden.hdc, &pfd); 
   Dprintf("xxx lglw: hidden pfmt=%d\n", pfmt);
   SetPixelFormat(lglw->hidden.hdc, pfmt, &pfd);

   lglw->hglrc = wglCreateContext(lglw->hidden.hdc);
   Dprintf("xxx lglw: hidden hglrc=%p\n", lglw->hglrc);

   return LGLW_TRUE;
}


// ---------------------------------------------------------------------------- loc_destroy_hidden_window
static void loc_destroy_hidden_window(lglw_int_t *lglw) {
   if(NULL != lglw->hidden.hwnd)
   {
      wglDeleteContext(lglw->hglrc);
      lglw->hglrc = NULL;

      DestroyWindow(lglw->hidden.hwnd);
      lglw->hidden.hwnd = NULL;

      UnregisterClass(lglw->hidden.class_name, GetModuleHandle(NULL));
   }
}


// ---------------------------------------------------------------------------- lglw_window_open
lglw_bool_t lglw_window_open (lglw_t _lglw, void *_parentHWNDOrNull, int32_t _x, int32_t _y, int32_t _w, int32_t _h) {
   lglw_bool_t r = LGLW_FALSE;
   LGLW(_lglw);

   if(NULL != lglw)
   {
      lglw->parent_hwnd = (HWND)_parentHWNDOrNull;

      sprintf(lglw->win.class_name, LGLW_WIN_BASE_CLASS_NAME "%p", lglw);

      WNDCLASS wc;
      ZeroMemory(&wc, sizeof(wc));
      wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
      wc.lpfnWndProc   = (WNDPROC) &loc_WndProc;
      wc.hInstance     = GetModuleHandle(NULL);
      wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
      wc.lpszClassName = lglw->win.class_name;

      if(!RegisterClass(&wc))
      {
         // something went terribly wrong
         printf("[---] lglw: failed to register window class\n");
         return LGLW_FALSE;
      }

      if(_w <= 16)
         _w = lglw->hidden.size.x;

      if(_h <= 16)
         _h = lglw->hidden.size.y;

      DWORD dwExStyle = 0;
      DWORD dwStyle = ((NULL != _parentHWNDOrNull) ? WS_CHILD : 0) | WS_VISIBLE;

      lglw->win.hwnd = CreateWindowEx(dwExStyle,
                                      lglw->win.class_name,
                                      "LGLW",
                                      dwStyle,
                                      (NULL == _parentHWNDOrNull) ? _x : 0,
                                      (NULL == _parentHWNDOrNull) ? _y : 0,
                                      _w, _h,
                                      _parentHWNDOrNull,
                                      NULL/*window menu*/,
                                      GetModuleHandle(NULL),
                                      NULL
                                      );

      lglw->win.size.x = _w;
      lglw->win.size.y = _h;

      // window_to_wrapper = this;

      PIXELFORMATDESCRIPTOR pfd = {
         sizeof(PIXELFORMATDESCRIPTOR),
         1,
         PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
         PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
         32,                   // Colordepth of the framebuffer.
         0, 0, 0, 0, 0, 0,
         0,
         0,
         0,
         0, 0, 0, 0,
         24,                   // Number of bits for the depthbuffer
         8,                    // Number of bits for the stencilbuffer
         0,                    // Number of Aux buffers in the framebuffer.
         PFD_MAIN_PLANE,
         0,
         0, 0, 0
      };

      lglw->win.hdc = GetDC(lglw->win.hwnd);

      int pfmt = ChoosePixelFormat(lglw->win.hdc, &pfd); 
      Dprintf("xxx lglw: win pfmt=%d\n", pfmt);
      SetPixelFormat(lglw->win.hdc, pfmt, &pfd);

      (void)SetWindowLongPtr(lglw->win.hwnd, GWLP_USERDATA, (LONG_PTR)lglw);

      loc_enable_dropfiles(lglw, (NULL != lglw->dropfiles.cbk));
   }
   return r;
}


// ---------------------------------------------------------------------------- lglw_window_resize
lglw_bool_t lglw_window_resize (lglw_t _lglw, int32_t _w, int32_t _h) {
   lglw_bool_t r = LGLW_FALSE;
   LGLW(_lglw);

   if(NULL != lglw)
   {
      if(NULL != lglw->win.hwnd)
      {
         r = LGLW_TRUE;

         SetWindowPos(lglw->win.hwnd,
                      HWND_TOP,
                      0, 0, // x/y
                      _w, _h,
                      SWP_NOMOVE | SWP_SHOWWINDOW
                      );

         int deltaW = _w - lglw->win.size.x;
         int deltaH = _h - lglw->win.size.y;

         lglw->win.size.x = _w;
         lglw->win.size.y = _h;

         // Resize parent window (if any)
         {
            HWND hwnd;
            hwnd = GetAncestor(lglw->win.hwnd, GA_ROOT);
            Dprintf("xxx lglw_window_resize: ancestor=%p\n", hwnd);
            if(NULL != hwnd)
            {
               RECT rect;
               if(GetWindowRect(hwnd, &rect))
               {
                  rect.right += deltaW;
                  rect.bottom += deltaH;
                  
                  SetWindowPos(hwnd,
                               HWND_TOP,
                               0, 0, // x/y
                               rect.right - rect.left, rect.bottom - rect.top,
                               SWP_NOMOVE | SWP_FRAMECHANGED | SWP_SHOWWINDOW
                               );
               }
            }
         }
      }
   }

   return r;
}


// ---------------------------------------------------------------------------- lglw_window_close
void lglw_window_close (lglw_t _lglw) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      if(NULL != lglw->win.hwnd)
      {
         lglw_timer_stop(_lglw);

         loc_key_unhook(lglw);

         DestroyWindow(lglw->win.hwnd);
         lglw->win.hwnd = NULL;
         lglw->win.hdc  = NULL;

         UnregisterClass(lglw->win.class_name, GetModuleHandle(NULL));

         // window_to_wrapper = NULL;
      }
   }
}


// ---------------------------------------------------------------------------- lglw_window_show
void lglw_window_show(lglw_t _lglw) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      if(NULL != lglw->win.hwnd)
      {
         ShowWindow(lglw->win.hwnd, SW_SHOWNORMAL);
      }
   }
}


// ---------------------------------------------------------------------------- lglw_window_hide
void lglw_window_hide(lglw_t _lglw) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      if(NULL != lglw->win.hwnd)
      {
         ShowWindow(lglw->win.hwnd, SW_HIDE);
      }
   }
}


// ---------------------------------------------------------------------------- lglw_window_is_visible
lglw_bool_t lglw_window_is_visible(lglw_t _lglw) {
   lglw_bool_t r = LGLW_FALSE;
   LGLW(_lglw);

   if(NULL != lglw)
   {
      r = (lglw_bool_t)IsWindowVisible(lglw->win.hwnd);
   }

   return r;
}


// ---------------------------------------------------------------------------- lglw_window_size_get
void lglw_window_size_get(lglw_t _lglw, int32_t *_retX, int32_t *_retY) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      if(NULL != lglw->win.hwnd)
      {
         if(NULL != _retX)
            *_retX = lglw->win.size.x;

         if(NULL != _retY)
            *_retY = lglw->win.size.y;
      }
   }
}


// ---------------------------------------------------------------------------- lglw_redraw
void lglw_redraw(lglw_t _lglw) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      if(NULL != lglw->win.hwnd)
      {
         RedrawWindow(lglw->win.hwnd, NULL/*lprcUpdate*/, NULL/*hrgnUpdate*/, RDW_INTERNALPAINT | RDW_UPDATENOW);
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
      lglw->prev.hdc   = wglGetCurrentDC();
      lglw->prev.hglrc = wglGetCurrentContext();
      // Dprintf("xxx lglw_glcontext_push: prev.hdc=%p prev.hglrc=%p\n", lglw->prev.hdc, lglw->prev.hglrc);

      // Dprintf("xxx lglw_glcontext_push: win.hdc=%p lglw->hglrc=%p\n", lglw->win.hdc, lglw->hglrc);
      if(!wglMakeCurrent((NULL == lglw->win.hdc) ? lglw->hidden.hdc : lglw->win.hdc, lglw->hglrc))
      {
         printf("[---] lglw_glcontext_push: wglMakeCurrent() failed. win.hdc=%p hidden.hdc=%p hglrc=%p GetLastError()=%d\n", lglw->win.hdc, lglw->hidden.hdc, lglw->hglrc, GetLastError());
      }
   }
}


// ---------------------------------------------------------------------------- lglw_glcontext_pop
void lglw_glcontext_pop(lglw_t _lglw) {
   LGLW(_lglw);
   
   if(NULL != lglw)
   {
      // Dprintf("xxx lglw_glcontext_pop: prev.hdc=%p prev.hglrc=%p\n", lglw->prev.hdc, lglw->prev.hglrc);
      if(!wglMakeCurrent(lglw->prev.hdc, lglw->prev.hglrc))
      {
         printf("[---] lglw_glcontext_pop: wglMakeCurrent() failed. prev.hdc=%p hglrc=%p GetLastError()=%d\n", lglw->prev.hdc, lglw->prev.hglrc, GetLastError());
      }
   }
}


// ---------------------------------------------------------------------------- lglw_swap_buffers
void lglw_swap_buffers(lglw_t _lglw) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      wglSwapLayerBuffers(lglw->win.hdc, WGL_SWAP_MAIN_PLANE);
   }
}


// ---------------------------------------------------------------------------- lglw_swap_interval_set
typedef void (APIENTRY *PFNWGLEXTSWAPINTERVALPROC) (int);
void lglw_swap_interval_set(lglw_t _lglw, int32_t _ival) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      PFNWGLEXTSWAPINTERVALPROC wglSwapIntervalEXT;
      wglSwapIntervalEXT = (PFNWGLEXTSWAPINTERVALPROC) wglGetProcAddress("wglSwapIntervalEXT");
      if(NULL != wglSwapIntervalEXT)
      {
         wglSwapIntervalEXT(_ival);
         lglw->win.swap_interval = _ival;
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


// ---------------------------------------------------------------------------- loc_key_hook
static void loc_key_hook(lglw_int_t *lglw) {
   loc_key_unhook(lglw);

   // https://msdn.microsoft.com/en-us/library/windows/desktop/ms644990(v=vs.85).aspx
   lglw->win.keyboard_ll_hhook = SetWindowsHookEx(WH_KEYBOARD_LL, &loc_LowLevelKeyboardProc, GetModuleHandle(NULL), 0/*dwThreadId*/);

   khook_lglw = lglw;
}


// ---------------------------------------------------------------------------- loc_key_unhook
static void loc_key_unhook(lglw_int_t *lglw) {
   if(NULL != lglw->win.keyboard_ll_hhook)
   {
      UnhookWindowsHookEx(lglw->win.keyboard_ll_hhook);
      lglw->win.keyboard_ll_hhook = NULL;

      if(khook_lglw == lglw)
         khook_lglw = NULL;
   }
}


// ---------------------------------------------------------------------------- loc_handle_mouseleave
static void loc_handle_mouseleave(lglw_int_t *lglw) {
   loc_key_unhook(lglw);

   lglw->focus.state &= ~LGLW_FOCUS_MOUSE;

   if(NULL != lglw->focus.cbk)
   {
      lglw->focus.cbk(lglw, lglw->focus.state, LGLW_FOCUS_MOUSE);
   }

   Dprintf("xxx lglw:loc_handle_mouseleave: LEAVE\n");
}


// ---------------------------------------------------------------------------- loc_handle_mouseenter
static void loc_handle_mouseenter(lglw_int_t *lglw) {
   TRACKMOUSEEVENT tme;
   ZeroMemory(&tme, sizeof(tme));
   tme.cbSize      = sizeof(TRACKMOUSEEVENT);
   tme.dwFlags     = TME_LEAVE;
   tme.hwndTrack   = lglw->win.hwnd;
   tme.dwHoverTime = HOVER_DEFAULT;
   (void)TrackMouseEvent(&tme);
   
   loc_key_hook(lglw);

   lglw->focus.state |= LGLW_FOCUS_MOUSE;

   if(NULL != lglw->focus.cbk)
   {
      lglw->focus.cbk(lglw, lglw->focus.state, LGLW_FOCUS_MOUSE);
   }

   Dprintf("xxx lglw:loc_handle_mouseenter: LEAVE\n");
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


// ---------------------------------------------------------------------------- loc_touchkeyboard_get_rect
#if 0
static lglw_bool_t loc_touchkeyboard_get_rect(RECT *rect) {
   lglw_bool_t r = LGLW_FALSE;
   //*******************************************************************
   //
   // RETURNS KEYBOARD RECTANGLE OR EMPTY ONE IF KEYBOARD IS NOT VISIBLE
   //
   //*******************************************************************
   // by "Sevast", <https://stackoverflow.com/questions/38774139/show-touch-keyboard-tabtip-exe-in-windows-10-anniversary-edition>
   IFrameworkInputPane *inputPane = NULL;
   HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);    
   rect->left = rect->right = rect->top = rect->bottom = 0; // screen location
   if (SUCCEEDED(hr))
   {
      hr = CoCreateInstance(CLSID_FrameworkInputPane, NULL, CLSCTX_INPROC_SERVER, IID_IFrameworkInputPane, (LPVOID*)&inputPane);
      if (SUCCEEDED(hr))
      {
         hr=inputPane->Location(rect);
         if(SUCCEEDED(hr))
         {
            r = LGLW_TRUE;
         }
         inputPane->Release();
      }
   }
   CoUninitialize();
   return r;
}
#endif



// ---------------------------------------------------------------------------- loc_touchkeyboard_is_visible
#if 0
static lglw_bool_t loc_touchkeyboard_is_visible(void) {
   lglw_bool_t r = LGLW_FALSE;
   HWND hwnd = FindWindow("IPTIP_Main_Window", NULL);
   printf("xxx lglw: IPTIP hwnd=%p\n", hwnd);
   if(NULL != hwnd)
   {
#if 0
      // (note) does not work, style flags are always the same and rect is always (0; 0; 0; 0)
      LONG style = GetWindowLong(hwnd, GWL_STYLE);
      RECT rect;
      r = (0u != (style & WS_VISIBLE)) && (0u == (style & WS_DISABLED));
      GetWindowRect(hwnd, &rect);
      printf("xxx lglw: touchkeyboard_is_visible: style=0x%08x rect=(%d; %d; %d; %d) r=%d\n", style, rect.top, rect.left, rect.bottom, rect.right, r);
#endif
      // (note) does not work, either
      static const char *WindowClass1709 = "Windows.UI.Core.CoreWindow";
      static const char *WindowCaption1709 = "Microsoft Text Input Application";
      static const char *WindowParentClass1709 = "ApplicationFrameWindow";
      HWND hwnd1709 = FindWindowEx(NULL, NULL, WindowClass1709, WindowCaption1709);
      printf("xxx lglw: touchkeyboard_is_visible: hwnd1709=%p\n", hwnd1709);
      // if there is a top-level window - the keyboard is closed
      if(NULL != hwnd1709)
      {
         r = LGLW_FALSE;
      }
      else
      {
         HWND parent = NULL;
         for(;;)
         {
            HWND childHWND;

            parent = FindWindowEx(NULL, parent, WindowParentClass1709, NULL);
            if(NULL == parent)
               break; // no more windows, keyboard state is unknown
            
            // if it's a child of a WindowParentClass1709 window - the keyboard is open
            childHWND = FindWindowEx(parent, NULL, WindowClass1709, WindowCaption1709);

            if (NULL != childHWND)
            {
               r = LGLW_TRUE;
               break;
            }
         }
      }
   }
   return r;
}
#endif


// ---------------------------------------------------------------------------- lglw_touchkeyboard_show
void lglw_touchkeyboard_show(lglw_t _lglw, lglw_bool_t _bEnable) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
#if 0
      RECT keyRect;
      if(loc_touchkeyboard_get_rect(&keyRect))
      {
         lglw_bool_t bToggle = _bEnable ^ ((keyRect.right - keyRect.left) > 0);

         if(bToggle)
         {
            loc_touchkeyboard_toggle();
         }
      }
#endif

#if 0
      lglw_bool_t bToggle = _bEnable ^ loc_touchkeyboard_is_visible();

      if(bToggle)
      {
         lglw_int_touchkeyboard_toggle();
      }
#endif

      // (note) there seems to be no (sane) way to determine whether the virtual keyboard is visible.
      //        the solutions suggested on stack overflow don't work (anymore).
      //         => just toggle the kbd
      lglw_int_touchkeyboard_toggle();
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
      if(NULL != lglw->win.hwnd)
      {
         if(!lglw->mouse.touch.b_enable)
         {
            if(lglw->mouse.grab.mode != _grabMode)
            {
               lglw_mouse_ungrab(_lglw);
            }

            switch(_grabMode)
            {
               default:
               case LGLW_MOUSE_GRAB_NONE:
                  break;

               case LGLW_MOUSE_GRAB_CAPTURE:
                  (void)SetCapture(lglw->win.hwnd);
                  lglw->mouse.grab.mode = _grabMode;
                  break;

               case LGLW_MOUSE_GRAB_WARP:
                  (void)SetCapture(lglw->win.hwnd);
                  lglw_mouse_cursor_show(_lglw, LGLW_FALSE);
                  lglw->mouse.grab.p = lglw->mouse.p;
                  lglw->mouse.grab.last_p = lglw->mouse.p;
                  lglw->mouse.grab.mode = _grabMode;
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
      if(NULL != lglw->win.hwnd)
      {
         if(!lglw->mouse.touch.b_enable)
         {
            switch(lglw->mouse.grab.mode)
            {
               default:
               case LGLW_MOUSE_GRAB_NONE:
                  break;

               case LGLW_MOUSE_GRAB_CAPTURE:
                  (void)ReleaseCapture();
                  lglw->mouse.grab.mode = LGLW_MOUSE_GRAB_NONE;
                  break;

               case LGLW_MOUSE_GRAB_WARP:
                  (void)ReleaseCapture();
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
      if(NULL != lglw->win.hwnd)
      {
         POINT p;
         p.x = _x;
         p.y = _y;

         if(ClientToScreen(lglw->win.hwnd, &p))
         {
            SetCursorPos(p.x, p.y);

            if(LGLW_MOUSE_GRAB_WARP != lglw->mouse.grab.mode)
            {
               lglw->mouse.p.x = p.x;
               lglw->mouse.p.y = p.y;
            }
         }
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
      if(NULL != lglw->win.hwnd)
      {
         (void)ShowCursor(_bShow);
      }
   }
}


// ---------------------------------------------------------------------------- lglw_timer_start
void lglw_timer_start(lglw_t _lglw, uint32_t _millisec) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      if(NULL != lglw->win.hwnd)
      {
         (void)SetTimer(lglw->win.hwnd, 1/*nIDEvent*/, _millisec, NULL/*lpTimerFunc*/);
         lglw->timer.b_running = LGLW_TRUE;
      }
   }
}


// ---------------------------------------------------------------------------- lglw_timer_stop
void lglw_timer_stop(lglw_t _lglw) {
   LGLW(_lglw);

   if(NULL != lglw)
   {
      if(NULL != lglw->win.hwnd)
      {
         if(lglw->timer.b_running)
         {
            (void)KillTimer(lglw->win.hwnd, 1/*nIDEvent*/);
            lglw->timer.b_running = LGLW_FALSE;
         }
      }
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


// ---------------------------------------------------------------------------- loc_enable_dropfiles
static void loc_enable_dropfiles(lglw_int_t *lglw, lglw_bool_t _bEnable) {
   if(NULL != lglw->win.hwnd)
   {
      DragAcceptFiles(lglw->win.hwnd, _bEnable ? TRUE : FALSE);
   }
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


// ---------------------------------------------------------------------------- loc_touchinput_update
typedef void (*EnableMouseInPointer_fxn_t)(BOOL fEnable);
static void loc_touchinput_update(lglw_int_t *lglw) {
   HMODULE hDLL = LoadLibrary("User32.dll");

   if(NULL != hDLL)
   {
      EnableMouseInPointer_fxn_t enableMouseInPointer = (EnableMouseInPointer_fxn_t)
         GetProcAddress(hDLL, "EnableMouseInPointer");

      if(NULL != enableMouseInPointer)
      {
         Dprintf("[dbg] lglw: EnableMouseInPointer() detected (win8+)\n");
         (void)enableMouseInPointer(lglw->mouse.touch.b_enable);
      }

      FreeLibrary(hDLL);
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
         if(NULL != lglw->win.hwnd)
         {
            uint32_t numChars = (0u == _numChars) ? ((uint32_t)strlen(_text)+1u) : _numChars;

            if(numChars > 0u)
            {
               if(OpenClipboard(lglw->win.hwnd))
               {
                  HANDLE hGlobal = GlobalAlloc(GMEM_MOVEABLE, numChars * sizeof(TCHAR));

                  if(NULL != hGlobal)
                  {
                     char *d = (char*) GlobalLock(hGlobal);

                     if(NULL != d)
                     {
                        uint32_t i;
                        for(i = 0u; i < numChars; i++)
                        {
                           d[i] = _text[i];
                        }
                        d[numChars - 1] = 0;

                        Dprintf("xxx lglw_clipboard_text_set: SetClipboardData(\"%s\")\n", d);

                        GlobalUnlock(hGlobal);
                        (void)SetClipboardData(CF_TEXT, hGlobal);
                     }
                  }

                  CloseClipboard();
               }
            }
         }
      }
   }
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
         if(NULL != lglw->win.hwnd)
         {
            if(OpenClipboard(lglw->win.hwnd))
            {
               const char *data = (const char*)GetClipboardData(CF_TEXT);

               Dprintf("xxx lglw_clipboard_text_get: data=\"%s\"\n", data);

               if(NULL != data)
               {
                  uint32_t i = 0u;

                  for(; i < _maxChars; i++)
                  {
                     _retText[i] = data[i];

                     if(0 == _retText[i])
                        break;
                  }
                  _retText[_maxChars - 1u] = 0;  // ASCIIZ

                  if(NULL != _retNumChars)
                  {
                     *_retNumChars = i;
                  }
               }

               CloseClipboard();
            }
         }
      }
   }
}


// ---------------------------------------------------------------------------- loc_LowLevelKeyboardProc
static LRESULT CALLBACK loc_LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {

   Dprintf("xxx lglw:loc_LowLevelKeyboardProc: wParam=0x%08x lParam=0x%08x\n", (uint32_t)wParam, (uint32_t)lParam);

   if(HC_ACTION == nCode)
   {
      if(NULL != khook_lglw)
      {
         KBDLLHOOKSTRUCT *hs = (KBDLLHOOKSTRUCT*)lParam;
         lglw_bool_t bHandled = LGLW_FALSE;

         switch(wParam)
         {
            default:
               break;

            case WM_KEYDOWN:
               // Dprintf("xxx lglw:loc_LowLevelKeyboardProc: WM_KEYDOWN vkCode=0x%08x scanCode=0x%08x bExt=%d\n", hs->vkCode, hs->scanCode, (0 != (hs->flags & LLKHF_EXTENDED)));

               switch(hs->vkCode)
               {
                  default:
                     if(0u == (hs->flags & LLKHF_EXTENDED))
                     {
                        uint16_t ucBuf[16];
                        BYTE keyState[256];

                        // Dprintf("xxx lglw call toUnicode\n");
                        GetKeyboardState(keyState);

                        // don't let e.g. ctrl-<a..z> produce symbol chars
                        keyState[VK_CONTROL] = 0;
                        keyState[VK_LCONTROL] = 0;
                        keyState[VK_RCONTROL] = 0;

                        if(ToUnicode(hs->vkCode, hs->scanCode, keyState, (LPWSTR)ucBuf, 8/*cchBuff*/, 0/*wFlags*/) >= 1)
                        {
                           // Dprintf("xxx lglw toUnicode: ucBuf[0]=0x%04x\n", ucBuf[0]);
                           bHandled = loc_handle_key(khook_lglw, LGLW_TRUE/*bPressed*/, ucBuf[0]);
                        }
                        else
                        {
                           Dprintf("xxx lglw:loc_LowLevelKeyboardProc<down>: ToUnicode failed\n");
                        }
                     }
                     else
                     {
                        bHandled = loc_handle_key(khook_lglw, LGLW_TRUE/*bPressed*/, hs->vkCode | LGLW_VKEY_EXT);
                        bHandled = LGLW_FALSE;
                     }
                     break;

                  case VK_F1:
                  case VK_F2:
                  case VK_F3:
                  case VK_F4:
                  case VK_F5:
                  case VK_F6:
                  case VK_F7:
                  case VK_F8:
                  case VK_F9:
                  case VK_F10:
                  case VK_F11:
                  case VK_F12:
                  case VK_BACK:
                  case VK_TAB:
                  case VK_RETURN:
                  case VK_ESCAPE:
                     bHandled = loc_handle_key(khook_lglw, LGLW_TRUE/*bPressed*/, hs->vkCode | LGLW_VKEY_EXT);
                     break;

                  case VK_LSHIFT:
                     // Dprintf("xxx lglw:loc_LowLevelKeyboardProc<down>: VK_LSHIFT\n");
                     khook_lglw->keyboard.kmod_state |= LGLW_KMOD_LSHIFT;
                     bHandled = loc_handle_key(khook_lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_LSHIFT);
                     bHandled = LGLW_FALSE;
                     break;

                  case VK_RSHIFT:
                     // Dprintf("xxx lglw:loc_LowLevelKeyboardProc<down>: VK_RSHIFT\n");
                     khook_lglw->keyboard.kmod_state |= LGLW_KMOD_RSHIFT;
                     bHandled = loc_handle_key(khook_lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_RSHIFT);
                     bHandled = LGLW_FALSE;
                     break;

                  case VK_LCONTROL:
                     // Dprintf("xxx lglw:loc_LowLevelKeyboardProc<down>: VK_LCONTROL\n");
                     khook_lglw->keyboard.kmod_state |= LGLW_KMOD_LCTRL;
                     bHandled = loc_handle_key(khook_lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_LCTRL);
                           bHandled = LGLW_FALSE;
                     break;

                  case VK_RCONTROL:
                     // Dprintf("xxx lglw:loc_LowLevelKeyboardProc<down>: VK_RCONTROL\n");
                     khook_lglw->keyboard.kmod_state |= LGLW_KMOD_RCTRL;
                     bHandled = loc_handle_key(khook_lglw, LGLW_TRUE/*bPressed*/, LGLW_VKEY_RCTRL);
                     bHandled = LGLW_FALSE;
                     break;

                     //case VK_RWIN:
                     //case VK_F1:
                  case VK_LMENU: // alt
                     // not received
                     // Dprintf("xxx lglw:loc_LowLevelKeyboardProc<down>: VK_LMENU\n");
                     break;

                  case VK_RMENU:
                     // Dprintf("xxx lglw:loc_LowLevelKeyboardProc<down>: VK_RMENU\n");
                     break;
               }

               break;

            case WM_KEYUP:
               // Dprintf("xxx lglw:loc_LowLevelKeyboardProc: WM_KEYUP vkCode=0x%08x scanCode=0x%08x bExt=%d\n", hs->vkCode, hs->scanCode, (0 != (hs->flags & LLKHF_EXTENDED)));
               switch(hs->vkCode)
               {
                  default:
                     if(0u == (hs->flags & LLKHF_EXTENDED))
                     {
                        uint16_t ucBuf[16];
                        BYTE keyState[256];

                        GetKeyboardState(keyState);

                        if(ToUnicode(hs->vkCode, hs->scanCode, keyState, (LPWSTR)ucBuf, 8/*cchBuff*/, 0/*wFlags*/) >= 1)
                        {
                           bHandled = loc_handle_key(khook_lglw, LGLW_FALSE/*bPressed*/, ucBuf[0]);
                        }
                     }
                     else
                     {
                        bHandled = loc_handle_key(khook_lglw, LGLW_FALSE/*bPressed*/, hs->vkCode | LGLW_VKEY_EXT);
                        bHandled = LGLW_FALSE;
                     }
                     break;

                  case VK_F1:
                  case VK_F2:
                  case VK_F3:
                  case VK_F4:
                  case VK_F5:
                  case VK_F6:
                  case VK_F7:
                  case VK_F8:
                  case VK_F9:
                  case VK_F10:
                  case VK_F11:
                  case VK_F12:
                  case VK_BACK:
                  case VK_TAB:
                  case VK_RETURN:
                  case VK_ESCAPE:
                     bHandled = loc_handle_key(khook_lglw, LGLW_FALSE/*bPressed*/, hs->vkCode | LGLW_VKEY_EXT);
                     break;

                  case VK_LSHIFT:
                     khook_lglw->keyboard.kmod_state &= ~LGLW_KMOD_LSHIFT;
                     bHandled = loc_handle_key(khook_lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_LSHIFT);
                     bHandled = LGLW_FALSE;
                     break;

                  case VK_RSHIFT:
                     khook_lglw->keyboard.kmod_state &= ~LGLW_KMOD_RSHIFT;
                     bHandled = loc_handle_key(khook_lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_RSHIFT);
                     bHandled = LGLW_FALSE;
                     break;

                  case VK_LCONTROL:
                     khook_lglw->keyboard.kmod_state &= ~LGLW_KMOD_LCTRL;
                     bHandled = loc_handle_key(khook_lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_LCTRL);
                     bHandled = LGLW_FALSE;
                     break;

                  case VK_RCONTROL:
                     khook_lglw->keyboard.kmod_state &= ~LGLW_KMOD_RCTRL;
                     bHandled = loc_handle_key(khook_lglw, LGLW_FALSE/*bPressed*/, LGLW_VKEY_RCTRL);
                     bHandled = LGLW_FALSE;
                     break;
               }
               break;
         }

         if(!bHandled)
         {
            Dprintf("xxx lglw: CallNextHookEx<kbd>\n");
            return CallNextHookEx(NULL, nCode, wParam, lParam);
         }
         else
         {
            Dprintf("xxx lglw: CallNextHookEx<kbd>: was handled\n");
            return 1;
         }

      } // if khook_lglw
   }
   return CallNextHookEx(NULL, nCode, wParam, lParam);
}


// ---------------------------------------------------------------------------- loc_WndProc
LRESULT CALLBACK loc_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

   LGLW(GetWindowLongPtr(hWnd, GWLP_USERDATA));

   if(NULL != lglw)
   {
      // Dprintf("xxx lglw:loc_WndProc: message=%08x wParam=0x%08x lParam=0x%08x\n", message, (uint32_t)wParam, (uint32_t)lParam);

      switch(message)
      {
         case WM_CREATE:
            Dprintf("xxx lglw: WM_CREATE\n");
            break;

         case WM_DESTROY:
            Dprintf("xxx lglw: WM_DESTROY\n");
            break;

         case WM_ACTIVATEAPP: // 0x1c  wParam=0/1
            Dprintf("xxx lglw: WM_ACTIVATEAPP wParam=%d\n", (int32_t)wParam);
            break;

         case WM_MOUSELEAVE:
            // (note) only received after TrackMouseEvent() has been called
            Dprintf("xxx lglw: WM_MOUSELEAVE\n");
            loc_handle_mouseleave(lglw);
            break;

         case WM_MOUSEMOVE:
            // Dprintf("xxx lglw: WM_MOUSEMOVE:\n");
            if(lglw->mouse.touch.b_update_queued)
            {
               loc_touchinput_update(lglw);
               lglw->mouse.touch.b_update_queued = LGLW_FALSE;
            }
            if(!lglw->mouse.touch.b_enable)
            {
               int32_t x = GET_X_LPARAM(lParam);  // lo
               int32_t y = GET_Y_LPARAM(lParam);  // hi

               if(LGLW_MOUSE_GRAB_WARP == lglw->mouse.grab.mode)
               {
                  lglw->mouse.grab.b_queue_warp = LGLW_TRUE;

                  lglw->mouse.p.x += (x - lglw->mouse.grab.last_p.x);
                  lglw->mouse.p.y += (y - lglw->mouse.grab.last_p.y);

                  lglw->mouse.grab.last_p.x = x;
                  lglw->mouse.grab.last_p.y = y;
               }
               else
               {
                  lglw->mouse.p.x = x;
                  lglw->mouse.p.y = y;
               }

               if(0u == (lglw->focus.state & LGLW_FOCUS_MOUSE))
               {
                  loc_handle_mouseenter(lglw);
               }

               loc_handle_mousemotion(lglw);
            }
            break;

         case WM_LBUTTONDOWN:
            if(!lglw->mouse.touch.b_enable)
            {
               // Dprintf("xxx lglw: WM_LBUTTONDOWN\n");
               loc_handle_mousebutton(lglw, LGLW_TRUE/*bPressed*/, LGLW_MOUSE_LBUTTON);
            }
            break;

         case WM_LBUTTONUP:
            if(!lglw->mouse.touch.b_enable)
            {
               // Dprintf("xxx lglw: WM_LBUTTONUP\n");
               loc_handle_mousebutton(lglw, LGLW_FALSE/*bPressed*/, LGLW_MOUSE_LBUTTON);
            }
            break;

         case WM_RBUTTONDOWN:
            if(!lglw->mouse.touch.b_enable)
            {
               // Dprintf("xxx lglw: WM_RBUTTONDOWN\n");
               loc_handle_mousebutton(lglw, LGLW_TRUE/*bPressed*/, LGLW_MOUSE_RBUTTON);
            }
            break;

         case WM_RBUTTONUP:
            if(!lglw->mouse.touch.b_enable)
            {
               // Dprintf("xxx lglw: WM_RBUTTONUP\n");
               loc_handle_mousebutton(lglw, LGLW_FALSE/*bPressed*/, LGLW_MOUSE_RBUTTON);
            }
            break;

         case WM_MBUTTONDOWN:
            if(!lglw->mouse.touch.b_enable)
            {
               // Dprintf("xxx lglw: WM_MBUTTONDOWN\n");
               loc_handle_mousebutton(lglw, LGLW_TRUE/*bPressed*/, LGLW_MOUSE_MBUTTON);
            }
            break;

         case WM_MBUTTONUP:
            if(!lglw->mouse.touch.b_enable)
            {
               // Dprintf("xxx lglw: WM_MBUTTONUP\n");
               loc_handle_mousebutton(lglw, LGLW_FALSE/*bPressed*/, LGLW_MOUSE_MBUTTON);
            }
            break;

         case WM_MOUSEWHEEL:
            // Dprintf("xxx lglw: WM_MOUSEWHEEL\n");
            {
               uint32_t bt = (((int16_t)(((uint32_t)wParam)>>16)) > 0) ? LGLW_MOUSE_WHEELUP : LGLW_MOUSE_WHEELDOWN;
               loc_handle_mousebutton(lglw, LGLW_TRUE/*bPressed*/, bt);
               loc_handle_mousebutton(lglw, LGLW_FALSE/*bPressed*/, bt);
            }
            break;

         case WM_CAPTURECHANGED:
            // e.g. after alt-tab
            Dprintf("xxx lglw: WM_CAPTURECHANGED\n");
            lglw->mouse.grab.mode = LGLW_MOUSE_GRAB_NONE;
            break;

#if 1
         // (note) VST windows usually don't receive key/char messages (they are consumed by the DAW)

         case WM_GETDLGCODE: 
            // never received
            Dprintf("xxx lglw: WM_GETDLGCODE\n");
            return DLGC_WANTALLKEYS;

         case WM_KEYDOWN:
            // never received
            Dprintf("xxx lglw: WM_KEYDOWN nVirtKey=0x%08x lKeydata=0x%08x\n", (uint32_t)wParam, (uint32_t)lParam);
            break;

         case WM_KEYUP:
            // never received
            Dprintf("xxx lglw: WM_KEYUP nVirtKey=0x%08x lKeydata=0x%08x\n", (uint32_t)wParam, (uint32_t)lParam);
            break;

         case WM_CHAR:
            // never received
            Dprintf("xxx lglw: WM_CHAR charCode=0x%08x bPressed=%u\n", (uint32_t)wParam, (((uint32_t)lParam)>>31)&1u);
            break;
#endif

         case WM_PAINT:
            // https://docs.microsoft.com/en-us/windows/desktop/api/Winuser/nf-winuser-redrawwindow
            // Dprintf("xxx lglw: WM_PAINT\n");
            loc_handle_queued_mouse_warp(lglw);
            if(NULL != lglw->redraw.cbk)
            {
               lglw->redraw.cbk(lglw);
            }
            break;

         case WM_TIMER:
            // Dprintf("xxx lglw: WM_TIMER cbk=%p\n", lglw->timer.cbk);
            if(lglw->mouse.touch.b_enable)
            {
               // Dprintf("xxx lglw: WM_TIMER hold_state=%u\n", lglw->mouse.touch.syn_rmb_hold_state);
               if(LGLW_MOUSE_TOUCH_RMB_STATE_IDLE != lglw->mouse.touch.syn_rmb_hold_state)
               {
                  uint32_t ms = (uint32_t)GetTickCount();
                  uint32_t delta;
                  if(ms < lglw->mouse.touch.hold_start_ms)
                  {
                     // Overflow after 49.7 days
                     delta = ~lglw->mouse.touch.hold_start_ms + 1u + ms;
                  }
                  else
                  {
                     delta = (ms - lglw->mouse.touch.hold_start_ms);
                  }

                  if(LGLW_MOUSE_TOUCH_RMB_STATE_LMB == lglw->mouse.touch.syn_rmb_hold_state)
                  {
                     if(delta >= LGLW_MOUSE_TOUCH_LMB_TIMEOUT)
                     {
                        loc_handle_mousebutton(lglw, LGLW_FALSE/*bPressed*/, LGLW_MOUSE_LBUTTON);
                        lglw->mouse.touch.syn_rmb_hold_state = LGLW_MOUSE_TOUCH_RMB_STATE_WAIT;
                     }
                  }

                  if(LGLW_MOUSE_TOUCH_RMB_STATE_WAIT == lglw->mouse.touch.syn_rmb_hold_state)
                  {
                     if(delta >= LGLW_MOUSE_TOUCH_RMB_TIMEOUT)
                     {
                        lglw->mouse.touch.syn_rmb_hold_state = LGLW_MOUSE_TOUCH_RMB_STATE_RMB;
                        loc_handle_mousebutton(lglw, LGLW_TRUE/*bPressed*/, LGLW_MOUSE_RBUTTON);
                     }
                  }
               }
            }
            else
            {
               loc_handle_queued_mouse_warp(lglw);
            }            

            if(NULL != lglw->timer.cbk)
            {
               lglw->timer.cbk(lglw);
            }
            break;

           case WM_DROPFILES:
              if(NULL != lglw->dropfiles.cbk)
              {
                 HDROP hDrop = (HDROP) wParam;
                 POINT p;

                 if(TRUE == DragQueryPoint(hDrop, &p))
                 {
                    Dprintf("xxx lglw: WM_DROPFILES: p=(%d; %d)\n", p.x, p.y);

                    UINT numFiles = DragQueryFile(hDrop, 0xffffffff, NULL, 0);
                    Dprintf("xxx lglw: WM_DROPFILES: dropped %u file(s)\n", numFiles);

                    if(numFiles > 0u)
                    {
                       UINT i;
                       char **pathnames = malloc((sizeof(char*) * numFiles) + (MAX_PATH * sizeof(char) * numFiles));
                       char *pathname = (char*)&pathnames[numFiles];

                       for(i=0; i<numFiles; i++)
                       {
                          DragQueryFile(hDrop, i, pathname, MAX_PATH);
                          Dprintf("xxx lglw: WM_DROPFILES: file[%d] = \"%s\"\n", i, pathname);
                          pathnames[i] = pathname;
                          pathname += MAX_PATH;
                       }

                       lglw->dropfiles.cbk(lglw, p.x, p.y, numFiles, pathnames);

                       free(pathnames);
                    }
                 }
              }
              break;

#ifdef BUILD_64
              // Touch messages:
#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))
#define GET_POINTERID_WPARAM(wParam)                (LOWORD(wParam))
#define IS_POINTER_FLAG_SET_WPARAM(wParam, flag)    (((DWORD)HIWORD(wParam) & (flag)) == (flag))
#define IS_POINTER_NEW_WPARAM(wParam)               IS_POINTER_FLAG_SET_WPARAM(wParam, POINTER_MESSAGE_FLAG_NEW)
#define IS_POINTER_INRANGE_WPARAM(wParam)           IS_POINTER_FLAG_SET_WPARAM(wParam, POINTER_MESSAGE_FLAG_INRANGE)
#define IS_POINTER_INCONTACT_WPARAM(wParam)         IS_POINTER_FLAG_SET_WPARAM(wParam, POINTER_MESSAGE_FLAG_INCONTACT)
#define IS_POINTER_FIRSTBUTTON_WPARAM(wParam)       IS_POINTER_FLAG_SET_WPARAM(wParam, POINTER_MESSAGE_FLAG_FIRSTBUTTON)
#define IS_POINTER_SECONDBUTTON_WPARAM(wParam)      IS_POINTER_FLAG_SET_WPARAM(wParam, POINTER_MESSAGE_FLAG_SECONDBUTTON)
#define IS_POINTER_THIRDBUTTON_WPARAM(wParam)       IS_POINTER_FLAG_SET_WPARAM(wParam, POINTER_MESSAGE_FLAG_THIRDBUTTON)
#define IS_POINTER_FOURTHBUTTON_WPARAM(wParam)      IS_POINTER_FLAG_SET_WPARAM(wParam, POINTER_MESSAGE_FLAG_FOURTHBUTTON)
#define IS_POINTER_FIFTHBUTTON_WPARAM(wParam)       IS_POINTER_FLAG_SET_WPARAM(wParam, POINTER_MESSAGE_FLAG_FIFTHBUTTON)
#define IS_POINTER_PRIMARY_WPARAM(wParam)           IS_POINTER_FLAG_SET_WPARAM(wParam, POINTER_MESSAGE_FLAG_PRIMARY)
#define HAS_POINTER_CONFIDENCE_WPARAM(wParam)       IS_POINTER_FLAG_SET_WPARAM(wParam, POINTER_MESSAGE_FLAG_CONFIDENCE)
#define IS_POINTER_CANCELED_WPARAM(wParam)          IS_POINTER_FLAG_SET_WPARAM(wParam, POINTER_MESSAGE_FLAG_CANCELED)
#endif
           case WM_POINTERDOWN:
           {
              POINT p;

              lglw->mouse.touch.b_enable = LGLW_TRUE;
              lglw->mouse.touch.b_update_queued = LGLW_FALSE;

               if(0u == (lglw->focus.state & LGLW_FOCUS_MOUSE))
               {
                  loc_handle_mouseenter(lglw);
               }

              // Dprintf("xxx lglw: WM_POINTERDOWN\n");

              // Dprintf("xxx lglw: WM_POINTERDOWN: IS_POINTER_FIRSTBUTTON_WPARAM(wParam)=%d\n", IS_POINTER_FIRSTBUTTON_WPARAM(wParam));
              // Dprintf("xxx lglw: WM_POINTERDOWN: IS_POINTER_SECONDBUTTON_WPARAM(wParam)=%d\n", IS_POINTER_SECONDBUTTON_WPARAM(wParam));
              // Dprintf("xxx lglw: WM_POINTERDOWN: IS_POINTER_THIRDBUTTON_WPARAM(wParam)=%d\n", IS_POINTER_THIRDBUTTON_WPARAM(wParam));

              p.x = GET_X_LPARAM(lParam);
              p.y = GET_Y_LPARAM(lParam);
              (void)ScreenToClient(lglw->win.hwnd, &p);
              lglw->mouse.p.x = p.x;
              lglw->mouse.p.y = p.y;
              loc_handle_mousemotion(lglw);

              if(IS_POINTER_FIRSTBUTTON_WPARAM(wParam))
              {
                 lglw->mouse.touch.b_syn_rmb = (0u != (lglw->keyboard.kmod_state & LGLW_KMOD_CTRL));
                 loc_handle_mousebutton(lglw, LGLW_TRUE/*bPressed*/,
                                        lglw->mouse.touch.b_syn_rmb ? LGLW_MOUSE_RBUTTON : LGLW_MOUSE_LBUTTON
                                        );
                 if(lglw->mouse.touch.b_syn_rmb)
                 {
                    lglw->mouse.touch.syn_rmb_hold_state = LGLW_MOUSE_TOUCH_RMB_STATE_IDLE;
                 }
                 else
                 {
                    lglw->mouse.touch.syn_rmb_hold_state = LGLW_MOUSE_TOUCH_RMB_STATE_LMB;
                    lglw->mouse.touch.hold_start_ms = GetTickCount();
                    lglw->mouse.touch.hold_start_p = lglw->mouse.p;
                 }
              }
              else if(IS_POINTER_SECONDBUTTON_WPARAM(wParam))
              {
                 loc_handle_mousebutton(lglw, LGLW_TRUE/*bPressed*/, LGLW_MOUSE_RBUTTON);
              }
              else if(IS_POINTER_THIRDBUTTON_WPARAM(wParam))
              {
                 loc_handle_mousebutton(lglw, LGLW_TRUE/*bPressed*/, LGLW_MOUSE_MBUTTON);
              }
           }
           break;

           case WM_POINTERUP:
           {
              POINT p;

              // Dprintf("xxx lglw: WM_POINTERUP\n");

              lglw->mouse.touch.b_enable = LGLW_TRUE;
              lglw->mouse.touch.b_update_queued = LGLW_FALSE;

              p.x = GET_X_LPARAM(lParam);
              p.y = GET_Y_LPARAM(lParam);
              (void)ScreenToClient(lglw->win.hwnd, &p);
              lglw->mouse.p.x = p.x;
              lglw->mouse.p.y = p.y;
              loc_handle_mousemotion(lglw);

              // Dprintf("xxx lglw: WM_POINTERUP: IS_POINTER_FIRSTBUTTON_WPARAM(wParam)=%d\n", IS_POINTER_FIRSTBUTTON_WPARAM(wParam));
              // Dprintf("xxx lglw: WM_POINTERUP: IS_POINTER_SECONDBUTTON_WPARAM(wParam)=%d\n", IS_POINTER_SECONDBUTTON_WPARAM(wParam));
              // Dprintf("xxx lglw: WM_POINTERUP: IS_POINTER_THIRDBUTTON_WPARAM(wParam)=%d\n", IS_POINTER_THIRDBUTTON_WPARAM(wParam));

              if( (0u != (lglw->mouse.button_state & LGLW_MOUSE_LBUTTON)) && !IS_POINTER_FIRSTBUTTON_WPARAM(wParam) )
              {
                 if(LGLW_MOUSE_TOUCH_RMB_STATE_RMB == lglw->mouse.touch.syn_rmb_hold_state)
                 {
                    loc_handle_mousebutton(lglw, LGLW_FALSE/*bPressed*/, LGLW_MOUSE_RBUTTON);
                 }
                 else
                 {
                    loc_handle_mousebutton(lglw, LGLW_FALSE/*bPressed*/,
                                           lglw->mouse.touch.b_syn_rmb ? LGLW_MOUSE_RBUTTON : LGLW_MOUSE_LBUTTON
                                           );
                 }
                 lglw->mouse.touch.b_syn_rmb = LGLW_FALSE;
                 lglw->mouse.touch.syn_rmb_hold_state = LGLW_MOUSE_TOUCH_RMB_STATE_IDLE;
              }
              else if( (0u != (lglw->mouse.button_state & LGLW_MOUSE_RBUTTON)) && !IS_POINTER_SECONDBUTTON_WPARAM(wParam))
              {
                 loc_handle_mousebutton(lglw, LGLW_FALSE/*bPressed*/, LGLW_MOUSE_RBUTTON);
              }
              else if( (0u != (lglw->mouse.button_state & LGLW_MOUSE_MBUTTON)) && !IS_POINTER_THIRDBUTTON_WPARAM(wParam))
              {
                 loc_handle_mousebutton(lglw, LGLW_FALSE/*bPressed*/, LGLW_MOUSE_MBUTTON);
              }
           }
           break;

           case WM_POINTERENTER:
           {
              POINT p;

              Dprintf("xxx lglw: WM_POINTERENTER\n");

              lglw->mouse.touch.b_enable = LGLW_TRUE;
              lglw->mouse.touch.b_update_queued = LGLW_FALSE;

               if(0u == (lglw->focus.state & LGLW_FOCUS_MOUSE))
               {
                  loc_handle_mouseenter(lglw);
               }

              p.x = GET_X_LPARAM(lParam);
              p.y = GET_Y_LPARAM(lParam);
              (void)ScreenToClient(lglw->win.hwnd, &p);
              lglw->mouse.p.x = p.x;
              lglw->mouse.p.y = p.y;
              loc_handle_mousemotion(lglw);
              loc_handle_mouseenter(lglw);
           }
           break;

           case WM_POINTERLEAVE:
           {
              POINT p;

              Dprintf("xxx lglw: WM_POINTERLEAVE\n");

              lglw->mouse.touch.b_enable = LGLW_TRUE;
              lglw->mouse.touch.b_update_queued = LGLW_FALSE;

              p.x = GET_X_LPARAM(lParam);
              p.y = GET_Y_LPARAM(lParam);
              (void)ScreenToClient(lglw->win.hwnd, &p);
              loc_handle_mouseleave(lglw);
           }
           break;

           case WM_POINTERUPDATE:
           {
              POINT p;

              lglw->mouse.touch.b_enable = LGLW_TRUE;
              lglw->mouse.touch.b_update_queued = LGLW_FALSE;

              p.x = GET_X_LPARAM(lParam);
              p.y = GET_Y_LPARAM(lParam);
              (void)ScreenToClient(lglw->win.hwnd, &p);
              lglw->mouse.p.x = p.x;
              lglw->mouse.p.y = p.y;
              loc_handle_mousemotion(lglw);

              if(LGLW_MOUSE_TOUCH_RMB_STATE_LMB  == lglw->mouse.touch.syn_rmb_hold_state)
              {
                 if( (sABS(p.x - lglw->mouse.touch.hold_start_p.x) >= LGLW_MOUSE_TOUCH_RMB_MOVE_THRESHOLD) ||
                     (sABS(p.y - lglw->mouse.touch.hold_start_p.y) >= LGLW_MOUSE_TOUCH_RMB_MOVE_THRESHOLD)
                     )
                 {
                    lglw->mouse.touch.syn_rmb_hold_state = LGLW_MOUSE_TOUCH_RMB_STATE_IDLE;
                 }
              }

           }
           break;
#endif // BUILD_64

      } // switch message
   } // if lglw

   return DefWindowProc(hWnd, message, wParam, lParam);
}

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
 * ----
 * ----
 */

#include "lglw.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define Dprintf if(0);else printf
// #define Dprintf if(1);else printf

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
   void *user_data;  // arbitrary user data

   struct {
      lglw_vec2i_t size;
   } hidden;

   struct {
      lglw_vec2i_t size;
      int32_t      swap_interval;
   } win;

   struct {
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

static void loc_key_hook(lglw_int_t *lglw);
static void loc_key_unhook(lglw_int_t *lglw);
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

   // (todo) implement me

   lglw->hidden.size.x = _w;
   lglw->hidden.size.y = _h;

   return LGLW_FALSE;
}


// ---------------------------------------------------------------------------- loc_destroy_hidden_window
static void loc_destroy_hidden_window(lglw_int_t *lglw) {
   // (todo) implement me
}


// ---------------------------------------------------------------------------- lglw_window_open
lglw_bool_t lglw_window_open (lglw_t _lglw, void *_parentHWNDOrNull, int32_t _x, int32_t _y, int32_t _w, int32_t _h) {
   lglw_bool_t r = LGLW_FALSE;
   LGLW(_lglw);

   // (todo) implement me

   if(NULL != lglw)
   {
      if(_w <= 16)
         _w = lglw->hidden.size.x;

      if(_h <= 16)
         _h = lglw->hidden.size.y;

      lglw->win.size.x = _w;
      lglw->win.size.y = _h;

      loc_enable_dropfiles(lglw, (NULL != lglw->dropfiles.cbk));
   }
   return r;
}


// ---------------------------------------------------------------------------- lglw_window_resize
lglw_bool_t lglw_window_resize (lglw_t _lglw, int32_t _w, int32_t _h) {
   lglw_bool_t r = LGLW_FALSE;
   LGLW(_lglw);

   // (todo) implement me

   if(NULL != lglw)
   {
   }

   return r;
}


// ---------------------------------------------------------------------------- lglw_window_close
void lglw_window_close (lglw_t _lglw) {
   LGLW(_lglw);

   // (todo) implement me

   if(NULL != lglw)
   {
      // if(NULL != lglw->win.hwnd)
      {
         lglw_timer_stop(_lglw);

         loc_key_unhook(lglw);
      }
   }
}


// ---------------------------------------------------------------------------- lglw_window_show
void lglw_window_show(lglw_t _lglw) {
   LGLW(_lglw);

   // (todo) implement me

   if(NULL != lglw)
   {
   }
}


// ---------------------------------------------------------------------------- lglw_window_hide
void lglw_window_hide(lglw_t _lglw) {
   LGLW(_lglw);

   // (todo) implement me

   if(NULL != lglw)
   {
   }
}


// ---------------------------------------------------------------------------- lglw_window_is_visible
lglw_bool_t lglw_window_is_visible(lglw_t _lglw) {
   lglw_bool_t r = LGLW_FALSE;
   LGLW(_lglw);

   // (todo) implement me

   if(NULL != lglw)
   {
   }

   return r;
}


// ---------------------------------------------------------------------------- lglw_window_size_get
void lglw_window_size_get(lglw_t _lglw, int32_t *_retX, int32_t *_retY) {
   LGLW(_lglw);

   // (todo) implement me

   if(NULL != lglw)
   {
   }
}


// ---------------------------------------------------------------------------- lglw_redraw
void lglw_redraw(lglw_t _lglw) {
   LGLW(_lglw);

   // (todo) implement me

   if(NULL != lglw)
   {
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
   
   // (todo) implement me

   if(NULL != lglw)
   {
   }
}


// ---------------------------------------------------------------------------- lglw_glcontext_pop
void lglw_glcontext_pop(lglw_t _lglw) {
   LGLW(_lglw);
   
   // (todo) implement me

   if(NULL != lglw)
   {
   }
}


// ---------------------------------------------------------------------------- lglw_swap_buffers
void lglw_swap_buffers(lglw_t _lglw) {
   LGLW(_lglw);

   // (todo) implement me

   if(NULL != lglw)
   {
   }
}


// ---------------------------------------------------------------------------- lglw_swap_interval_set
void lglw_swap_interval_set(lglw_t _lglw, int32_t _ival) {
   LGLW(_lglw);

   // (todo) implement me

   if(NULL != lglw)
   {
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

   // (todo) implement me

   khook_lglw = lglw;
}


// ---------------------------------------------------------------------------- loc_key_unhook
static void loc_key_unhook(lglw_int_t *lglw) {

   // (todo) implement me
   if(khook_lglw == lglw)
      khook_lglw = NULL;
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
      // (todo) implement me
      // if(NULL != lglw->win.hwnd)
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
                  // (todo) implement me
                  // (void)SetCapture(lglw->win.hwnd);
                  lglw->mouse.grab.mode = _grabMode;
                  break;

               case LGLW_MOUSE_GRAB_WARP:
                  // (todo) implement me
                  // (void)SetCapture(lglw->win.hwnd);
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
      // (todo) implement me
      // if(NULL != lglw->win.hwnd)
      {
         if(!lglw->mouse.touch.b_enable)
         {
            switch(lglw->mouse.grab.mode)
            {
               default:
               case LGLW_MOUSE_GRAB_NONE:
                  break;

               case LGLW_MOUSE_GRAB_CAPTURE:
                  // (todo) implement me
                  // (void)ReleaseCapture();
                  lglw->mouse.grab.mode = LGLW_MOUSE_GRAB_NONE;
                  break;

               case LGLW_MOUSE_GRAB_WARP:
                  // (todo) implement me
                  // (void)ReleaseCapture();
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

   // (todo) implement me

   if(NULL != lglw)
   {
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

   // (todo) implement me

   if(NULL != lglw)
   {
   }
}


// ---------------------------------------------------------------------------- lglw_timer_start
void lglw_timer_start(lglw_t _lglw, uint32_t _millisec) {
   LGLW(_lglw);

   // (todo) implement me

   if(NULL != lglw)
   {
   }
}


// ---------------------------------------------------------------------------- lglw_timer_stop
void lglw_timer_stop(lglw_t _lglw) {
   LGLW(_lglw);

   // (todo) implement me

   if(NULL != lglw)
   {
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


// ---------------------------------------------------------------------------- loc_touchinput_update
static void loc_touchinput_update(lglw_int_t *lglw) {

   // (todo) implement me
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

   // (todo) implement me

   if(NULL != _text)
   {
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
         // (todo) implement me
      }
   }
}

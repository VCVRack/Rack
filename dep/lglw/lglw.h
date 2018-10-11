/* ----
 * ---- file   : lglw.h
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
 * ---- changed: 05Aug2018, 06Aug2018, 07Aug2018, 08Aug2018, 18Aug2018, 05Sep2018
 * ----
 * ----
 */

#ifndef __LGLW_H__
#define __LGLW_H__

#include "cplusplus_begin.h"

#include <stdint.h>


// Opaque library / instance handle
typedef void *lglw_t;

// Boolean type
#define LGLW_TRUE  (1)
#define LGLW_FALSE (0)
typedef int32_t lglw_bool_t;

// Vector type
typedef struct lglw_vec2i_s {
   int32_t x;
   int32_t y;
} lglw_vec2i_t;

// Mouse buttons
#define LGLW_MOUSE_LBUTTON    (1u << 0)
#define LGLW_MOUSE_RBUTTON    (1u << 1)
#define LGLW_MOUSE_MBUTTON    (1u << 2)
#define LGLW_MOUSE_WHEELUP    (1u << 3)
#define LGLW_MOUSE_WHEELDOWN  (1u << 4)

// Mouse button utility macros
#define LGLW_IS_MOUSE_BUTTON_DOWN(a)  ((a) == (_buttonState & _changedButtonState))
#define LGLW_IS_MOUSE_BUTTON_UP(a)    ( (0u != (_changedButtonState & (a))) && (0u == (_buttonState & (a))) )

#define LGLW_IS_MOUSE_LBUTTON_DOWN()  LGLW_IS_MOUSE_BUTTON_DOWN(LGLW_MOUSE_LBUTTON)
#define LGLW_IS_MOUSE_RBUTTON_DOWN()  LGLW_IS_MOUSE_BUTTON_DOWN(LGLW_MOUSE_RBUTTON)
#define LGLW_IS_MOUSE_MBUTTON_DOWN()  LGLW_IS_MOUSE_BUTTON_DOWN(LGLW_MOUSE_MBUTTON)
#define LGLW_IS_MOUSE_WHEELUP()       LGLW_IS_MOUSE_BUTTON_DOWN(LGLW_MOUSE_WHEELUP)
#define LGLW_IS_MOUSE_WHEELDOWN()     LGLW_IS_MOUSE_BUTTON_DOWN(LGLW_MOUSE_WHEELDOWN)

#define LGLW_IS_MOUSE_LBUTTON_UP()  LGLW_IS_MOUSE_BUTTON_UP(LGLW_MOUSE_LBUTTON)
#define LGLW_IS_MOUSE_RBUTTON_UP()  LGLW_IS_MOUSE_BUTTON_UP(LGLW_MOUSE_RBUTTON)
#define LGLW_IS_MOUSE_MBUTTON_UP()  LGLW_IS_MOUSE_BUTTON_UP(LGLW_MOUSE_MBUTTON)

// Mouse grab modes
#define LGLW_MOUSE_GRAB_NONE     (0u)   // no mouse grab
#define LGLW_MOUSE_GRAB_CAPTURE  (1u)   // report mouse positions beyond window boundaries (+ show mouse pointer)
#define LGLW_MOUSE_GRAB_WARP     (2u)   // capture + invisible mouse pointer + warp back to original position when ungrabbed

// Focus types
#define LGLW_FOCUS_MOUSE  (1u << 0)

// Focus utility macros
#define LGLW_FOCUS_MOUSE_GAINED  (LGLW_FOCUS_MOUSE == (_focusState & _changedFocusState))
#define LGLW_FOCUS_MOUSE_LOST    ( (0u != (_changedFocusState & LGLW_FOCUS_MOUSE)) && (0u == (_focusState & LGLW_FOCUS_MOUSE)) )

// Modifier keys
#define LGLW_KMOD_LCTRL   (1u << 0)
#define LGLW_KMOD_LSHIFT  (1u << 1)
#define LGLW_KMOD_RCTRL   (1u << 2)
#define LGLW_KMOD_RSHIFT  (1u << 3)

#define LGLW_KMOD_CTRL    (LGLW_KMOD_LCTRL | LGLW_KMOD_RCTRL)
#define LGLW_KMOD_SHIFT   (LGLW_KMOD_LSHIFT | LGLW_KMOD_RSHIFT)

// Special (non-unicode) keys
#define LGLW_VKEY_EXT        (0x10000000u)
#define LGLW_VKEY_BACKSPACE  (LGLW_VKEY_EXT | 0x08u)
#define LGLW_VKEY_TAB        (LGLW_VKEY_EXT | 0x09u)
#define LGLW_VKEY_RETURN     (LGLW_VKEY_EXT | 0x0Du)
#define LGLW_VKEY_ESCAPE     (LGLW_VKEY_EXT | 0x1Bu)
#define LGLW_VKEY_PAGEUP     (LGLW_VKEY_EXT | 0x21u)
#define LGLW_VKEY_PAGEDOWN   (LGLW_VKEY_EXT | 0x22u)
#define LGLW_VKEY_END        (LGLW_VKEY_EXT | 0x23u)
#define LGLW_VKEY_HOME       (LGLW_VKEY_EXT | 0x24u)
#define LGLW_VKEY_LEFT       (LGLW_VKEY_EXT | 0x25u)
#define LGLW_VKEY_UP         (LGLW_VKEY_EXT | 0x26u)
#define LGLW_VKEY_RIGHT      (LGLW_VKEY_EXT | 0x27u)
#define LGLW_VKEY_DOWN       (LGLW_VKEY_EXT | 0x28u)
#define LGLW_VKEY_INSERT     (LGLW_VKEY_EXT | 0x2Du)
#define LGLW_VKEY_DELETE     (LGLW_VKEY_EXT | 0x2Eu)
#define LGLW_VKEY_F1         (LGLW_VKEY_EXT | 0x70u)
#define LGLW_VKEY_F2         (LGLW_VKEY_EXT | 0x71u)
#define LGLW_VKEY_F3         (LGLW_VKEY_EXT | 0x72u)
#define LGLW_VKEY_F4         (LGLW_VKEY_EXT | 0x73u)
#define LGLW_VKEY_F5         (LGLW_VKEY_EXT | 0x74u)
#define LGLW_VKEY_F6         (LGLW_VKEY_EXT | 0x75u)
#define LGLW_VKEY_F7         (LGLW_VKEY_EXT | 0x76u)
#define LGLW_VKEY_F8         (LGLW_VKEY_EXT | 0x77u)
#define LGLW_VKEY_F9         (LGLW_VKEY_EXT | 0x78u)
#define LGLW_VKEY_F10        (LGLW_VKEY_EXT | 0x79u)
#define LGLW_VKEY_F11        (LGLW_VKEY_EXT | 0x7Au)
#define LGLW_VKEY_F12        (LGLW_VKEY_EXT | 0x7Bu)
#define LGLW_VKEY_NUMLOCK    (LGLW_VKEY_EXT | 0x90u)
#define LGLW_VKEY_SCROLLLOCK (LGLW_VKEY_EXT | 0x91u)
#define LGLW_VKEY_LSHIFT     (LGLW_VKEY_EXT | 0xa0u)
#define LGLW_VKEY_RSHIFT     (LGLW_VKEY_EXT | 0xa1u)
#define LGLW_VKEY_LCTRL      (LGLW_VKEY_EXT | 0xa2u)
#define LGLW_VKEY_RCTRL      (LGLW_VKEY_EXT | 0xa3u)

// Keyboard utility macros
#define LGLW_IS_CHAR_KEY(a)     (0u == ((a) & LGLW_VKEY_EXT))
#define LGLW_IS_SPECIAL_KEY(a)  (0u != ((a) & LGLW_VKEY_EXT))

// Mouse callback function type
typedef void (*lglw_mouse_fxn_t) (lglw_t _lglw, int32_t _x, int32_t _y, uint32_t _buttonState, uint32_t _changedButtonState);

// Focus callback function type
typedef void (*lglw_focus_fxn_t) (lglw_t _lglw, uint32_t _focusState, uint32_t _changedFocusState);

// Keyboard callback function type
//  (note) 'vkey' is either a special key (cursor, insert, ..) or a unicode-translated character
//  Must return LGLW_TRUE when event has been processed, LGLW_FALSE when event shall be forwarded to next window / listener
typedef lglw_bool_t (*lglw_keyboard_fxn_t) (lglw_t _lglw, uint32_t _vkey, uint32_t _kmod, lglw_bool_t _bPressed);

// Timer callback function type
typedef void (*lglw_timer_fxn_t) (lglw_t _lglw);

// File drag'n'drop callback function type
typedef void (*lglw_dropfiles_fxn_t) (lglw_t _lglw, int32_t _x, int32_t _y, uint32_t _numFiles, const char**_pathNames);

// Redraw function type
typedef void (*lglw_redraw_fxn_t) (lglw_t _lglw);

// Initialize LGLW instance
//  (note) (w; h) determine the hidden window size, which should match the size of the actual window that is created later on
//  (note) when w or h is less than 16, a default width/height is used instead
lglw_t lglw_init (int32_t _w, int32_t _h);

// Shutdown LGLW instance
void lglw_exit (lglw_t _lglw);

// Set LGLW instance userdata
void lglw_userdata_set (lglw_t _lglw, void *_userData);

// Get LGLW instance userdata
void *lglw_userdata_get (lglw_t _lglw);

// Open LGLW window
//   Return: 1=window open, 0=error
//   (note) when w or h is less than 16, the hidden window size is used instead (see lglw_init())
lglw_bool_t lglw_window_open (lglw_t _lglw, void *_parentHWNDOrNull, int32_t _x, int32_t _y, int32_t _w, int32_t _h);

// Resize previously opened LGLW window
lglw_bool_t lglw_window_resize (lglw_t _lglw, int32_t _w, int32_t _h);

// Close LGLW window
void lglw_window_close (lglw_t _lglw);

// Show window
void lglw_window_show (lglw_t _lglw);

// Hide window
void lglw_window_hide (lglw_t _lglw);

// Check if window is visible
lglw_bool_t lglw_window_is_visible (lglw_t _lglw);

// Get window size
void lglw_window_size_get (lglw_t _lglw, int32_t *_retX, int32_t *_retY);

// Redraw window (send message)
void lglw_redraw (lglw_t _lglw);

// Set redraw callback
void lglw_redraw_callback_set (lglw_t _lglw, lglw_redraw_fxn_t _cbk);

// Save previous GL context and bind LGLW context
void lglw_glcontext_push (lglw_t _lglw);

// Unbind LGLW context and restore previous GL context
void lglw_glcontext_pop (lglw_t _lglw);

// Swap front- and backbuffers
void lglw_swap_buffers (lglw_t _lglw);

// Set swap interval (0=vsync off, 1=vsync on)
void lglw_swap_interval_set (lglw_t _lglw, int32_t _ival);

// Get swap interval (0=vsync off, 1=vsync on)
int32_t lglw_swap_interval_get (lglw_t _lglw);

// Install mouse callback
void lglw_mouse_callback_set (lglw_t _lglw, lglw_mouse_fxn_t _cbk);

// Install focus callback
void lglw_focus_callback_set (lglw_t _lglw, lglw_focus_fxn_t _cbk);

// Install keyboard callback
void lglw_keyboard_callback_set (lglw_t _lglw, lglw_keyboard_fxn_t _cbk);

// Get current key modifier state
uint32_t lglw_keyboard_get_modifiers (lglw_t _lglw);

// Get current mouse button state
uint32_t lglw_mouse_get_buttons (lglw_t _lglw);

// Grab (lock) mouse pointer
//   (note) see LGLW_MOUSE_GRAB_xxx
void lglw_mouse_grab (lglw_t _lglw, uint32_t _grabMode);

// Ungrab (unlock) mouse pointer
void lglw_mouse_ungrab (lglw_t _lglw);

// Set mouse pointer position
void lglw_mouse_warp (lglw_t _lglw, int32_t _x, int32_t _y);

// Show / hide mouse pointer
void lglw_mouse_cursor_show (lglw_t _lglw, lglw_bool_t _bShow);

// Start periodic timer
//  (note) requires an output window (see lglw_window_open())
void lglw_timer_start (lglw_t _lglw, uint32_t _millisec);

// Stop periodic timer
void lglw_timer_stop (lglw_t _lglw);

// Set periodic timer callback
void lglw_timer_callback_set (lglw_t _lglw, lglw_timer_fxn_t _cbk);

// Set file drag'n'drop callback
void lglw_dropfiles_callback_set (lglw_t _lglw, lglw_dropfiles_fxn_t _cbk);

// Enable / disable Windows 8+ touch API
void lglw_touchinput_set (lglw_t _lglw, lglw_bool_t _bEnable);

// Check if touch input is enabled
lglw_bool_t lglw_touchinput_get (lglw_t _lglw);

// Show / hide virtual keyboard
void lglw_touchkeyboard_show (lglw_t _lglw, lglw_bool_t _bEnable);

// Set clipboard string
//  (note) numChars==0: copy until (including) ASCIIz
void lglw_clipboard_text_set (lglw_t _lglw, const uint32_t _numChars, const char *_text);

// Get clipboard string
void lglw_clipboard_text_get (lglw_t _lglw, uint32_t _maxChars, uint32_t *_retNumChars, char *_retText);

#include "cplusplus_end.h"

#endif // __LGLW_H__

#include "global_pre.hpp"
#include "window.hpp"
#include "app.hpp"
#include "asset.hpp"
#include "settings.hpp"
#include "gamepad.hpp"
#include "keyboard.hpp"
#include "util/color.hpp"

#include <map>
#include <queue>
#include <thread>

#include "osdialog.h"

#define NANOVG_GL2_IMPLEMENTATION 1
// #define NANOVG_GL3_IMPLEMENTATION 1
// #define NANOVG_GLES2_IMPLEMENTATION 1
// #define NANOVG_GLES3_IMPLEMENTATION 1
#include "nanovg_gl.h"
// Hack to get framebuffer objects working on OpenGL 2 (we blindly assume the extension is supported)
#define NANOVG_FBO_VALID 1
#include "nanovg_gl_utils.h"
#define BLENDISH_IMPLEMENTATION
#include "blendish.h"
#define NANOSVG_IMPLEMENTATION
#define NANOSVG_ALL_COLOR_KEYWORDS
#include "nanosvg.h"

#ifdef ARCH_MAC
	// For CGAssociateMouseAndMouseCursorPosition
	#include <ApplicationServices/ApplicationServices.h>
#endif

#include "global.hpp"
#include "global_ui.hpp"


#ifndef RACK_PLUGIN_SHARED_LIB_BUILD
extern void vst2_set_globals (void *_wrapper);
extern "C" { extern void lglw_timer_cbk (lglw_t _lglw); }   // implemented in vst2_main.cpp
extern "C" { extern void lglw_redraw_cbk (lglw_t _lglw); }  // implemented in vst2_main.cpp
#else
void vst2_set_globals(void *) { }
void vst2_window_size_set(int, int) { }
void vst2_refresh_rate_set(float) { }
void lglw_timer_cbk(lglw_t) { }
void lglw_redraw_cbk(lglw_t) { }
#endif // RACK_PLUGIN_SHARED_LIB_BUILD

namespace rack {


extern "C" {

static void lglw_mouse_cbk(lglw_t _lglw, int32_t _x, int32_t _y, uint32_t _buttonState, uint32_t _changedButtonState) {
   // printf("xxx lglw_mouse_cbk: lglw=%p wrapper=%p p=(%d; %d) bt=0x%08x changedBt=0x%08x\n", _lglw, lglw_userdata_get(_lglw), _x, _y, _buttonState, _changedButtonState);
   vst2_set_globals(lglw_userdata_get(_lglw));
   // printf("xxx   lglw_mouse_cbk: &global=%p global=%p\n", &rack::global, rack::global);

   // (note) assumes that GL context is never touched during regular mouse move
   // (note) mouse clicks may cause new SVGs to be loaded, which in turn may cause new GL textures to be created
   // if(0u != _changedButtonState)
      lglw_glcontext_push(global_ui->window.lglw);

   if(LGLW_MOUSE_WHEELUP == _buttonState)
   {
      // onScroll
      EventScroll e;
      e.pos = global_ui->window.gMousePos;
      Vec scrollRel = Vec(0, 1);
      e.scrollRel = scrollRel.mult(50.0);
      global_ui->ui.gScene->onScroll(e);
   }
   else if(LGLW_MOUSE_WHEELDOWN == _buttonState)
   {
      // onScroll
      EventScroll e;
      e.pos = global_ui->window.gMousePos;
      Vec scrollRel = Vec(0, -1);
      e.scrollRel = scrollRel.mult(50.0);
      global_ui->ui.gScene->onScroll(e);
   }
   else if(0u == _changedButtonState)
   {
      // onMouseMotion
      Vec mousePos = Vec(_x, _y);//.div(global_ui->window.gPixelRatio / global_ui->window.gWindowRatio).round();
      Vec mouseRel = mousePos.minus(global_ui->window.gMousePos);

// #ifdef ARCH_MAC
//       // Workaround for Mac. We can't use GLFW_CURSOR_DISABLED because it's buggy, so implement it on our own.
//       // This is not an ideal implementation. For example, if the user drags off the screen, the new mouse position will be clamped.
//       if (cursorMode == GLFW_CURSOR_HIDDEN) {
//          // CGSetLocalEventsSuppressionInterval(0.0);
//          glfwSetCursorPos(global_ui->window.gWindow, global_ui->window.gMousePos.x, global_ui->window.gMousePos.y);
//          CGAssociateMouseAndMouseCursorPosition(true);
//          mousePos = global_ui->window.gMousePos;
//       }
//       // Because sometimes the cursor turns into an arrow when its position is on the boundary of the window
//       glfwSetCursor(global_ui->window.gWindow, NULL);
// #endif

      global_ui->window.gMousePos = mousePos;

      global_ui->widgets.gTempWidget = NULL;

      // onMouseMove
      {
         EventMouseMove e;
         e.pos = mousePos;
         e.mouseRel = mouseRel;
         global_ui->ui.gScene->onMouseMove(e);
         global_ui->widgets.gTempWidget = e.target;
      }

      if (global_ui->widgets.gDraggedWidget) {
         // onDragMove
         EventDragMove e;
         e.mouseRel = mouseRel;
         global_ui->widgets.gDraggedWidget->onDragMove(e);

         if (global_ui->widgets.gTempWidget != global_ui->widgets.gDragHoveredWidget) {
            if (global_ui->widgets.gDragHoveredWidget) {
               EventDragEnter e;
               e.origin = global_ui->widgets.gDraggedWidget;
               global_ui->widgets.gDragHoveredWidget->onDragLeave(e);
            }
            global_ui->widgets.gDragHoveredWidget = global_ui->widgets.gTempWidget;
            if (global_ui->widgets.gDragHoveredWidget) {
               EventDragEnter e;
               e.origin = global_ui->widgets.gDraggedWidget;
               global_ui->widgets.gDragHoveredWidget->onDragEnter(e);
            }
         }
      }
      else {
         if (global_ui->widgets.gTempWidget != global_ui->widgets.gHoveredWidget) {
            if (global_ui->widgets.gHoveredWidget) {
               // onMouseLeave
               EventMouseLeave e;
               global_ui->widgets.gHoveredWidget->onMouseLeave(e);
            }
            global_ui->widgets.gHoveredWidget = global_ui->widgets.gTempWidget;
            if (global_ui->widgets.gHoveredWidget) {
               // onMouseEnter
               EventMouseEnter e;
               global_ui->widgets.gHoveredWidget->onMouseEnter(e);
            }
         }
      }
      global_ui->widgets.gTempWidget = NULL;

      if(0u != (_buttonState & LGLW_MOUSE_MBUTTON)) {
         // TODO
         // Define a new global called gScrollWidget, which remembers the widget where middle-click was first pressed
         EventScroll e;
         e.pos = mousePos;
         e.scrollRel = mouseRel;
         global_ui->ui.gScene->onScroll(e);
      }
   }
   else
   {
      // Mouse button state changed

// #ifdef ARCH_MAC
//       // Ctrl-left click --> right click
//       if (button == GLFW_MOUSE_BUTTON_LEFT) {
//          if (glfwGetKey(global_ui->window.gWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(global_ui->window.gWindow, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
//             button = GLFW_MOUSE_BUTTON_RIGHT;
//          }
//       }
// #endif
      int button =
         (0u != (_changedButtonState & LGLW_MOUSE_LBUTTON)) ? RACK_MOUSE_BUTTON_LEFT/*0*/ :
         (0u != (_changedButtonState & LGLW_MOUSE_RBUTTON)) ? RACK_MOUSE_BUTTON_RIGHT/*1*/ :
         (0u != (_changedButtonState & LGLW_MOUSE_MBUTTON)) ? RACK_MOUSE_BUTTON_MIDDLE/*2*/ :
         -1;

      bool bPressed = (0u != (_changedButtonState & _buttonState));

      if (bPressed) {
         global_ui->widgets.gTempWidget = NULL;
         // onMouseDown
         {
            EventMouseDown e;
            e.pos = global_ui->window.gMousePos;
            e.button = button;
            global_ui->ui.gScene->onMouseDown(e);
            global_ui->widgets.gTempWidget = e.target;
         }

         if (RACK_MOUSE_BUTTON_LEFT/*0*/ == button) {
            if (global_ui->widgets.gTempWidget) {
               // onDragStart
               EventDragStart e;
               global_ui->widgets.gTempWidget->onDragStart(e);
            }
            global_ui->widgets.gDraggedWidget = global_ui->widgets.gTempWidget;

            if (global_ui->widgets.gTempWidget != global_ui->widgets.gFocusedWidget) {
               if (global_ui->widgets.gFocusedWidget) {
                  // onDefocus
                  EventDefocus e;
                  global_ui->widgets.gFocusedWidget->onDefocus(e);
               }
               global_ui->widgets.gFocusedWidget = NULL;
               if (global_ui->widgets.gTempWidget) {
                  // onFocus
                  EventFocus e;
                  global_ui->widgets.gTempWidget->onFocus(e);
                  if (e.consumed) {
                     global_ui->widgets.gFocusedWidget = global_ui->widgets.gTempWidget;
                  }
               }
            }
         }
         global_ui->widgets.gTempWidget = NULL;
      }
      else {
         // onMouseUp
         global_ui->widgets.gTempWidget = NULL;
         {
            EventMouseUp e;
            e.pos = global_ui->window.gMousePos;
            e.button = button;
            global_ui->ui.gScene->onMouseUp(e);
            global_ui->widgets.gTempWidget = e.target;
         }

         if (RACK_MOUSE_BUTTON_LEFT/*0*/ == button) {
            if (global_ui->widgets.gDraggedWidget) {
               // onDragDrop
               EventDragDrop e;
               e.origin = global_ui->widgets.gDraggedWidget;
               global_ui->widgets.gTempWidget->onDragDrop(e);
            }
            // gDraggedWidget might have been set to null in the last event, recheck here
            if (global_ui->widgets.gDraggedWidget) {
               // onDragEnd
               EventDragEnd e;
               global_ui->widgets.gDraggedWidget->onDragEnd(e);
            }
            global_ui->widgets.gDraggedWidget = NULL;
            global_ui->widgets.gDragHoveredWidget = NULL;
         }
         global_ui->widgets.gTempWidget = NULL;
      }
   }

   // if(0u != _changedButtonState)
      lglw_glcontext_pop(global_ui->window.lglw);

   // printf("xxx lglw_mouse_cbk: LEAVE global=%p\n", rack::global);
}

static void lglw_focus_cbk(lglw_t _lglw, uint32_t _focusState, uint32_t _changedFocusState) {
   // printf("xxx lglw_focus_cbk: lglw=%p focusState=0x%08x changedFocusState=0x%08x\n", _lglw, _focusState, _changedFocusState);

   vst2_set_globals(lglw_userdata_get(_lglw));

   if(0u != (_changedFocusState & LGLW_FOCUS_MOUSE))
   {
      if(0u != (_focusState & LGLW_FOCUS_MOUSE))
      {
         // onMouseEnter
      }
      else
      {
         if (global_ui->widgets.gHoveredWidget) {
            // onMouseLeave
            EventMouseLeave e;
            global_ui->widgets.gHoveredWidget->onMouseLeave(e);
         }
         global_ui->widgets.gHoveredWidget = NULL;
      }
   }
}

static lglw_bool_t lglw_keyboard_cbk(lglw_t _lglw, uint32_t _vkey, uint32_t _kmod, lglw_bool_t _bPressed) {
   // printf("xxx lglw_keyboard_cbk: lglw=%p vkey=0x%08x (\'%c\') kmod=0x%08x bPressed=%d\n", _lglw, _vkey, (char)_vkey, _kmod, _bPressed);
   lglw_bool_t bHandled = LGLW_FALSE;

   vst2_set_globals(lglw_userdata_get(_lglw));

   if( (0u == (_vkey & LGLW_VKEY_EXT)) && (0u == (_kmod & (LGLW_KMOD_LCTRL | LGLW_KMOD_RCTRL))) )
   {
      // Unicode or ASCII character
      if(_bPressed)
      {
         if (global_ui->widgets.gFocusedWidget) {
            // onText
            EventText e;
            e.codepoint = _vkey;
            global_ui->widgets.gFocusedWidget->onText(e);
            if(e.consumed)
               return LGLW_TRUE;
         }
      }
   }

   if(_bPressed)
   {
      if (global_ui->widgets.gFocusedWidget) {
         // onKey
         EventKey e;
         e.key = _vkey;
         global_ui->widgets.gFocusedWidget->onKey(e);
         if (e.consumed)
            return LGLW_TRUE;
      }

      // onHoverKey
      EventHoverKey e;
      e.pos = global_ui->window.gMousePos;
      e.key = _vkey;
      global_ui->ui.gScene->onHoverKey(e);
   }

   return bHandled;
}

int vst2_handle_effeditkeydown(unsigned int _vkey) {
   // (note) only used for touch keyboard input
   lglw_bool_t bHandled = lglw_keyboard_cbk(rack::global_ui->window.lglw, _vkey, 0u/*kmod*/, LGLW_TRUE/*bPressed*/);
   lglw_keyboard_cbk(rack::global_ui->window.lglw, _vkey, 0u/*kmod*/, LGLW_FALSE/*bPressed*/);
   return bHandled;
}


void lglw_dropfiles_cbk(lglw_t _lglw, int32_t _x, int32_t _y, uint32_t _numFiles, const char **_pathNames) {
	// onPathDrop
   vst2_set_globals(lglw_userdata_get(_lglw));
   lglw_glcontext_push(global_ui->window.lglw);
	EventPathDrop e;
	e.pos = Vec(_x, _y);
	for(uint32_t i = 0u; i < _numFiles; i++) {
		e.paths.push_back(_pathNames[i]);
	}
	global_ui->ui.gScene->onPathDrop(e);
   lglw_glcontext_pop(global_ui->window.lglw);
}

} // extern C

void renderGui() {
	int width, height;
   // printf("xxx renderGui: ENTER\n");
   lglw_window_size_get(global_ui->window.lglw, &width, &height);

	// Update and render
	nvgBeginFrame(global_ui->window.gVg, width, height, global_ui->window.gPixelRatio);

	nvgReset(global_ui->window.gVg);
	nvgScale(global_ui->window.gVg, global_ui->window.gPixelRatio, global_ui->window.gPixelRatio);
   // printf("xxx renderGui: gScene->draw() BEGIN\n");
	global_ui->ui.gScene->draw(global_ui->window.gVg);
   // printf("xxx renderGui: gScene->draw() END\n");

	glViewport(0, 0, width, height);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	nvgEndFrame(global_ui->window.gVg);
   // printf("xxx renderGui: LEAVE\n");
}

void windowInit() {
	int err;

   printf("xxx vstrack_plugin:windowInit: ENTER\n");

   // (note) the hidden LGLW context window must have the same size as the real window created later on
   settingsLoad(assetLocal("settings.json"), true/*bWindowSizeOnly*/);

   printf("xxx vstrack_plugin:windowInit: 2\n");

   global_ui->window.lglw = lglw_init(global_ui->window.windowWidth, global_ui->window.windowHeight);

   printf("xxx vstrack_plugin:windowInit: 3\n");

   lglw_userdata_set(global_ui->window.lglw, global->vst2.wrapper);

   printf("xxx vstrack_plugin:windowInit: 4\n");

	global_ui->window.lastWindowTitle = "";

   lglw_glcontext_push(global_ui->window.lglw);
   printf("xxx vstrack_plugin:windowInit: 5\n");
   lglw_swap_interval_set(global_ui->window.lglw, 1);  // can be overridden via settings.json:"vsync" property
   printf("xxx vstrack_plugin:windowInit: 6\n");

   lglw_mouse_callback_set     (global_ui->window.lglw, &lglw_mouse_cbk);
   lglw_focus_callback_set     (global_ui->window.lglw, &lglw_focus_cbk);
   lglw_keyboard_callback_set  (global_ui->window.lglw, &lglw_keyboard_cbk);
   lglw_timer_callback_set     (global_ui->window.lglw, &lglw_timer_cbk);
   lglw_dropfiles_callback_set (global_ui->window.lglw, &lglw_dropfiles_cbk);
   lglw_redraw_callback_set    (global_ui->window.lglw, &lglw_redraw_cbk);
   printf("xxx vstrack_plugin:windowInit: 7\n");

	// Set up GLEW
	glewExperimental = GL_TRUE;
	err = glewInit();
	if (err != GLEW_OK) {
		osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, "Could not initialize GLEW. Does your graphics card support OpenGL 2.0 or greater? If so, make sure you have the latest graphics drivers installed.");
      lglw_glcontext_pop(global_ui->window.lglw);
		exit(1);
	}
   printf("xxx vstrack_plugin:windowInit: 8\n");

	// GLEW generates GL error because it calls glGetString(GL_EXTENSIONS), we'll consume it here.
	glGetError();
   printf("xxx vstrack_plugin:windowInit: 9\n");

	// Set up NanoVG
	int nvgFlags = NVG_ANTIALIAS;
#if defined NANOVG_GL2
	global_ui->window.gVg = nvgCreateGL2(nvgFlags);
#elif defined NANOVG_GL3
	global_ui->window.gVg = nvgCreateGL3(nvgFlags);
#elif defined NANOVG_GLES2
	global_ui->window.gVg = nvgCreateGLES2(nvgFlags);
#endif
   printf("xxx vstrack_plugin:windowInit: 10\n");
	assert(global_ui->window.gVg);

#if defined NANOVG_GL2
	global_ui->window.gFramebufferVg = nvgCreateGL2(nvgFlags);
#elif defined NANOVG_GL3
	global_ui->window.gFramebufferVg = nvgCreateGL3(nvgFlags);
#elif defined NANOVG_GLES2
	global_ui->window.gFramebufferVg = nvgCreateGLES2(nvgFlags);
#endif
   printf("xxx vstrack_plugin:windowInit: 11\n");
	assert(global_ui->window.gFramebufferVg);

	// Set up Blendish
   printf("xxx vstrack_plugin:windowInit: 12\n");
	global_ui->window.gGuiFont = Font::load(assetGlobal("res/fonts/DejaVuSans.ttf"));
   printf("xxx vstrack_plugin:windowInit: 13\n");
	bndSetFont(global_ui->window.gGuiFont->handle);
   printf("xxx vstrack_plugin:windowInit: 14\n");

	windowSetTheme(nvgRGB(0x33, 0x33, 0x33), nvgRGB(0xf0, 0xf0, 0xf0));
   printf("xxx vstrack_plugin:windowInit: 15\n");

   lglw_glcontext_pop(global_ui->window.lglw);

   printf("xxx vstrack_plugin:windowInit: LEAVE\n");
}

void windowDestroy() {
   printf("xxx vstrack_plugin: windowDestroy()\n");
   lglw_glcontext_push(global_ui->window.lglw);

	global_ui->window.gGuiFont.reset();

#if defined NANOVG_GL2
	nvgDeleteGL2(global_ui->window.gVg);
#elif defined NANOVG_GL3
	nvgDeleteGL3(global_ui->window.gVg);
#elif defined NANOVG_GLES2
	nvgDeleteGLES2(global_ui->window.gVg);
#endif

#if defined NANOVG_GL2
	nvgDeleteGL2(global_ui->window.gFramebufferVg);
#elif defined NANOVG_GL3
	nvgDeleteGL3(global_ui->window.gFramebufferVg);
#elif defined NANOVG_GLES2
	nvgDeleteGLES2(global_ui->window.gFramebufferVg);
#endif

   lglw_glcontext_pop(global_ui->window.lglw);

   lglw_exit(global_ui->window.lglw);
   global_ui->window.lglw = NULL;
}

void vst2_editor_redraw(void) {
   // (note) the GL context is set by the caller

   global_ui->window.gGuiFrame++;

   // Find/validate hover param
   if(NULL != global_ui->param_info.last_param_widget)
   {
      int uniqueParamId;
      ParamWidget *paramWidget =
         global_ui->app.gRackWidget->findParamWidgetAndUniqueParamIdByWidgetRef(global_ui->param_info.last_param_widget,
                                                                                &uniqueParamId
                                                                                );
      if(NULL != paramWidget)
      {
         global_ui->param_info.last_param_gid = uniqueParamId;
         global_ui->param_info.last_param_value = paramWidget->value;

#if 0
         printf("xxx vst2_editor_redraw: param_info: uniqueParamId=%d value=%f clipboardValue=%f\n",
                global_ui->param_info.last_param_gid,
                global_ui->param_info.last_param_value,
                global_ui->param_info.value_clipboard
                );
#endif

         char buf[64];
         sprintf(buf, "%d", global_ui->param_info.last_param_gid);
         global_ui->param_info.tf_id->setTextQuiet(buf);
         sprintf(buf, "%f", global_ui->param_info.last_param_value);

         // Delete trailing zeros
         {
            char *d = buf;
            while(0 != *d)
               d++;
            d--;
            if(d > buf)
            {
               while('0' == *d)
               {
                  if(((d-1) > buf) && ('.' != d[-1]))
                     *d-- = 0;
                  else
                     break;
               }
            }
         }

         global_ui->param_info.tf_value->setTextQuiet(buf);
      }

      global_ui->param_info.last_param_widget = NULL;
      global_ui->param_info.placeholder_framecount = 1;
   }
   else if(0 != global_ui->param_info.placeholder_framecount)
   {
      if(++global_ui->param_info.placeholder_framecount > (30*30))
      {
         global_ui->param_info.tf_id->setTextQuiet("");
         global_ui->param_info.tf_value->setTextQuiet("");
         global_ui->param_info.placeholder_framecount = 0;
      }
   }
   

#if 0
   // Set window title
   //  (note) the VST plugin editor window title is set by the VST host
   std::string windowTitle;
   windowTitle = global_ui->app.gApplicationName;
   windowTitle += " ";
   windowTitle += global_ui->app.gApplicationVersion;
   if (!global_ui->app.gRackWidget->lastPath.empty()) {
      windowTitle += " - ";
      windowTitle += stringFilename(global_ui->app.gRackWidget->lastPath);
   }
   if (windowTitle != global_ui->window.lastWindowTitle) {
      // // glfwSetWindowTitle(global_ui->window.gWindow, windowTitle.c_str());
      global_ui->window.lastWindowTitle = windowTitle;
   }
#endif

   // Get framebuffer/window ratio
   int width, height;
   lglw_window_size_get(global_ui->window.lglw, &width, &height);
   global_ui->window.gWindowRatio = 1.0f;
   global_ui->ui.gScene->box.size = Vec(width, height);

   // Step scene
   global_ui->ui.gScene->step();

   // Render
   renderGui();

   // Present
   glFlush();
   lglw_swap_buffers(global_ui->window.lglw);
}

void windowCursorLock() {
 	if (global_ui->window.gAllowCursorLock) {
      lglw_mouse_grab(global_ui->window.lglw, LGLW_MOUSE_GRAB_WARP);
 	}
}

void windowCursorUnlock() {
   if (global_ui->window.gAllowCursorLock) {
      lglw_mouse_ungrab(global_ui->window.lglw);
   }
}

bool windowIsModPressed() {
   return (0u != (lglw_keyboard_get_modifiers(global_ui->window.lglw) & (LGLW_KMOD_LCTRL | LGLW_KMOD_RCTRL))); 
}

bool windowIsShiftPressed() {
   return (0u != (lglw_keyboard_get_modifiers(global_ui->window.lglw) & (LGLW_KMOD_LSHIFT | LGLW_KMOD_RSHIFT))); 
}

Vec windowGetWindowSize() {
	int width, height;
   lglw_window_size_get(global_ui->window.lglw, &width, &height);
	return Vec(width, height);
}

void windowSetWindowSize(Vec size) {
   (void)size;
   // (note) not supported
}

Vec windowGetWindowPos() {
	int x, y;
   x = 0;
   y = 0;
	return Vec(x, y);
}

void windowSetWindowPos(Vec pos) {
	int x = pos.x;
	int y = pos.y;
   // (note) not supported
}

bool windowIsMaximized() {
   return true;
}

void windowSetTheme(NVGcolor bg, NVGcolor fg) {
	// Assume dark background and light foreground

	BNDwidgetTheme w;
	w.outlineColor = bg;
	w.itemColor = fg;
	w.innerColor = bg;
	w.innerSelectedColor = colorPlus(bg, nvgRGB(0x30, 0x30, 0x30));
	w.textColor = fg;
	w.textSelectedColor = fg;
	w.shadeTop = 0;
	w.shadeDown = 0;

	BNDtheme t;
	t.backgroundColor = colorPlus(bg, nvgRGB(0x30, 0x30, 0x30));
	t.regularTheme = w;
	t.toolTheme = w;
	t.radioTheme = w;
	t.textFieldTheme = w;
	t.optionTheme = w;
	t.choiceTheme = w;
	t.numberFieldTheme = w;
	t.sliderTheme = w;
	t.scrollBarTheme = w;
	t.tooltipTheme = w;
	t.menuTheme = w;
	t.menuItemTheme = w;

	t.sliderTheme.itemColor = bg;
	t.sliderTheme.innerColor = colorPlus(bg, nvgRGB(0x50, 0x50, 0x50));
	t.sliderTheme.innerSelectedColor = colorPlus(bg, nvgRGB(0x60, 0x60, 0x60));

	t.textFieldTheme = t.sliderTheme;
	t.textFieldTheme.textColor = colorMinus(bg, nvgRGB(0x20, 0x20, 0x20));
	t.textFieldTheme.textSelectedColor = t.textFieldTheme.textColor;

	t.scrollBarTheme.itemColor = colorPlus(bg, nvgRGB(0x50, 0x50, 0x50));
	t.scrollBarTheme.innerColor = bg;

	t.menuTheme.innerColor = colorMinus(bg, nvgRGB(0x10, 0x10, 0x10));
	t.menuTheme.textColor = colorMinus(fg, nvgRGB(0x50, 0x50, 0x50));
	t.menuTheme.textSelectedColor = t.menuTheme.textColor;

	bndSetTheme(t);
}

void windowSetFullScreen(bool fullScreen) {
   (void)fullScreen;
   // (note) not supported
}

bool windowGetFullScreen() {
   return false;
}

void windowClose(void) {
   // (note) not supported
}

////////////////////
// resources
////////////////////

Font::Font(const std::string &filename) {
   printf("xxx vstrack_plugin: Font::Font\n");
	handle = nvgCreateFont(global_ui->window.gVg, filename.c_str(), filename.c_str());
	if (handle >= 0) {
		info("Loaded font %s", filename.c_str());
	}
	else {
		warn("Failed to load font %s", filename.c_str());
	}
}

Font::~Font() {
	// There is no NanoVG deleteFont() function yet, so do nothing
}

std::shared_ptr<Font> Font::load(const std::string &filename) {
	auto sp = global_ui->window.font_cache[filename].lock();
	if (!sp)
		global_ui->window.font_cache[filename] = sp = std::make_shared<Font>(filename);
	return sp;
}

////////////////////
// Image
////////////////////

Image::Image(const std::string &filename) {
   printf("xxx vstrack_plugin: Image::Image\n");
	handle = nvgCreateImage(global_ui->window.gVg, filename.c_str(), NVG_IMAGE_REPEATX | NVG_IMAGE_REPEATY);
	if (handle > 0) {
		info("Loaded image %s", filename.c_str());
	}
	else {
		warn("Failed to load image %s", filename.c_str());
	}
}

Image::~Image() {
	// TODO What if handle is invalid?
	nvgDeleteImage(global_ui->window.gVg, handle);
}

std::shared_ptr<Image> Image::load(const std::string &filename) {
	auto sp = global_ui->window.image_cache[filename].lock();
	if (!sp)
		global_ui->window.image_cache[filename] = sp = std::make_shared<Image>(filename);
	return sp;
}

////////////////////
// SVG
////////////////////

SVG::SVG(const std::string &filename) {
   printf("xxx vstrack: SVG::SVG\n");
   // printf("xxx SVG::SVG: ENTER\n");
	handle = nsvgParseFromFile(filename.c_str(), "px", SVG_DPI);
   // printf("xxx SVG::SVG: handle=%p\n");
	if (handle) {
		info("Loaded SVG %s", filename.c_str());
	}
	else {
		warn("Failed to load SVG %s", filename.c_str());
	}
   // printf("xxx SVG::SVG: LEAVE\n");
}

SVG::~SVG() {
	nsvgDelete(handle);
}

std::shared_ptr<SVG> SVG::load(const std::string &filename) {
   // printf("xxx SVG::load: ENTER\n");
	auto sp = global_ui->window.svg_cache[filename].lock();
	if (!sp)
		global_ui->window.svg_cache[filename] = sp = std::make_shared<SVG>(filename);
   // printf("xxx SVG::load: RETURN\n");
	return sp;
}


} // namespace rack

#include "global_pre.hpp"
#include "window.hpp"
#include "app.hpp"
#include "asset.hpp"
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


#ifdef USE_VST2
extern void vst2_handle_queued_set_program_chunk (void);
#endif // USE_VST2

namespace rack {


void windowSizeCallback(GLFWwindow* window, int width, int height) {
}

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
#ifdef ARCH_MAC
	// Ctrl-left click --> right click
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (glfwGetKey(global_ui->window.gWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(global_ui->window.gWindow, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
			button = GLFW_MOUSE_BUTTON_RIGHT;
		}
	}
#endif

	if (action == GLFW_PRESS) {
		global_ui->widgets.gTempWidget = NULL;
		// onMouseDown
		{
			EventMouseDown e;
			e.pos = global_ui->window.gMousePos;
			e.button = button;
			global_ui->ui.gScene->onMouseDown(e);
			global_ui->widgets.gTempWidget = e.target;
		}

		if (button == GLFW_MOUSE_BUTTON_LEFT) {
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
	else if (action == GLFW_RELEASE) {
		// onMouseUp
		global_ui->widgets.gTempWidget = NULL;
		{
			EventMouseUp e;
			e.pos = global_ui->window.gMousePos;
			e.button = button;
			global_ui->ui.gScene->onMouseUp(e);
			global_ui->widgets.gTempWidget = e.target;
		}

		if (button == GLFW_MOUSE_BUTTON_LEFT) {
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

struct MouseButtonArguments {
	GLFWwindow *window;
	int button;
	int action;
	int mods;
};

static std::queue<MouseButtonArguments> mouseButtonQueue;
void mouseButtonStickyPop() {
	if (!mouseButtonQueue.empty()) {
		MouseButtonArguments args = mouseButtonQueue.front();
		mouseButtonQueue.pop();
		mouseButtonCallback(args.window, args.button, args.action, args.mods);
	}
}

void mouseButtonStickyCallback(GLFWwindow *window, int button, int action, int mods) {
	// Defer multiple clicks per frame to future frames
	MouseButtonArguments args = {window, button, action, mods};
	mouseButtonQueue.push(args);
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
	Vec mousePos = Vec(xpos, ypos).div(global_ui->window.gPixelRatio / global_ui->window.gWindowRatio).round();
	Vec mouseRel = mousePos.minus(global_ui->window.gMousePos);

	int cursorMode = glfwGetInputMode(global_ui->window.gWindow, GLFW_CURSOR);
	(void) cursorMode;

#ifdef ARCH_MAC
	// Workaround for Mac. We can't use GLFW_CURSOR_DISABLED because it's buggy, so implement it on our own.
	// This is not an ideal implementation. For example, if the user drags off the screen, the new mouse position will be clamped.
	if (cursorMode == GLFW_CURSOR_HIDDEN) {
		// CGSetLocalEventsSuppressionInterval(0.0);
		glfwSetCursorPos(global_ui->window.gWindow, global_ui->window.gMousePos.x, global_ui->window.gMousePos.y);
		CGAssociateMouseAndMouseCursorPosition(true);
		mousePos = global_ui->window.gMousePos;
	}
	// Because sometimes the cursor turns into an arrow when its position is on the boundary of the window
	glfwSetCursor(global_ui->window.gWindow, NULL);
#endif

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
	if (glfwGetMouseButton(global_ui->window.gWindow, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
		// TODO
		// Define a new global called gScrollWidget, which remembers the widget where middle-click was first pressed
		EventScroll e;
		e.pos = mousePos;
		e.scrollRel = mouseRel;
		global_ui->ui.gScene->onScroll(e);
	}
}

void cursorEnterCallback(GLFWwindow* window, int entered) {
	if (!entered) {
		if (global_ui->widgets.gHoveredWidget) {
			// onMouseLeave
			EventMouseLeave e;
			global_ui->widgets.gHoveredWidget->onMouseLeave(e);
		}
		global_ui->widgets.gHoveredWidget = NULL;
	}
}

void scrollCallback(GLFWwindow *window, double x, double y) {
	Vec scrollRel = Vec(x, y);
#if ARCH_LIN || ARCH_WIN
	if (windowIsShiftPressed())
		scrollRel = Vec(y, x);
#endif
	// onScroll
	EventScroll e;
	e.pos = global_ui->window.gMousePos;
	e.scrollRel = scrollRel.mult(50.0);
	global_ui->ui.gScene->onScroll(e);
}

void charCallback(GLFWwindow *window, unsigned int codepoint) {
	if (global_ui->widgets.gFocusedWidget) {
		// onText
		EventText e;
		e.codepoint = codepoint;
		global_ui->widgets.gFocusedWidget->onText(e);
	}
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		if (global_ui->widgets.gFocusedWidget) {
			// onKey
			EventKey e;
			e.key = key;
			global_ui->widgets.gFocusedWidget->onKey(e);
			if (e.consumed)
				return;
		}
		// onHoverKey
		EventHoverKey e;
		e.pos = global_ui->window.gMousePos;
		e.key = key;
		global_ui->ui.gScene->onHoverKey(e);
	}

	// Keyboard MIDI driver
	if (!(mods & (GLFW_MOD_SHIFT | GLFW_MOD_CONTROL | GLFW_MOD_ALT | GLFW_MOD_SUPER))) {
		if (action == GLFW_PRESS) {
			keyboardPress(key);
		}
		else if (action == GLFW_RELEASE) {
			keyboardRelease(key);
		}
	}
}

void dropCallback(GLFWwindow *window, int count, const char **paths) {
	// onPathDrop
	EventPathDrop e;
	e.pos = global_ui->window.gMousePos;
	for (int i = 0; i < count; i++) {
		e.paths.push_back(paths[i]);
	}
	global_ui->ui.gScene->onPathDrop(e);
}

void errorCallback(int error, const char *description) {
	warn("GLFW error %d: %s", error, description);
}

void renderGui() {
	int width, height;
	glfwGetFramebufferSize(global_ui->window.gWindow, &width, &height);

	// Update and render
	nvgBeginFrame(global_ui->window.gVg, width, height, global_ui->window.gPixelRatio);

	nvgReset(global_ui->window.gVg);
	nvgScale(global_ui->window.gVg, global_ui->window.gPixelRatio, global_ui->window.gPixelRatio);
	global_ui->ui.gScene->draw(global_ui->window.gVg);

	glViewport(0, 0, width, height);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	nvgEndFrame(global_ui->window.gVg);
	glfwSwapBuffers(global_ui->window.gWindow);
}

void windowInit() {
	int err;
   bool bInitGLFW = true;

#ifdef USE_VST2
   // (note) GLFW uses global vars. hmmmmm. multiple init seems to work, though (and opening multiple windows)
   // (note) just don't terminate until last instance is closed
   // (note) (this will probably result in memleaks..?!)
   bInitGLFW = (1 == global->vst2.last_seen_instance_count);
#endif // USE_VST2

#if 0
   if(bInitGLFW)
   {
      // Set up GLFW
      glfwSetErrorCallback(errorCallback);
      err = glfwInit();
      if (err != GLFW_TRUE) {
         osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, "Could not initialize GLFW.");
         exit(1);
      }
   }
#else
   glfwSetErrorCallback(errorCallback);
#endif

#if defined NANOVG_GL2
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#elif defined NANOVG_GL3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	global_ui->window.lastWindowTitle = "";
	global_ui->window.gWindow = glfwCreateWindow(640, 480, global_ui->window.lastWindowTitle.c_str(), NULL, NULL);
	if (!global_ui->window.gWindow) {
		osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, "Cannot open window with OpenGL 2.0 renderer. Does your graphics card support OpenGL 2.0 or greater? If so, make sure you have the latest graphics drivers installed.");
		exit(1);
	}

	glfwMakeContextCurrent(global_ui->window.gWindow);

	glfwSwapInterval(1);

	glfwSetInputMode(global_ui->window.gWindow, GLFW_LOCK_KEY_MODS, 1);

	glfwSetWindowSizeCallback(global_ui->window.gWindow, windowSizeCallback);
	glfwSetMouseButtonCallback(global_ui->window.gWindow, mouseButtonStickyCallback);
	// Call this ourselves, but on every frame instead of only when the mouse moves
	// glfwSetCursorPosCallback(gWindow, cursorPosCallback);
	glfwSetCursorEnterCallback(global_ui->window.gWindow, cursorEnterCallback);
	glfwSetScrollCallback(global_ui->window.gWindow, scrollCallback);
	glfwSetCharCallback(global_ui->window.gWindow, charCallback);
	glfwSetKeyCallback(global_ui->window.gWindow, keyCallback);
	glfwSetDropCallback(global_ui->window.gWindow, dropCallback);

	// Set up GLEW
	glewExperimental = GL_TRUE;
	err = glewInit();
	if (err != GLEW_OK) {
		osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, "Could not initialize GLEW. Does your graphics card support OpenGL 2.0 or greater? If so, make sure you have the latest graphics drivers installed.");
		exit(1);
	}

	// GLEW generates GL error because it calls glGetString(GL_EXTENSIONS), we'll consume it here.
	glGetError();

	glfwSetWindowSizeLimits(global_ui->window.gWindow, 640, 480, GLFW_DONT_CARE, GLFW_DONT_CARE);

	// Set up NanoVG
	int nvgFlags = NVG_ANTIALIAS;
#if defined NANOVG_GL2
	global_ui->window.gVg = nvgCreateGL2(nvgFlags);
#elif defined NANOVG_GL3
	global_ui->window.gVg = nvgCreateGL3(nvgFlags);
#elif defined NANOVG_GLES2
	global_ui->window.gVg = nvgCreateGLES2(nvgFlags);
#endif
	assert(global_ui->window.gVg);

#if defined NANOVG_GL2
	global_ui->window.gFramebufferVg = nvgCreateGL2(nvgFlags);
#elif defined NANOVG_GL3
	global_ui->window.gFramebufferVg = nvgCreateGL3(nvgFlags);
#elif defined NANOVG_GLES2
	global_ui->window.gFramebufferVg = nvgCreateGLES2(nvgFlags);
#endif
	assert(global_ui->window.gFramebufferVg);

	// Set up Blendish
	global_ui->window.gGuiFont = Font::load(assetGlobal("res/fonts/DejaVuSans.ttf"));
	bndSetFont(global_ui->window.gGuiFont->handle);

	windowSetTheme(nvgRGB(0x33, 0x33, 0x33), nvgRGB(0xf0, 0xf0, 0xf0));

#ifdef USE_VST2
   // (note) the patch loader requires an initialized window system (widgets etc)
   ::glfwHideWindow(global_ui->window.gWindow);
#endif // USE_VST2
}

void windowDestroy() {
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

	glfwDestroyWindow(global_ui->window.gWindow);
   global_ui->window.gWindow = 0;

#if 0
   bool bTerminateGLFW = true;
#ifdef USE_VST2
   bTerminateGLFW = (1 == global->vst2.last_seen_instance_count);
#endif // USE_VST2
   if(bTerminateGLFW)
      glfwTerminate();
#endif
}

void windowRun() {
   printf("xxx vcvrack: windowRun() ENTER\n");
	assert(global_ui->window.gWindow);
	global_ui->window.gGuiFrame = 0;
	while(!glfwWindowShouldClose(global_ui->window.gWindow)
#ifdef USE_VST2
         && !global_ui->vst2.b_close_window
#endif // USE_VST2
         ) {

      vst2_handle_queued_set_program_chunk();

		double startTime = glfwGetTime();
		global_ui->window.gGuiFrame++;
      // printf("xxx vcvrack: windowRun(): startTime=%g\n", startTime);

		// Poll events
		glfwPollEvents();
		{
			double xpos, ypos;
			glfwGetCursorPos(global_ui->window.gWindow, &xpos, &ypos);
			cursorPosCallback(global_ui->window.gWindow, xpos, ypos);
		}
		mouseButtonStickyPop();

#ifndef USE_VST2
		gamepadStep();
#endif // !USE_VST2

		// Set window title
		std::string windowTitle;
		windowTitle = global_ui->app.gApplicationName;
		windowTitle += " ";
		windowTitle += global_ui->app.gApplicationVersion;
		if (!global_ui->app.gRackWidget->lastPath.empty()) {
			windowTitle += " - ";
			windowTitle += stringFilename(global_ui->app.gRackWidget->lastPath);
		}
		if (windowTitle != global_ui->window.lastWindowTitle) {
			glfwSetWindowTitle(global_ui->window.gWindow, windowTitle.c_str());
			global_ui->window.lastWindowTitle = windowTitle;
		}

		// Get desired scaling
		float pixelRatio;
		glfwGetWindowContentScale(global_ui->window.gWindow, &pixelRatio, NULL);
		pixelRatio = roundf(pixelRatio);
		if (pixelRatio != global_ui->window.gPixelRatio) {
			EventZoom eZoom;
			global_ui->ui.gScene->onZoom(eZoom);
			global_ui->window.gPixelRatio = pixelRatio;
		}

		// Get framebuffer/window ratio
		int width, height;
		glfwGetFramebufferSize(global_ui->window.gWindow, &width, &height);
		int windowWidth, windowHeight;
		glfwGetWindowSize(global_ui->window.gWindow, &windowWidth, &windowHeight);
		global_ui->window.gWindowRatio = (float)width / windowWidth;

		global_ui->ui.gScene->box.size = Vec(width, height).div(global_ui->window.gPixelRatio);

		// Step scene
		global_ui->ui.gScene->step();

		// Render
		bool visible = glfwGetWindowAttrib(global_ui->window.gWindow, GLFW_VISIBLE) && !glfwGetWindowAttrib(global_ui->window.gWindow, GLFW_ICONIFIED);
		if (visible) {
			renderGui();
		}

		// Limit framerate manually if vsync isn't working
		double endTime = glfwGetTime();
		double frameTime = endTime - startTime;
		double minTime = 1.0 / 90.0;
		if (frameTime < minTime) {
			std::this_thread::sleep_for(std::chrono::duration<double>(minTime - frameTime));
		}
		endTime = glfwGetTime();
		// info("%lf fps", 1.0 / (endTime - startTime));

#ifdef USE_VST2
      if(global_ui->vst2.b_hide_window)
      {
         global_ui->vst2.b_hide_window = 0;
         break;
      }
#endif // USE_VST2
	}

#ifdef USE_VST2
   ::glfwSetWindowShouldClose(global_ui->window.gWindow, 0);
   ::glfwHideWindow(global_ui->window.gWindow);
#endif

   printf("xxx vcvrack: windowRun() LEAVE\n");
}

void windowClose() {
	glfwSetWindowShouldClose(global_ui->window.gWindow, GLFW_TRUE);
}

void windowCursorLock() {
	if (global_ui->window.gAllowCursorLock) {
#ifdef ARCH_MAC
		glfwSetInputMode(global_ui->window.gWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
#else
		glfwSetInputMode(global_ui->window.gWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#endif
	}
}

void windowCursorUnlock() {
	if (global_ui->window.gAllowCursorLock) {
		glfwSetInputMode(global_ui->window.gWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

bool windowIsModPressed() {
#ifdef ARCH_MAC
	return glfwGetKey(global_ui->window.gWindow, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS || glfwGetKey(global_ui->window.gWindow, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS;
#else
	return glfwGetKey(global_ui->window.gWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(global_ui->window.gWindow, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
#endif
}

bool windowIsShiftPressed() {
	return glfwGetKey(global_ui->window.gWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(global_ui->window.gWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
}

Vec windowGetWindowSize() {
	int width, height;
	glfwGetWindowSize(global_ui->window.gWindow, &width, &height);
	return Vec(width, height);
}

void windowSetWindowSize(Vec size) {
	int width = size.x;
	int height = size.y;
	glfwSetWindowSize(global_ui->window.gWindow, width, height);
}

Vec windowGetWindowPos() {
	int x, y;
	glfwGetWindowPos(global_ui->window.gWindow, &x, &y);
	return Vec(x, y);
}

void windowSetWindowPos(Vec pos) {
	int x = pos.x;
	int y = pos.y;
	glfwSetWindowPos(global_ui->window.gWindow, x, y);
}

bool windowIsMaximized() {
   if(global_ui->window.gWindow)
      return glfwGetWindowAttrib(global_ui->window.gWindow, GLFW_MAXIMIZED);
   else
      return false;
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
	if (windowGetFullScreen()) {
		glfwSetWindowMonitor(global_ui->window.gWindow, NULL, global_ui->window.windowX, global_ui->window.windowY, global_ui->window.windowWidth, global_ui->window.windowHeight, GLFW_DONT_CARE);
	}
	else {
		glfwGetWindowPos(global_ui->window.gWindow, &global_ui->window.windowX, &global_ui->window.windowY);
		glfwGetWindowSize(global_ui->window.gWindow, &global_ui->window.windowWidth, &global_ui->window.windowHeight);
		GLFWmonitor *monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(global_ui->window.gWindow, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	}
}

bool windowGetFullScreen() {
	GLFWmonitor *monitor = glfwGetWindowMonitor(global_ui->window.gWindow);
	return monitor != NULL;
}


////////////////////
// resources
////////////////////

Font::Font(const std::string &filename) {
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
	handle = nsvgParseFromFile(filename.c_str(), "px", SVG_DPI);
	if (handle) {
		info("Loaded SVG %s", filename.c_str());
	}
	else {
		warn("Failed to load SVG %s", filename.c_str());
	}
}

SVG::~SVG() {
	nsvgDelete(handle);
}

std::shared_ptr<SVG> SVG::load(const std::string &filename) {
	auto sp = global_ui->window.svg_cache[filename].lock();
	if (!sp)
		global_ui->window.svg_cache[filename] = sp = std::make_shared<SVG>(filename);
	return sp;
}


} // namespace rack

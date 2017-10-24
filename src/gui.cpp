#include "gui.hpp"
#include "app.hpp"
#include "asset.hpp"

#include <map>
#include <queue>
#include <thread>

#include "../ext/osdialog/osdialog.h"

#define NANOVG_GL2_IMPLEMENTATION
// #define NANOVG_GL3_IMPLEMENTATION
#include "../ext/nanovg/src/nanovg_gl.h"
// Hack to get framebuffer objects working on OpenGL 2 (we blindly assume the extension is supported)
#define NANOVG_FBO_VALID 1
#include "../ext/nanovg/src/nanovg_gl_utils.h"
#define BLENDISH_IMPLEMENTATION
#include "../ext/oui-blendish/blendish.h"
#define NANOSVG_IMPLEMENTATION
#define NANOSVG_ALL_COLOR_KEYWORDS
#include "../ext/nanosvg/src/nanosvg.h"

#ifdef ARCH_MAC
	// For CGAssociateMouseAndMouseCursorPosition
	#include <ApplicationServices/ApplicationServices.h>
#endif

namespace rack {

GLFWwindow *gWindow = NULL;
NVGcontext *gVg = NULL;
NVGcontext *gFramebufferVg = NULL;
std::shared_ptr<Font> gGuiFont;
float gPixelRatio = 0.0;
bool gAllowCursorLock = true;


void windowSizeCallback(GLFWwindow* window, int width, int height) {
	gScene->box.size = Vec(width, height);
}

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
#ifdef ARCH_MAC
	// Ctrl-left click --> right click
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (glfwGetKey(gWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(gWindow, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
			button = GLFW_MOUSE_BUTTON_RIGHT;
		}
	}
#endif

	if (action == GLFW_PRESS) {
		// onMouseDown
		Widget *w = gScene->onMouseDown(gMousePos, button);

		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (w) {
				// onDragStart
				w->onDragStart();
			}
			gDraggedWidget = w;

			if (w != gFocusedWidget) {
				if (gFocusedWidget) {
					// onDefocus
					w->onDefocus();
				}
				gFocusedWidget = NULL;
				if (w) {
					// onFocus
					if (w->onFocus()) {
						gFocusedWidget = w;
					}
				}
			}
		}
	}
	else if (action == GLFW_RELEASE) {
		// onMouseUp
		Widget *w = gScene->onMouseUp(gMousePos, button);

		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (gDraggedWidget) {
				// onDragDrop
				w->onDragDrop(gDraggedWidget);
			}
			// gDraggedWidget might have been set to null in the last event, recheck here
			if (gDraggedWidget) {
				// onDragEnd
				gDraggedWidget->onDragEnd();
			}
			gDraggedWidget = NULL;
			gDragHoveredWidget = NULL;
		}
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
	Vec mousePos = Vec(xpos, ypos).round();
	Vec mouseRel = mousePos.minus(gMousePos);

#ifdef ARCH_MAC
	// Workaround for Mac. We can't use GLFW_CURSOR_DISABLED because it's buggy, so implement it on our own.
	// This is not an ideal implementation. For example, if the user drags off the screen, the new mouse position will be clamped.
	int mouseMode = glfwGetInputMode(gWindow, GLFW_CURSOR);
	if (mouseMode == GLFW_CURSOR_HIDDEN) {
		// CGSetLocalEventsSuppressionInterval(0.0);
		glfwSetCursorPos(gWindow, gMousePos.x, gMousePos.y);
		CGAssociateMouseAndMouseCursorPosition(true);
		mousePos = gMousePos;
	}
	// Because sometimes the cursor turns into an arrow when its position is on the boundary of the window
	glfwSetCursor(gWindow, NULL);
#endif

	gMousePos = mousePos;

	// onMouseMove
	Widget *hovered = gScene->onMouseMove(gMousePos, mouseRel);

	if (gDraggedWidget) {
		// onDragMove
		gDraggedWidget->onDragMove(mouseRel);

		if (hovered != gDragHoveredWidget) {
			if (gDragHoveredWidget) {
				gDragHoveredWidget->onDragLeave(gDraggedWidget);
			}
			if (hovered) {
				hovered->onDragEnter(gDraggedWidget);
			}
			gDragHoveredWidget = hovered;
		}
	}
	else {
		if (hovered != gHoveredWidget) {
			if (gHoveredWidget) {
				// onMouseLeave
				gHoveredWidget->onMouseLeave();
			}
			if (hovered) {
			// onMouseEnter
				hovered->onMouseEnter();
			}
			gHoveredWidget = hovered;
		}
	}
	if (glfwGetMouseButton(gWindow, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
		// TODO
		// Define a new global called gScrollWidget, which remembers the widget where middle-click was first pressed
		gScene->onScroll(mousePos, mouseRel);
	}
}

void cursorEnterCallback(GLFWwindow* window, int entered) {
	if (!entered) {
		if (gHoveredWidget) {
			gHoveredWidget->onMouseLeave();
		}
		gHoveredWidget = NULL;
	}
}

void scrollCallback(GLFWwindow *window, double x, double y) {
	Vec scrollRel = Vec(x, y);
#if ARCH_LIN || ARCH_WIN
	if (guiIsShiftPressed())
		scrollRel = Vec(y, x);
#endif
	// onScroll
	gScene->onScroll(gMousePos, scrollRel.mult(50.0));
}

void charCallback(GLFWwindow *window, unsigned int codepoint) {
	if (gFocusedWidget) {
		gFocusedWidget->onFocusText(codepoint);
	}
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		// onFocusKey
		if (gFocusedWidget && gFocusedWidget->onFocusKey(key))
			return;
		// onHoverKey
		gScene->onHoverKey(gMousePos, key);
	}
}

void errorCallback(int error, const char *description) {
	fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

void renderGui() {
	int width, height;
	glfwGetFramebufferSize(gWindow, &width, &height);
	int windowWidth, windowHeight;
	glfwGetWindowSize(gWindow, &windowWidth, &windowHeight);
	gPixelRatio = (float)width / windowWidth;

	// Update and render
	nvgBeginFrame(gVg, width, height, gPixelRatio);

	nvgReset(gVg);
	nvgScale(gVg, gPixelRatio, gPixelRatio);
	gScene->draw(gVg);

	glViewport(0, 0, width, height);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	nvgEndFrame(gVg);
	glfwSwapBuffers(gWindow);
}

void guiInit() {
	int err;

	// Set up GLFW
	glfwSetErrorCallback(errorCallback);
	err = glfwInit();
	if (err != GLFW_TRUE) {
		osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, "Could not initialize GLFW.");
		exit(1);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	// glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	gWindow = glfwCreateWindow(640, 480, "", NULL, NULL);
	if (!gWindow) {
		osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, "Cannot open window with OpenGL 2.0 renderer. Does your graphics card support OpenGL 2.0 or greater? If so, are the latest drivers installed?");
		exit(1);
	}

	glfwMakeContextCurrent(gWindow);

	glfwSwapInterval(1);

	glfwSetWindowSizeCallback(gWindow, windowSizeCallback);
	glfwSetMouseButtonCallback(gWindow, mouseButtonStickyCallback);
	// glfwSetCursorPosCallback(gWindow, cursorPosCallback);
	glfwSetCursorEnterCallback(gWindow, cursorEnterCallback);
	glfwSetScrollCallback(gWindow, scrollCallback);
	glfwSetCharCallback(gWindow, charCallback);
	glfwSetKeyCallback(gWindow, keyCallback);

	// Set up GLEW
	glewExperimental = GL_TRUE;
	err = glewInit();
	if (err != GLEW_OK) {
		osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, "Could not initialize GLEW. Does your graphics card support OpenGL 2.0 or greater? If so, are the latest drivers installed?");
		exit(1);
	}

	// GLEW generates GL error because it calls glGetString(GL_EXTENSIONS), we'll consume it here.
	glGetError();

	glfwSetWindowSizeLimits(gWindow, 640, 480, GLFW_DONT_CARE, GLFW_DONT_CARE);

	// Set up NanoVG
	gVg = nvgCreateGL2(NVG_ANTIALIAS);
	// gVg = nvgCreateGL3(NVG_ANTIALIAS);
	assert(gVg);

	gFramebufferVg = nvgCreateGL2(NVG_ANTIALIAS);
	assert(gFramebufferVg);

	// Set up Blendish
	gGuiFont = Font::load(assetGlobal("res/DejaVuSans.ttf"));
	bndSetFont(gGuiFont->handle);
	// bndSetIconImage(loadImage(assetGlobal("res/icons.png")));
}

void guiDestroy() {
	gGuiFont.reset();
	nvgDeleteGL2(gVg);
	// nvgDeleteGL3(gVg);
	nvgDeleteGL2(gFramebufferVg);
	glfwDestroyWindow(gWindow);
	glfwTerminate();
}

void guiRun() {
	assert(gWindow);
	{
		int width, height;
		glfwGetWindowSize(gWindow, &width, &height);
		windowSizeCallback(gWindow, width, height);
	}
	gGuiFrame = 0;
	while(!glfwWindowShouldClose(gWindow)) {
		double startTime = glfwGetTime();
		gGuiFrame++;

		// Poll events
		glfwPollEvents();
		{
			double xpos, ypos;
			glfwGetCursorPos(gWindow, &xpos, &ypos);
			cursorPosCallback(gWindow, xpos, ypos);
		}
		mouseButtonStickyPop();

		// Set window title
		std::string title = gApplicationName;
		if (!gApplicationVersion.empty()) {
			title += " v" + gApplicationVersion;
		}
		if (!gRackWidget->lastPath.empty()) {
			title += " - ";
			title += extractFilename(gRackWidget->lastPath);
		}
		glfwSetWindowTitle(gWindow, title.c_str());

		// Step scene
		gScene->step();

		// Render
		bool visible = glfwGetWindowAttrib(gWindow, GLFW_VISIBLE) && !glfwGetWindowAttrib(gWindow, GLFW_ICONIFIED);
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
		// printf("%lf fps\n", 1.0 / (endTime - startTime));
	}
}

void guiClose() {
	glfwSetWindowShouldClose(gWindow, GLFW_TRUE);
}

void guiCursorLock() {
	if (gAllowCursorLock) {
#ifdef ARCH_MAC
		glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
#else
		glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#endif
	}
}

void guiCursorUnlock() {
	if (gAllowCursorLock) {
		glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

bool guiIsModPressed() {
#ifdef ARCH_MAC
	return glfwGetKey(gWindow, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS || glfwGetKey(gWindow, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS;
#else
	return glfwGetKey(gWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(gWindow, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
#endif
}

bool guiIsShiftPressed() {
	return glfwGetKey(gWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(gWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
}


////////////////////
// resources
////////////////////

Font::Font(const std::string &filename) {
	handle = nvgCreateFont(gVg, filename.c_str(), filename.c_str());
	if (handle >= 0) {
		fprintf(stderr, "Loaded font %s\n", filename.c_str());
	}
	else {
		fprintf(stderr, "Failed to load font %s\n", filename.c_str());
	}
}

Font::~Font() {
	// There is no NanoVG deleteFont() function yet, so do nothing
}

std::shared_ptr<Font> Font::load(const std::string &filename) {
	static std::map<std::string, std::weak_ptr<Font>> cache;
	auto sp = cache[filename].lock();
	if (!sp)
		cache[filename] = sp = std::make_shared<Font>(filename);
	return sp;
}

////////////////////
// Image
////////////////////

Image::Image(const std::string &filename) {
	handle = nvgCreateImage(gVg, filename.c_str(), NVG_IMAGE_REPEATX | NVG_IMAGE_REPEATY);
	if (handle > 0) {
		fprintf(stderr, "Loaded image %s\n", filename.c_str());
	}
	else {
		fprintf(stderr, "Failed to load image %s\n", filename.c_str());
	}
}

Image::~Image() {
	// TODO What if handle is invalid?
	nvgDeleteImage(gVg, handle);
}

std::shared_ptr<Image> Image::load(const std::string &filename) {
	static std::map<std::string, std::weak_ptr<Image>> cache;
	auto sp = cache[filename].lock();
	if (!sp)
		cache[filename] = sp = std::make_shared<Image>(filename);
	return sp;
}

////////////////////
// SVG
////////////////////

SVG::SVG(const std::string &filename) {
	handle = nsvgParseFromFile(filename.c_str(), "px", 96.0);
	if (handle) {
		fprintf(stderr, "Loaded SVG %s\n", filename.c_str());
	}
	else {
		fprintf(stderr, "Failed to load SVG %s\n", filename.c_str());
	}
}

SVG::~SVG() {
	nsvgDelete(handle);
}

std::shared_ptr<SVG> SVG::load(const std::string &filename) {
	static std::map<std::string, std::weak_ptr<SVG>> cache;
	auto sp = cache[filename].lock();
	if (!sp)
		cache[filename] = sp = std::make_shared<SVG>(filename);
	return sp;
}


} // namespace rack

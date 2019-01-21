#include "window.hpp"
#include "asset.hpp"
#include "app/Scene.hpp"
#include "keyboard.hpp"
#include "gamepad.hpp"
#include "event.hpp"
#include "app.hpp"

#include <map>
#include <queue>
#include <thread>

#ifdef ARCH_MAC
	// For CGAssociateMouseAndMouseCursorPosition
	#include <ApplicationServices/ApplicationServices.h>
#endif

#include <osdialog.h>


namespace rack {


static std::map<std::string, std::weak_ptr<Font>> fontCache;
static std::map<std::string, std::weak_ptr<Image>> imageCache;
static std::map<std::string, std::weak_ptr<SVG>> svgCache;


Font::Font(const std::string &filename) {
	handle = nvgCreateFont(app()->window->vg, filename.c_str(), filename.c_str());
	if (handle >= 0) {
		INFO("Loaded font %s", filename.c_str());
	}
	else {
		WARN("Failed to load font %s", filename.c_str());
	}
}

Font::~Font() {
	// There is no NanoVG deleteFont() function yet, so do nothing
}

std::shared_ptr<Font> Font::load(const std::string &filename) {
	auto sp = fontCache[filename].lock();
	if (!sp)
		fontCache[filename] = sp = std::make_shared<Font>(filename);
	return sp;
}

Image::Image(const std::string &filename) {
	handle = nvgCreateImage(app()->window->vg, filename.c_str(), NVG_IMAGE_REPEATX | NVG_IMAGE_REPEATY);
	if (handle > 0) {
		INFO("Loaded image %s", filename.c_str());
	}
	else {
		WARN("Failed to load image %s", filename.c_str());
	}
}

Image::~Image() {
	// TODO What if handle is invalid?
	nvgDeleteImage(app()->window->vg, handle);
}

std::shared_ptr<Image> Image::load(const std::string &filename) {
	auto sp = imageCache[filename].lock();
	if (!sp)
		imageCache[filename] = sp = std::make_shared<Image>(filename);
	return sp;
}

SVG::SVG(const std::string &filename) {
	handle = nsvgParseFromFile(filename.c_str(), "px", APP_SVG_DPI);
	if (handle) {
		INFO("Loaded SVG %s", filename.c_str());
	}
	else {
		WARN("Failed to load SVG %s", filename.c_str());
	}
}

SVG::~SVG() {
	nsvgDelete(handle);
}

std::shared_ptr<SVG> SVG::load(const std::string &filename) {
	auto sp = svgCache[filename].lock();
	if (!sp)
		svgCache[filename] = sp = std::make_shared<SVG>(filename);
	return sp;
}


struct Window::Internal {
	std::string lastWindowTitle;

	int lastWindowX = 0;
	int lastWindowY = 0;
	int lastWindowWidth = 0;
	int lastWindowHeight = 0;
};


static void windowSizeCallback(GLFWwindow *win, int width, int height) {
	// Do nothing. Window size is reset each frame anyway.
}

static void mouseButtonCallback(GLFWwindow *win, int button, int action, int mods) {
	Window *window = (Window*) glfwGetWindowUserPointer(win);
#ifdef ARCH_MAC
	// Remap Ctrl-left click to right click on Mac
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (mods & GLFW_MOD_CONTROL) {
			button = GLFW_MOUSE_BUTTON_RIGHT;
			mods &= ~GLFW_MOD_CONTROL;
		}
	}
#endif

	app()->event->handleButton(window->mousePos, button, action, mods);
}

static void cursorPosCallback(GLFWwindow *win, double xpos, double ypos) {
	Window *window = (Window*) glfwGetWindowUserPointer(win);
	math::Vec mousePos = math::Vec(xpos, ypos).div(window->pixelRatio / window->windowRatio).round();
	math::Vec mouseDelta = mousePos.minus(window->mousePos);

	int cursorMode = glfwGetInputMode(win, GLFW_CURSOR);
	(void) cursorMode;

#ifdef ARCH_MAC
	// Workaround for Mac. We can't use GLFW_CURSOR_DISABLED because it's buggy, so implement it on our own.
	// This is not an ideal implementation. For example, if the user drags off the screen, the new mouse position will be clamped.
	if (cursorMode == GLFW_CURSOR_HIDDEN) {
		// CGSetLocalEventsSuppressionInterval(0.0);
		glfwSetCursorPos(win, window->mousePos.x, window->mousePos.y);
		CGAssociateMouseAndMouseCursorPosition(true);
		mousePos = window->mousePos;
	}
	// Because sometimes the cursor turns into an arrow when its position is on the boundary of the window
	glfwSetCursor(win, NULL);
#endif

	window->mousePos = mousePos;

	app()->event->handleHover(mousePos, mouseDelta);
}

static void cursorEnterCallback(GLFWwindow *win, int entered) {
	if (!entered) {
		app()->event->handleLeave();
	}
}

static void scrollCallback(GLFWwindow *win, double x, double y) {
	Window *window = (Window*) glfwGetWindowUserPointer(win);
	math::Vec scrollDelta = math::Vec(x, y);
	scrollDelta = scrollDelta.mult(50.0);

	app()->event->handleScroll(window->mousePos, scrollDelta);
}

static void charCallback(GLFWwindow *win, unsigned int codepoint) {
	Window *window = (Window*) glfwGetWindowUserPointer(win);
	app()->event->handleText(window->mousePos, codepoint);
}

static void keyCallback(GLFWwindow *win, int key, int scancode, int action, int mods) {
	Window *window = (Window*) glfwGetWindowUserPointer(win);
	app()->event->handleKey(window->mousePos, key, scancode, action, mods);

	// Keyboard MIDI driver
	if ((mods & WINDOW_MOD_MASK) == 0) {
		if (action == GLFW_PRESS) {
			keyboard::press(key);
		}
		else if (action == GLFW_RELEASE) {
			keyboard::release(key);
		}
	}
}

static void dropCallback(GLFWwindow *win, int count, const char **paths) {
	Window *window = (Window*) glfwGetWindowUserPointer(win);
	std::vector<std::string> pathsVec;
	for (int i = 0; i < count; i++) {
		pathsVec.push_back(paths[i]);
	}
	app()->event->handleDrop(window->mousePos, pathsVec);
}

static void errorCallback(int error, const char *description) {
	WARN("GLFW error %d: %s", error, description);
}

Window::Window() {
	internal = new Internal;
	int err;

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

	internal->lastWindowTitle = "";
	win = glfwCreateWindow(800, 600, internal->lastWindowTitle.c_str(), NULL, NULL);
	if (!win) {
		osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, "Cannot open window with OpenGL 2.0 renderer. Does your graphics card support OpenGL 2.0 or greater? If so, make sure you have the latest graphics drivers installed.");
		exit(1);
	}

	glfwSetWindowUserPointer(win, this);
	glfwSetInputMode(win, GLFW_LOCK_KEY_MODS, 1);

	glfwMakeContextCurrent(win);
	glfwSwapInterval(1);

	glfwSetWindowSizeCallback(win, windowSizeCallback);
	glfwSetMouseButtonCallback(win, mouseButtonCallback);
	// Call this ourselves, but on every frame instead of only when the mouse moves
	// glfwSetCursorPosCallback(win, cursorPosCallback);
	glfwSetCursorEnterCallback(win, cursorEnterCallback);
	glfwSetScrollCallback(win, scrollCallback);
	glfwSetCharCallback(win, charCallback);
	glfwSetKeyCallback(win, keyCallback);
	glfwSetDropCallback(win, dropCallback);

	// Set up GLEW
	glewExperimental = GL_TRUE;
	err = glewInit();
	if (err != GLEW_OK) {
		osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, "Could not initialize GLEW. Does your graphics card support OpenGL 2.0 or greater? If so, make sure you have the latest graphics drivers installed.");
		exit(1);
	}

	const GLubyte *renderer = glGetString(GL_RENDERER);
	const GLubyte *version = glGetString(GL_VERSION);
	INFO("Renderer: %s", renderer);
	INFO("OpenGL: %s", version);

	// GLEW generates GL error because it calls glGetString(GL_EXTENSIONS), we'll consume it here.
	glGetError();

	glfwSetWindowSizeLimits(win, 800, 600, GLFW_DONT_CARE, GLFW_DONT_CARE);

	// Set up NanoVG
	int nvgFlags = NVG_ANTIALIAS;
#if defined NANOVG_GL2
	vg = nvgCreateGL2(nvgFlags);
#elif defined NANOVG_GL3
	vg = nvgCreateGL3(nvgFlags);
#elif defined NANOVG_GLES2
	vg = nvgCreateGLES2(nvgFlags);
#endif
	assert(vg);

#if defined NANOVG_GL2
	fbVg = nvgCreateGL2(nvgFlags);
#elif defined NANOVG_GL3
	fbVg = nvgCreateGL3(nvgFlags);
#elif defined NANOVG_GLES2
	fbVg = nvgCreateGLES2(nvgFlags);
#endif
	assert(fbVg);
}

Window::~Window() {
#if defined NANOVG_GL2
	nvgDeleteGL2(vg);
#elif defined NANOVG_GL3
	nvgDeleteGL3(vg);
#elif defined NANOVG_GLES2
	nvgDeleteGLES2(vg);
#endif

#if defined NANOVG_GL2
	nvgDeleteGL2(fbVg);
#elif defined NANOVG_GL3
	nvgDeleteGL3(fbVg);
#elif defined NANOVG_GLES2
	nvgDeleteGLES2(fbVg);
#endif

	glfwDestroyWindow(win);
	delete internal;
}

void Window::run() {
	uiFont = Font::load(asset::system("res/fonts/DejaVuSans.ttf"));

	frame = 0;
	while(!glfwWindowShouldClose(win)) {
		double startTime = glfwGetTime();

		// Poll events
		glfwPollEvents();
		// In case glfwPollEvents() set another OpenGL context
		glfwMakeContextCurrent(win);
		// Call cursorPosCallback every frame, not just when the mouse moves
		{
			double xpos, ypos;
			glfwGetCursorPos(win, &xpos, &ypos);
			cursorPosCallback(win, xpos, ypos);
		}
		gamepad::step();

		// Set window title
		std::string windowTitle;
		windowTitle = APP_NAME;
		windowTitle += " ";
		windowTitle += APP_VERSION;
		if (!app()->scene->rackWidget->lastPath.empty()) {
			windowTitle += " - ";
			windowTitle += string::filename(app()->scene->rackWidget->lastPath);
		}
		if (windowTitle != internal->lastWindowTitle) {
			glfwSetWindowTitle(win, windowTitle.c_str());
			internal->lastWindowTitle = windowTitle;
		}

		bndSetFont(uiFont->handle);

		// Get desired scaling
		float pixelRatio;
		glfwGetWindowContentScale(win, &pixelRatio, NULL);
		pixelRatio = std::round(pixelRatio);
		if (pixelRatio != this->pixelRatio) {
			app()->event->handleZoom();
			this->pixelRatio = pixelRatio;
		}

		// Get framebuffer/window ratio
		int fbWidth, fbHeight;
		glfwGetFramebufferSize(win, &fbWidth, &fbHeight);
		int winWidth, winHeight;
		glfwGetWindowSize(win, &winWidth, &winHeight);
		windowRatio = (float)fbWidth / winWidth;

		// Resize scene
		app()->event->rootWidget->box.size = math::Vec(fbWidth, fbHeight).div(pixelRatio);

		// Step scene
		app()->event->rootWidget->step();

		// Render
		bool visible = glfwGetWindowAttrib(win, GLFW_VISIBLE) && !glfwGetWindowAttrib(win, GLFW_ICONIFIED);
		if (visible) {
			// Update and render
			nvgBeginFrame(vg, winWidth, winHeight, pixelRatio);

			// nvgReset(vg);
			// nvgScale(vg, pixelRatio, pixelRatio);

			app()->event->rootWidget->draw(vg);

			glViewport(0, 0, fbWidth, fbHeight);
			glClearColor(0.0, 0.0, 0.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			nvgEndFrame(vg);

			glfwSwapBuffers(win);
		}

		// Limit framerate manually if vsync isn't working
		double endTime = glfwGetTime();
		double frameTime = endTime - startTime;
		double minTime = 1 / 120.0;
		if (frameTime < minTime) {
			std::this_thread::sleep_for(std::chrono::duration<double>(minTime - frameTime));
		}
		endTime = glfwGetTime();
		// INFO("%lf fps", 1.0 / (endTime - startTime));
		frame++;
	}
}

void Window::close() {
	glfwSetWindowShouldClose(win, GLFW_TRUE);
}

void Window::cursorLock() {
	if (allowCursorLock) {
#ifdef ARCH_MAC
		glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
#else
		glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#endif
	}
}

void Window::cursorUnlock() {
	if (allowCursorLock) {
		glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

int Window::getMods() {
	int mods = 0;
	if (glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(win, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
		mods |= GLFW_MOD_SHIFT;
	if (glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(win, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
		mods |= GLFW_MOD_CONTROL;
	if (glfwGetKey(win, GLFW_KEY_LEFT_ALT) == GLFW_PRESS || glfwGetKey(win, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS)
		mods |= GLFW_MOD_ALT;
	if (glfwGetKey(win, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS || glfwGetKey(win, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS)
		mods |= GLFW_MOD_SUPER;
	return mods;
}

math::Vec Window::getWindowSize() {
	int width, height;
	glfwGetWindowSize(win, &width, &height);
	return math::Vec(width, height);
}

void Window::setWindowSize(math::Vec size) {
	int width = size.x;
	int height = size.y;
	glfwSetWindowSize(win, width, height);
}

math::Vec Window::getWindowPos() {
	int x, y;
	glfwGetWindowPos(win, &x, &y);
	return math::Vec(x, y);
}

void Window::setWindowPos(math::Vec pos) {
	int x = pos.x;
	int y = pos.y;
	glfwSetWindowPos(win, x, y);
}

bool Window::isMaximized() {
	return glfwGetWindowAttrib(win, GLFW_MAXIMIZED);
}

void Window::setFullScreen(bool fullScreen) {
	if (isFullScreen()) {
		glfwSetWindowMonitor(win, NULL, internal->lastWindowX, internal->lastWindowY, internal->lastWindowWidth, internal->lastWindowHeight, GLFW_DONT_CARE);
	}
	else {
		glfwGetWindowPos(win, &internal->lastWindowX, &internal->lastWindowY);
		glfwGetWindowSize(win, &internal->lastWindowWidth, &internal->lastWindowHeight);
		GLFWmonitor *monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(win, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	}
}

bool Window::isFullScreen() {
	GLFWmonitor *monitor = glfwGetWindowMonitor(win);
	return monitor != NULL;
}


void windowInit() {
	int err;

	// Set up GLFW
	glfwSetErrorCallback(errorCallback);
	err = glfwInit();
	if (err != GLFW_TRUE) {
		osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, "Could not initialize GLFW.");
		exit(1);
	}
}

void windowDestroy() {
	glfwTerminate();
}



} // namespace rack

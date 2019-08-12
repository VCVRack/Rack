#include <window.hpp>
#include <asset.hpp>
#include <app/Scene.hpp>
#include <keyboard.hpp>
#include <gamepad.hpp>
#include <event.hpp>
#include <app.hpp>
#include <patch.hpp>
#include <settings.hpp>
#include <plugin.hpp> // used in Window::screenshot
#include <system.hpp> // used in Window::screenshot

#include <map>
#include <queue>
#include <thread>

#if defined ARCH_MAC
	// For CGAssociateMouseAndMouseCursorPosition
	#include <ApplicationServices/ApplicationServices.h>
#endif

#include <osdialog.h>
#include <stb_image_write.h>


namespace rack {


void Font::loadFile(const std::string& filename, NVGcontext* vg) {
	this->vg = vg;
	handle = nvgCreateFont(vg, filename.c_str(), filename.c_str());
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

std::shared_ptr<Font> Font::load(const std::string& filename) {
	return APP->window->loadFont(filename);
}

void Image::loadFile(const std::string& filename, NVGcontext* vg) {
	this->vg = vg;
	handle = nvgCreateImage(vg, filename.c_str(), NVG_IMAGE_REPEATX | NVG_IMAGE_REPEATY);
	if (handle > 0) {
		INFO("Loaded image %s", filename.c_str());
	}
	else {
		WARN("Failed to load image %s", filename.c_str());
	}
}

Image::~Image() {
	// TODO What if handle is invalid?
	if (handle >= 0)
		nvgDeleteImage(vg, handle);
}

std::shared_ptr<Image> Image::load(const std::string& filename) {
	return APP->window->loadImage(filename);
}

void Svg::loadFile(const std::string& filename) {
	handle = nsvgParseFromFile(filename.c_str(), "px", app::SVG_DPI);
	if (handle) {
		INFO("Loaded SVG %s", filename.c_str());
	}
	else {
		WARN("Failed to load SVG %s", filename.c_str());
	}
}

Svg::~Svg() {
	if (handle)
		nsvgDelete(handle);
}

std::shared_ptr<Svg> Svg::load(const std::string& filename) {
	return APP->window->loadSvg(filename);
}


struct Window::Internal {
	std::string lastWindowTitle;

	int lastWindowX = 0;
	int lastWindowY = 0;
	int lastWindowWidth = 0;
	int lastWindowHeight = 0;

	bool ignoreNextMouseDelta = false;
};


static void windowSizeCallback(GLFWwindow* win, int width, int height) {
	// Do nothing. Window size is reset each frame anyway.
}

static void mouseButtonCallback(GLFWwindow* win, int button, int action, int mods) {
	Window* window = (Window*) glfwGetWindowUserPointer(win);
#if defined ARCH_MAC
	// Remap Ctrl-left click to right click on Mac
	if (button == GLFW_MOUSE_BUTTON_LEFT && (mods & RACK_MOD_MASK) == GLFW_MOD_CONTROL) {
		button = GLFW_MOUSE_BUTTON_RIGHT;
		mods &= ~GLFW_MOD_CONTROL;
	}
	// Remap Ctrl-shift-left click to middle click on Mac
	if (button == GLFW_MOUSE_BUTTON_LEFT && (mods & RACK_MOD_MASK) == (GLFW_MOD_CONTROL | GLFW_MOD_SHIFT)) {
		button = GLFW_MOUSE_BUTTON_MIDDLE;
		mods &= ~(GLFW_MOD_CONTROL | GLFW_MOD_SHIFT);
	}
#endif

	APP->event->handleButton(window->mousePos, button, action, mods);
}

static void cursorPosCallback(GLFWwindow* win, double xpos, double ypos) {
	Window* window = (Window*) glfwGetWindowUserPointer(win);
	math::Vec mousePos = math::Vec(xpos, ypos).div(window->pixelRatio / window->windowRatio).round();
	math::Vec mouseDelta = mousePos.minus(window->mousePos);

	// Workaround for GLFW warping mouse to a different position when the cursor is locked or unlocked.
	if (window->internal->ignoreNextMouseDelta) {
		window->internal->ignoreNextMouseDelta = false;
		mouseDelta = math::Vec();
	}

	int cursorMode = glfwGetInputMode(win, GLFW_CURSOR);
	(void) cursorMode;

#if defined ARCH_MAC
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

	APP->event->handleHover(mousePos, mouseDelta);
}

static void cursorEnterCallback(GLFWwindow* win, int entered) {
	if (!entered) {
		APP->event->handleLeave();
	}
}

static void scrollCallback(GLFWwindow* win, double x, double y) {
	Window* window = (Window*) glfwGetWindowUserPointer(win);
	math::Vec scrollDelta = math::Vec(x, y);
#if defined ARCH_MAC
	scrollDelta = scrollDelta.mult(10.0);
#else
	scrollDelta = scrollDelta.mult(50.0);
#endif

	APP->event->handleScroll(window->mousePos, scrollDelta);
}

static void charCallback(GLFWwindow* win, unsigned int codepoint) {
	Window* window = (Window*) glfwGetWindowUserPointer(win);
	APP->event->handleText(window->mousePos, codepoint);
}

static void keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods) {
	Window* window = (Window*) glfwGetWindowUserPointer(win);
	if (APP->event->handleKey(window->mousePos, key, scancode, action, mods))
		return;

	// Keyboard MIDI driver
	if (action == GLFW_PRESS && (mods & RACK_MOD_MASK) == 0) {
		keyboard::press(key);
	}
	if (action == GLFW_RELEASE) {
		keyboard::release(key);
	}
}

static void dropCallback(GLFWwindow* win, int count, const char** paths) {
	Window* window = (Window*) glfwGetWindowUserPointer(win);
	std::vector<std::string> pathsVec;
	for (int i = 0; i < count; i++) {
		pathsVec.push_back(paths[i]);
	}
	APP->event->handleDrop(window->mousePos, pathsVec);
}

static void errorCallback(int error, const char* description) {
	WARN("GLFW error %d: %s", error, description);
}

Window::Window() {
	internal = new Internal;
	int err;

	// Set window hints
#if defined NANOVG_GL2
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#elif defined NANOVG_GL3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

#if defined ARCH_MAC
	glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
#endif

	// Create window
	win = glfwCreateWindow(800, 600, "", NULL, NULL);
	if (!win) {
		osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, "Could not open GLFW window. Does your graphics card support OpenGL 2.0 or greater? If so, make sure you have the latest graphics drivers installed.");
		exit(1);
	}

	float contentScale;
	glfwGetWindowContentScale(win, &contentScale, NULL);
	INFO("Window content scale: %f", contentScale);

	glfwSetWindowSizeLimits(win, 640, 480, GLFW_DONT_CARE, GLFW_DONT_CARE);
	if (settings::windowSize.isZero()) {
		glfwMaximizeWindow(win);
	}
	else {
		glfwSetWindowPos(win, settings::windowPos.x, settings::windowPos.y);
		glfwSetWindowSize(win, settings::windowSize.x, settings::windowSize.y);
	}
	glfwShowWindow(win);

	glfwSetWindowUserPointer(win, this);
	glfwSetInputMode(win, GLFW_LOCK_KEY_MODS, 1);

	glfwMakeContextCurrent(win);
	// Enable v-sync
	glfwSwapInterval(settings::frameRateSync ? 1 : 0);

	// Set window callbacks
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

	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	INFO("Renderer: %s", renderer);
	INFO("OpenGL: %s", version);

	// GLEW generates GL error because it calls glGetString(GL_EXTENSIONS), we'll consume it here.
	glGetError();

	// Set up NanoVG
	int nvgFlags = NVG_ANTIALIAS;
#if defined NANOVG_GL2
	vg = nvgCreateGL2(nvgFlags);
#elif defined NANOVG_GL3
	vg = nvgCreateGL3(nvgFlags);
#elif defined NANOVG_GLES2
	vg = nvgCreateGLES2(nvgFlags);
#endif
	if (!vg) {
		osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, "Could not initialize NanoVG. Does your graphics card support OpenGL 2.0 or greater? If so, make sure you have the latest graphics drivers installed.");
		exit(1);
	}

	// Load default Blendish font
	uiFont = loadFont(asset::system("res/fonts/DejaVuSans.ttf"));
}

Window::~Window() {
	if (glfwGetWindowAttrib(win, GLFW_MAXIMIZED)) {
		settings::windowSize = math::Vec();
		settings::windowPos = math::Vec();
	}
	else {
		int winWidth, winHeight;
		glfwGetWindowSize(win, &winWidth, &winHeight);
		int winX, winY;
		glfwGetWindowPos(win, &winX, &winY);
		settings::windowSize = math::Vec(winWidth, winHeight);
		settings::windowPos = math::Vec(winX, winY);
	}

#if defined NANOVG_GL2
	nvgDeleteGL2(vg);
#elif defined NANOVG_GL3
	nvgDeleteGL3(vg);
#elif defined NANOVG_GLES2
	nvgDeleteGLES2(vg);
#endif

	glfwDestroyWindow(win);
	delete internal;
}

void Window::run() {
	frame = 0;
	while (!glfwWindowShouldClose(win)) {
		frameTimeStart = glfwGetTime();

		// Make event handlers and step() have a clean nanovg context
		nvgReset(vg);
		bndSetFont(uiFont->handle);

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
		std::string windowTitle = app::APP_NAME + " v" + app::APP_VERSION;
		if (!APP->patch->path.empty()) {
			windowTitle += " - ";
			if (!APP->history->isSaved())
				windowTitle += "*";
			windowTitle += string::filename(APP->patch->path);
		}
		if (windowTitle != internal->lastWindowTitle) {
			glfwSetWindowTitle(win, windowTitle.c_str());
			internal->lastWindowTitle = windowTitle;
		}

		// Get desired scaling
		float newPixelRatio;
		glfwGetWindowContentScale(win, &newPixelRatio, NULL);
		newPixelRatio = std::floor(newPixelRatio + 0.5);
		if (newPixelRatio != pixelRatio) {
			APP->event->handleZoom();
			pixelRatio = newPixelRatio;
		}

		// Get framebuffer/window ratio
		int fbWidth, fbHeight;
		glfwGetFramebufferSize(win, &fbWidth, &fbHeight);
		int winWidth, winHeight;
		glfwGetWindowSize(win, &winWidth, &winHeight);
		windowRatio = (float)fbWidth / winWidth;

		// DEBUG("%f %f %d %d", pixelRatio, windowRatio, fbWidth, winWidth);
		// Resize scene
		APP->scene->box.size = math::Vec(fbWidth, fbHeight).div(pixelRatio);

		// Step scene
		APP->scene->step();

		// Render scene
		bool visible = glfwGetWindowAttrib(win, GLFW_VISIBLE) && !glfwGetWindowAttrib(win, GLFW_ICONIFIED);
		if (visible) {
			// Update and render
			nvgBeginFrame(vg, fbWidth, fbHeight, pixelRatio);
			nvgScale(vg, pixelRatio, pixelRatio);

			// Draw scene
			widget::Widget::DrawArgs args;
			args.vg = vg;
			args.clipBox = APP->scene->box.zeroPos();
			APP->scene->draw(args);

			glViewport(0, 0, fbWidth, fbHeight);
			glClearColor(0.0, 0.0, 0.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			nvgEndFrame(vg);

			glfwSwapBuffers(win);
		}

		// Limit frame rate
		double frameTimeEnd = glfwGetTime();
		if (settings::frameRateLimit > 0.0) {
			double frameDuration = frameTimeEnd - frameTimeStart;
			double waitDuration = 1.0 / settings::frameRateLimit - frameDuration;
			if (waitDuration > 0.0) {
				std::this_thread::sleep_for(std::chrono::duration<double>(waitDuration));
			}
		}

		// Compute actual frame rate
		frameTimeEnd = glfwGetTime();
		// DEBUG("%g fps", 1 / (endTime - startTime));
		frame++;
	}
}

void Window::screenshot(float zoom) {
	// Iterate plugins and create directories
	std::string screenshotsDir = asset::user("screenshots");
	system::createDirectory(screenshotsDir);
	for (plugin::Plugin* p : plugin::plugins) {
		std::string dir = screenshotsDir + "/" + p->slug;
		system::createDirectory(dir);
		for (plugin::Model* model : p->models) {
			std::string filename = dir + "/" + model->slug + ".png";
			// Skip model if screenshot already exists
			if (system::isFile(filename))
				continue;
			INFO("Screenshotting %s %s to %s", p->slug.c_str(), model->slug.c_str(), filename.c_str());

			// Create widgets
			app::ModuleWidget* mw = model->createModuleWidgetNull();
			widget::FramebufferWidget* fb = new widget::FramebufferWidget;
			fb->oversample = 2;
			fb->addChild(mw);
			fb->scale = math::Vec(zoom, zoom);

			// Draw to framebuffer
			frameTimeStart = glfwGetTime();
			fb->step();
			nvgluBindFramebuffer(fb->fb);

			// Read pixels
			int width, height;
			nvgImageSize(vg, fb->getImageHandle(), &width, &height);
			uint8_t* data = new uint8_t[height * width * 4];
			glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

			// Flip image vertically
			for (int y = 0; y < height / 2; y++) {
				int flipY = height - y - 1;
				uint8_t tmp[width * 4];
				memcpy(tmp, &data[y * width * 4], width * 4);
				memcpy(&data[y * width * 4], &data[flipY * width * 4], width * 4);
				memcpy(&data[flipY * width * 4], tmp, width * 4);
			}

			// Write pixels to PNG
			stbi_write_png(filename.c_str(), width, height, 4, data, width * 4);

			// Cleanup
			delete[] data;
			nvgluBindFramebuffer(NULL);
			delete fb;
		}
	}
}

void Window::close() {
	glfwSetWindowShouldClose(win, GLFW_TRUE);
}

void Window::cursorLock() {
	if (settings::allowCursorLock) {
#if defined ARCH_MAC
		glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
#else
		glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#endif
		internal->ignoreNextMouseDelta = true;
	}
}

void Window::cursorUnlock() {
	if (settings::allowCursorLock) {
		glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		internal->ignoreNextMouseDelta = true;
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

void Window::setFullScreen(bool fullScreen) {
	if (!fullScreen) {
		glfwSetWindowMonitor(win, NULL, internal->lastWindowX, internal->lastWindowY, internal->lastWindowWidth, internal->lastWindowHeight, GLFW_DONT_CARE);
	}
	else {
		glfwGetWindowPos(win, &internal->lastWindowX, &internal->lastWindowY);
		glfwGetWindowSize(win, &internal->lastWindowWidth, &internal->lastWindowHeight);
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(win, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	}
}

bool Window::isFullScreen() {
	GLFWmonitor* monitor = glfwGetWindowMonitor(win);
	return monitor != NULL;
}

bool Window::isFrameOverdue() {
	if (settings::frameRateLimit == 0.0)
		return false;
	double frameDuration = glfwGetTime() - frameTimeStart;
	return frameDuration > 1.0 / settings::frameRateLimit;
}

std::shared_ptr<Font> Window::loadFont(const std::string& filename) {
	auto sp = fontCache[filename].lock();
	if (!sp) {
		fontCache[filename] = sp = std::make_shared<Font>();
		sp->loadFile(filename, vg);
	}
	return sp;
}

std::shared_ptr<Image> Window::loadImage(const std::string& filename) {
	auto sp = imageCache[filename].lock();
	if (!sp) {
		imageCache[filename] = sp = std::make_shared<Image>();
		sp->loadFile(filename, vg);
	}
	return sp;
}

std::shared_ptr<Svg> Window::loadSvg(const std::string& filename) {
	auto sp = svgCache[filename].lock();
	if (!sp) {
		svgCache[filename] = sp = std::make_shared<Svg>();
		sp->loadFile(filename);
	}
	return sp;
}


void windowInit() {
	int err;

	// Set up GLFW
#if defined ARCH_MAC
	glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_TRUE);
	glfwInitHint(GLFW_COCOA_MENUBAR, GLFW_TRUE);
#endif

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

#pragma once
#include "common.hpp"
#include "math.hpp"

#include <memory>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <nanovg.h>
#include <nanosvg.h>


/** Remaps Ctrl to Cmd on Mac
Use this instead of GLFW_MOD_CONTROL, since Cmd should be used on Mac in place of Ctrl on Linux/Windows.
*/
#ifdef ARCH_MAC
	#define WINDOW_MOD_CTRL GLFW_MOD_SUPER
	#define WINDOW_MOD_CTRL_NAME "Cmd"
#else
	#define WINDOW_MOD_CTRL GLFW_MOD_CONTROL
	#define WINDOW_MOD_CTRL_NAME "Ctrl"
#endif
#define WINDOW_MOD_SHIFT_NAME "Shift"
#define WINDOW_MOD_ALT_NAME "Alt"

/** Filters actual mod keys from the mod flags.
Use this if you don't care about GLFW_MOD_CAPS_LOCK and GLFW_MOD_NUM_LOCK.
Example usage:
	if ((e.mod & WINDOW_MOD_MASK) == (WINDOW_MOD | GLFW_MOD_SHIFT)) ...
*/
#define WINDOW_MOD_MASK (GLFW_MOD_SHIFT | GLFW_MOD_CONTROL | GLFW_MOD_ALT | GLFW_MOD_SUPER)


namespace rack {


// Constructing these directly will load from the disk each time. Use the load() functions to load from disk and cache them as long as the shared_ptr is held.

struct Font {
	int handle;
	Font(const std::string &filename);
	~Font();
	static std::shared_ptr<Font> load(const std::string &filename);
};

struct Image {
	int handle;
	Image(const std::string &filename);
	~Image();
	static std::shared_ptr<Image> load(const std::string &filename);
};

struct SVG {
	NSVGimage *handle;
	SVG(const std::string &filename);
	~SVG();
	static std::shared_ptr<SVG> load(const std::string &filename);
};

struct Window {
	GLFWwindow *win = NULL;
	NVGcontext *vg = NULL;
	NVGcontext *framebufferVg = NULL;
	/** The scaling ratio */
	float pixelRatio = 1.f;
	/* The ratio between the framebuffer size and the window size reported by the OS.
	This is not equal to gPixelRatio in general.
	*/
	float windowRatio = 1.f;
	bool allowCursorLock = true;
	int frame = 0;
	/** The last known absolute mouse position in the window */
	math::Vec mousePos;
	std::shared_ptr<Font> uiFont;

	struct Internal;
	Internal *internal;

	Window();
	~Window();
	void run();
	void close();
	void cursorLock();
	void cursorUnlock();
	/** Gets the current keyboard mod state
	Don't call this from a Key event. Simply use `e.mods` instead.
	*/
	int getMods();
	math::Vec getWindowSize();
	void setWindowSize(math::Vec size);
	math::Vec getWindowPos();
	void setWindowPos(math::Vec pos);
	bool isMaximized();
	void setFullScreen(bool fullScreen);
	bool isFullScreen();
};


void windowInit();
void windowDestroy();


} // namespace rack

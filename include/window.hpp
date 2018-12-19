#pragma once
#include "common.hpp"
#include "math.hpp"

#include <memory>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <nanovg.h>
#include <nanosvg.h>


#ifdef ARCH_MAC
	#define WINDOW_MOD_KEY_NAME "Cmd"
#else
	#define WINDOW_MOD_KEY_NAME "Ctrl"
#endif


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
	bool isModPressed();
	bool isShiftPressed();
	math::Vec getWindowSize();
	void setWindowSize(math::Vec size);
	math::Vec getWindowPos();
	void setWindowPos(math::Vec pos);
	bool isMaximized();
	void setFullScreen(bool fullScreen);
	bool isFullScreen();
};


} // namespace rack

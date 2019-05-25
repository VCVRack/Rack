#pragma once
#include <common.hpp>
#include <math.hpp>

#include <memory>
#include <map>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <nanovg.h>
#define NANOVG_GL2
#include <nanovg_gl.h>
#include <nanovg_gl_utils.h>
#include <nanosvg.h>


namespace rack {


// Constructing these directly will load from the disk each time. Use the load() functions to load from disk and cache them as long as the shared_ptr is held.

struct Font {
	NVGcontext *vg;
	int handle = -1;
	/** Don't call this directly but instead use `APP->window->loadFont()` */
	void loadFile(const std::string &filename, NVGcontext *vg);
	~Font();
	/** Use `APP->window->loadFont()` instead. */
	DEPRECATED static std::shared_ptr<Font> load(const std::string &filename);
};

struct Image {
	NVGcontext *vg;
	int handle = -1;
	/** Don't call this directly but instead use `APP->window->loadImage()` */
	void loadFile(const std::string &filename, NVGcontext *vg);
	~Image();
	/** Use `APP->window->loadImage()` instead. */
	DEPRECATED static std::shared_ptr<Image> load(const std::string &filename);
};

struct Svg {
	NSVGimage *handle = NULL;
	/** Don't call this directly but instead use `APP->window->loadSvg()` */
	void loadFile(const std::string &filename);
	~Svg();
	/** Use `APP->window->loadSvg()` instead. */
	DEPRECATED static std::shared_ptr<Svg> load(const std::string &filename);
};

DEPRECATED typedef Svg SVG;


struct Window {
	GLFWwindow *win = NULL;
	NVGcontext *vg = NULL;
	/** The scaling ratio */
	float pixelRatio = 1.f;
	/* The ratio between the framebuffer size and the window size reported by the OS.
	This is not equal to gPixelRatio in general.
	*/
	float windowRatio = 1.f;
	int frame = 0;
	/** The last known absolute mouse position in the window */
	math::Vec mousePos;
	std::shared_ptr<Font> uiFont;
	double frameTimeStart = 0.f;

	/** Use load*() instead of modifying these directly. */
	std::map<std::string, std::weak_ptr<Font>> fontCache;
	std::map<std::string, std::weak_ptr<Image>> imageCache;
	std::map<std::string, std::weak_ptr<Svg>> svgCache;

	struct Internal;
	Internal *internal;

	Window();
	~Window();
	void run();
	/** Takes a screenshot of each module */
	void screenshot(float zoom);
	void close();
	void cursorLock();
	void cursorUnlock();
	/** Gets the current keyboard mod state
	Don't call this from a Key event. Simply use `e.mods` instead.
	*/
	int getMods();
	void setFullScreen(bool fullScreen);
	bool isFullScreen();
	bool isFrameOverdue();

	std::shared_ptr<Font> loadFont(const std::string &filename);
	std::shared_ptr<Image> loadImage(const std::string &filename);
	std::shared_ptr<Svg> loadSvg(const std::string &filename);
};


void windowInit();
void windowDestroy();


} // namespace rack

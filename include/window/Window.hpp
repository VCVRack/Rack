#pragma once

#include <memory>
#include <map>

#include <common.hpp>
#include <math.hpp>
#include <window/Svg.hpp>

#define GLEW_STATIC
#define GLEW_NO_GLU
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <nanovg.h>
#define NANOVG_GL2
#include <nanovg_gl.h>
#include <nanovg_gl_utils.h>


namespace rack {
/** Handles OS windowing, OpenGL, and NanoVG
*/
namespace window {


// Constructing these directly will load from the disk each time. Use the load() functions to load from disk and cache them as long as the shared_ptr is held.

/** Text font loaded in a particular Window context. */
struct Font {
	NVGcontext* vg;
	int handle = -1;
	/** Don't call this directly but instead use `APP->window->loadFont()` */
	void loadFile(const std::string& filename, NVGcontext* vg);
	~Font();
	/** Use `APP->window->loadFont()` instead. */
	DEPRECATED static std::shared_ptr<Font> load(const std::string& filename);
};


/** Bitmap/raster image loaded in a particular Window context. */
struct Image {
	NVGcontext* vg;
	int handle = -1;
	/** Don't call this directly but instead use `APP->window->loadImage()` */
	void loadFile(const std::string& filename, NVGcontext* vg);
	~Image();
	/** Use `APP->window->loadImage()` instead. */
	DEPRECATED static std::shared_ptr<Image> load(const std::string& filename);
};


/** OS window with OpenGL context. */
struct Window {
	struct Internal;
	Internal* internal;

	GLFWwindow* win = NULL;
	NVGcontext* vg = NULL;
	NVGcontext* fbVg = NULL;
	/** The scaling ratio */
	float pixelRatio = 1.f;
	/* The ratio between the framebuffer size and the window size reported by the OS.
	This is not equal to gPixelRatio in general.
	*/
	float windowRatio = 1.f;
	std::shared_ptr<Font> uiFont;

	PRIVATE Window();
	PRIVATE ~Window();
	math::Vec getSize();
	void setSize(math::Vec size);
	PRIVATE void run();
	PRIVATE void step();
	void activateContext();
	/** Takes a screenshot of the screen and saves it to a PNG file. */
	PRIVATE void screenshot(const std::string& screenshotPath);
	/** Saves a PNG image of all modules to `screenshotsDir/<plugin slug>/<module slug>.png`.
	Skips screenshot if the file already exists.
	*/
	PRIVATE void screenshotModules(const std::string& screenshotsDir, float zoom = 1.f);
	/** Request Window to be closed after rendering the current frame. */
	void close();
	void cursorLock();
	void cursorUnlock();
	bool isCursorLocked();
	/** Gets the current keyboard mod state
	Don't call this from a Key event. Simply use `e.mods` instead.
	*/
	int getMods();
	void setFullScreen(bool fullScreen);
	bool isFullScreen();
	double getMonitorRefreshRate();
	double getFrameTime();
	double getLastFrameDuration();
	double getFrameDurationRemaining();

	std::shared_ptr<Font> loadFont(const std::string& filename);
	std::shared_ptr<Image> loadImage(const std::string& filename);
	std::shared_ptr<Svg> loadSvg(const std::string& filename) {
		return Svg::load(filename);
	}

	PRIVATE bool& fbDirtyOnSubpixelChange();
};


PRIVATE void init();
PRIVATE void destroy();


} // namespace window
} // namespace rack

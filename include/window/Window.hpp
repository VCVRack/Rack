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

	~Font();
	/** Don't call this directly but instead use `APP->window->loadFont()` */
	void loadFile(const std::string& filename, NVGcontext* vg);
	/** Use `APP->window->loadFont()` instead. */
	DEPRECATED static std::shared_ptr<Font> load(const std::string& filename);
};


/** Bitmap/raster image loaded in a particular Window context. */
struct Image {
	NVGcontext* vg;
	int handle = -1;

	~Image();
	/** Don't call this directly but instead use `APP->window->loadImage()` */
	void loadFile(const std::string& filename, NVGcontext* vg);
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
	/** UI scaling ratio */
	float pixelRatio = 1.f;
	/** Ratio between the framebuffer size and the window size reported by the OS.
	This is not equal to pixelRatio in general.
	*/
	float windowRatio = 1.f;
	std::shared_ptr<Font> uiFont;

	PRIVATE Window();
	PRIVATE ~Window();
	math::Vec getSize();
	void setSize(math::Vec size);
	PRIVATE void run();
	PRIVATE void step();
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
	/** Returns the primary monitor's refresh rate in Hz. */
	double getMonitorRefreshRate();
	/** Returns the timestamp of the beginning of the current frame render process.
	Returns NAN if no frames have begun rendering.
	*/
	double getFrameTime();
	/** Returns the total time in seconds spent rendering the last frame.
	Returns NAN if no frames have ended rendering.
	*/
	double getLastFrameDuration();
	/** Returns the current time remaining in seconds until the frame deadline, according to the frame rate limit. */
	double getFrameDurationRemaining();

	/** Loads and caches a Font from a file path.
	Do not store this reference across screen frames, as the Window may have changed, invalidating the Font.
	*/
	std::shared_ptr<Font> loadFont(const std::string& filename);
	/** Loads and caches an Image from a file path.
	Do not store this reference across screen frames, as the Window may have changed, invalidating the Image.
	*/
	std::shared_ptr<Image> loadImage(const std::string& filename);
	/** Loads and caches an Svg from a file path.
	Alias for `Svg::load()`.
	*/
	std::shared_ptr<Svg> loadSvg(const std::string& filename) {
		return Svg::load(filename);
	}

	PRIVATE bool& fbDirtyOnSubpixelChange();
	PRIVATE int& fbCount();
};


PRIVATE void init();
PRIVATE void destroy();


} // namespace window
} // namespace rack

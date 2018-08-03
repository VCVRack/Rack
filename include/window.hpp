#pragma once
#include <memory>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "nanovg.h"
#include "nanosvg.h"

#include "common.hpp"


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


extern GLFWwindow *gWindow;
extern NVGcontext *gVg;
extern NVGcontext *gFramebufferVg;
/** The default font to use for GUI elements */
extern std::shared_ptr<Font> gGuiFont;
/** The scaling ratio */
extern float gPixelRatio;
/* The ratio between the framebuffer size and the window size reported by the OS.
This is not equal to gPixelRatio in general.
*/
extern float gWindowRatio;
extern bool gAllowCursorLock;
extern int gGuiFrame;
extern math::Vec gMousePos;


void windowInit();
void windowDestroy();
void windowRun();
void windowClose();
void windowCursorLock();
void windowCursorUnlock();
bool windowIsModPressed();
bool windowIsShiftPressed();
math::Vec windowGetWindowSize();
void windowSetWindowSize(math::Vec size);
math::Vec windowGetWindowPos();
void windowSetWindowPos(math::Vec pos);
bool windowIsMaximized();
void windowSetTheme(NVGcolor bg, NVGcolor fg);
void windowSetFullScreen(bool fullScreen);
bool windowGetFullScreen();


} // namespace rack

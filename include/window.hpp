#pragma once
#include "widgets.hpp"
#define GLEW_STATIC
#include <GL/glew.h>
////#include <GLFW/glfw3.h>
#include <lglw/lglw.h>


#ifdef ARCH_MAC
	#define WINDOW_MOD_KEY_NAME "Cmd"
#else
	#define WINDOW_MOD_KEY_NAME "Ctrl"
#endif


// backwards compatibility:
//   (note) currently only used by Bidoo.ACNE module
#define RACK_MOUSE_BUTTON_LEFT   0
#define RACK_MOUSE_BUTTON_RIGHT  1
#define RACK_MOUSE_BUTTON_MIDDLE 2


extern const char *g_program_dir;

namespace rack {


// extern GLFWwindow *gWindow;
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
extern Vec gMousePos;


void windowInit();
void windowDestroy();
void windowRun();
void windowClose();
void windowCursorLock();
void windowCursorUnlock();
bool windowIsModPressed();
bool windowIsShiftPressed();
Vec windowGetWindowSize();
void windowSetWindowSize(Vec size);
Vec windowGetWindowPos();
void windowSetWindowPos(Vec pos);
bool windowIsMaximized();
void windowSetTheme(NVGcolor bg, NVGcolor fg);
void windowSetFullScreen(bool fullScreen);
bool windowGetFullScreen();


} // namespace rack

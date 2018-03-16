#pragma once
#include "widgets.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>


#ifdef ARCH_MAC
	#define WINDOW_MOD_KEY_NAME "Cmd"
#else
	#define WINDOW_MOD_KEY_NAME "Ctrl"
#endif


namespace rack {


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


} // namespace rack

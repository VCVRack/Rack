#pragma once
#include "app.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>


#ifdef ARCH_MAC
	#define GUI_MOD_KEY_NAME "Cmd"
#else
	#define GUI_MOD_KEY_NAME "Ctrl"
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


void guiInit();
void guiDestroy();
void guiRun();
void guiClose();
void guiCursorLock();
void guiCursorUnlock();
bool guiIsModPressed();
bool guiIsShiftPressed();
Vec guiGetWindowSize();
void guiSetWindowSize(Vec size);
Vec guiGetWindowPos();
void guiSetWindowPos(Vec pos);
bool guiIsMaximized();


} // namespace rack

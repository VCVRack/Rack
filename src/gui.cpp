#include <unistd.h>
#include "Rack.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// #define NANOVG_GLEW
#define NANOVG_IMPLEMENTATION
#include "../lib/nanovg/src/nanovg.h"
#define NANOVG_GL2_IMPLEMENTATION
#include "../lib/nanovg/src/nanovg_gl.h"
#define BLENDISH_IMPLEMENTATION
#include "../lib/oui/blendish.h"

extern "C" {
	#include "../lib/noc/noc_file_dialog.h"
}


namespace rack {

Scene *gScene = NULL;
RackWidget *gRackWidget = NULL;

Vec gMousePos;
Widget *gHoveredWidget = NULL;
Widget *gDraggedWidget = NULL;
Widget *gSelectedWidget = NULL;

int gGuiFrame;

static GLFWwindow *window = NULL;
static NVGcontext *vg = NULL;


void windowSizeCallback(GLFWwindow* window, int width, int height) {
	gScene->box.size = Vec(width, height);
}

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		// onMouseDown
		Widget *w = gScene->onMouseDown(gMousePos, button);
		gSelectedWidget = w;

		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			gDraggedWidget = w;
			if (gDraggedWidget) {
				// onDragStart
				gDraggedWidget->onDragStart();
			}
		}
	}
	else if (action == GLFW_RELEASE) {
		// onMouseUp
		Widget *w = gScene->onMouseUp(gMousePos, button);

		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (gDraggedWidget) {
				// onDragDrop
				w->onDragDrop(gDraggedWidget);
			}
			// gDraggedWidget might have been set to null in the last event, recheck here
			if (gDraggedWidget) {
				// onDragEnd
				gDraggedWidget->onDragEnd();
			}
			gDraggedWidget = NULL;
		}
	}
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
	Vec mousePos = Vec(xpos, ypos).round();
	Vec mouseRel = mousePos.minus(gMousePos);
	gMousePos = mousePos;

	if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
		// TODO Lock gMousePos
	}

	// onScroll
	// int middleButton = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);
	// if (middleButton == GLFW_PRESS) {
	// 	gScene->scrollWidget->onScroll(mouseRel.neg());
	// }

	if (gDraggedWidget) {
		// onDragMove
		// Drag slower if Ctrl is held
		bool fine = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
		float factor = fine ? 1.0/8.0 : 1.0;
		gDraggedWidget->onDragMove(mouseRel.mult(factor));
	}
	else {
		// onMouseMove
		Widget *hovered = gScene->onMouseMove(gMousePos, mouseRel);

		if (hovered != gHoveredWidget) {
			if (gHoveredWidget) {
				// onMouseLeave
				gHoveredWidget->onMouseLeave();
			}
			if (hovered) {
			// onMouseEnter
				hovered->onMouseEnter();
			}
		}
		gHoveredWidget = hovered;
	}
}

void cursorEnterCallback(GLFWwindow* window, int entered) {
	if (!entered) {
		if (gHoveredWidget) {
			gHoveredWidget->onMouseLeave();
		}
		gHoveredWidget = NULL;
	}
}

void scrollCallback(GLFWwindow *window, double x, double y) {
	Vec scrollRel = Vec(x, y);
	// onScroll
	gScene->onScroll(gMousePos, scrollRel.mult(-95));
}

void charCallback(GLFWwindow *window, unsigned int value) {
}

static int lastWindowX, lastWindowY, lastWindowWidth, lastWindowHeight;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_F11 || key == GLFW_KEY_ESCAPE) {
			// Toggle fullscreen
			GLFWmonitor *monitor = glfwGetWindowMonitor(window);
			if (monitor) {
				// Window mode
				glfwSetWindowMonitor(window, NULL, lastWindowX, lastWindowY, lastWindowWidth, lastWindowHeight, 0);
			}
			else {
				// Fullscreen
				glfwGetWindowPos(window, &lastWindowX, &lastWindowY);
				glfwGetWindowSize(window, &lastWindowWidth, &lastWindowHeight);
				monitor = glfwGetPrimaryMonitor();
				assert(monitor);
				const GLFWvidmode *mode = glfwGetVideoMode(monitor);
				glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
			}
		}
	}
}

void renderGui() {
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	// glfwGetWindowSize(window, &width, &height);

	// Update and render
	glViewport(0, 0, width, height);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	nvgBeginFrame(vg, width, height, 1.0);

	gScene->draw(vg);
	nvgEndFrame(vg);
	glfwSwapBuffers(window);
}

void guiInit() {
	int err;

	// Set up GLFW
	err = glfwInit();
	assert(err);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	window = glfwCreateWindow(1020, 700, gApplicationName.c_str(), NULL, NULL);
	assert(window);
	glfwMakeContextCurrent(window);

	glfwSwapInterval(1);

	glfwSetWindowSizeCallback(window, windowSizeCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	// glfwSetCursorPosCallback(window, cursorPosCallback);
	glfwSetCursorEnterCallback(window, cursorEnterCallback);
	glfwSetScrollCallback(window, scrollCallback);
	glfwSetCharCallback(window, charCallback);
	glfwSetKeyCallback(window, keyCallback);

	// Set up GLEW
	glewExperimental = GL_TRUE;
	err = glewInit();
	assert(err == GLEW_OK);
	// GLEW generates GL error because it calls glGetString(GL_EXTENSIONS), we'll consume it here.
	glGetError();

	glfwSetWindowSizeLimits(window, 240, 160, GLFW_DONT_CARE, GLFW_DONT_CARE);

	// Set up NanoVG
	vg = nvgCreateGL2(NVG_ANTIALIAS);
	assert(vg);

	// Set up Blendish
	bndSetFont(loadFont("res/DejaVuSans.ttf"));
	// bndSetIconImage(loadImage("res/icons.png"));

	gScene = new Scene();
}

void guiDestroy() {
	delete gScene;

	nvgDeleteGL2(vg);
	glfwDestroyWindow(window);
	glfwTerminate();
}

void guiRun() {
	assert(window);
	{
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		windowSizeCallback(window, width, height);
	}
	gGuiFrame = 0;
	double lastTime = 0.0;
	while(!glfwWindowShouldClose(window)) {
		gGuiFrame++;
		glfwPollEvents();
		{
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			cursorPosCallback(window, xpos, ypos);
		}
		gScene->step();
		renderGui();

		double currTime = glfwGetTime();
		// printf("%lf fps\n", 1.0/(currTime - lastTime));
		lastTime = currTime;
		(void) lastTime;
	}
}

void guiCursorLock() {
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void guiCursorUnlock() {
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

const char *guiSaveDialog(const char *filters, const char *filename) {
	return noc_file_dialog_open(NOC_FILE_DIALOG_SAVE, filters, NULL, filename);
}

const char *guiOpenDialog(const char *filters, const char *filename) {
	return noc_file_dialog_open(NOC_FILE_DIALOG_OPEN, filters, NULL, filename);
}





std::map<std::string, int> images;
std::map<std::string, int> fonts;

int loadImage(std::string filename) {
	assert(vg);
	int imageId;
	auto it = images.find(filename);
	if (it == images.end()) {
		// Load image
		imageId = nvgCreateImage(vg, filename.c_str(), NVG_IMAGE_REPEATX | NVG_IMAGE_REPEATY);
		if (imageId == 0) {
			printf("Failed to load image %s\n", filename.c_str());
		}
		else {
			printf("Loaded image %s\n", filename.c_str());
		}
		images[filename] = imageId;
	}
	else {
		imageId = it->second;
	}
	return imageId;
}

int loadFont(std::string filename) {
	assert(vg);
	int fontId;
	auto it = fonts.find(filename);
	if (it == fonts.end()) {
		fontId = nvgCreateFont(vg, filename.c_str(), filename.c_str());
		if (fontId < 0) {
			printf("Failed to load font %s\n", filename.c_str());
		}
		else {
			printf("Loaded font %s\n", filename.c_str());
		}
		fonts[filename] = fontId;
	}
	else {
		fontId = it->second;
	}
	return fontId;
}


void drawImage(NVGcontext *vg, Vec pos, int imageId) {
	int width, height;
	nvgImageSize(vg, imageId, &width, &height);
	NVGpaint paint = nvgImagePattern(vg, pos.x, pos.y, width, height, 0, imageId, 1.0);
	nvgFillPaint(vg, paint);
	nvgRect(vg, pos.x, pos.y, width, height);
	nvgFill(vg);
}


} // namespace rack

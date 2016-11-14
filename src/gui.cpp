#include "5V.hpp"
#include <unistd.h>

// #define NANOVG_GLEW
#define NANOVG_IMPLEMENTATION
#include "../lib/nanovg/src/nanovg.h"
#define NANOVG_GL2_IMPLEMENTATION
#include "../lib/nanovg/src/nanovg_gl.h"
#define BLENDISH_IMPLEMENTATION
#include "../lib/oui/blendish.h"


static GLFWwindow *window;
static NVGcontext *vg = NULL;

Vec gMousePos;
Widget *gHoveredWidget = NULL;
Widget *gDraggedWidget = NULL;


void windowSizeCallback(GLFWwindow* window, int width, int height) {
	gScene->box.size = Vec(width, height);
	gScene->onResize();
}

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
	if (gHoveredWidget) {
		// onMouseDown and onMouseUp
		if (action == GLFW_PRESS) {
			gHoveredWidget->onMouseDown(button);
		}
		else if (action == GLFW_RELEASE) {
			gHoveredWidget->onMouseUp(button);
		}
	}

	// onDragStart, onDragEnd, and onDragDrop
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			if (gHoveredWidget) {
				gDraggedWidget = gHoveredWidget;
				gDraggedWidget->onDragStart();
			}
		}
		else if (action == GLFW_RELEASE) {
			if (gDraggedWidget) {
				Widget *dropped = gScene->pick(gMousePos);
				if (dropped) {
					dropped->onDragDrop(gDraggedWidget);
				}
				gDraggedWidget->onDragEnd();
				gDraggedWidget = NULL;
			}
		}
	}
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
	Vec mousePos = Vec(xpos, ypos);
	Vec mouseRel = mousePos.minus(gMousePos);
	gMousePos = mousePos;

	if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
		// TODO Lock gMousePos
	}

	// onScroll
	int middleButton = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);
	if (middleButton == GLFW_PRESS) {
		gScene->scrollWidget->onScroll(mouseRel.neg());
	}

	Widget *hovered = gScene->pick(mousePos);

	if (gDraggedWidget) {
		// onDragMove
		// Drag slower if Ctrl is held
		bool fine = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL ) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
		float factor = fine ? 0.1 : 1.0;
		gDraggedWidget->onDragMove(mouseRel.mult(factor));
		// onDragHover
		if (hovered) {
			hovered->onDragHover(gDraggedWidget);
		}
	}
	else {
		// onMouseEnter and onMouseLeave
		if (hovered != gHoveredWidget) {
			if (gHoveredWidget) {
				gHoveredWidget->onMouseLeave();
			}
			if (hovered) {
				hovered->onMouseEnter();
			}
		}
		gHoveredWidget = hovered;

		// onMouseMove
		if (hovered) {
			hovered->onMouseMove(mouseRel);
		}
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
	gScene->scrollWidget->onScroll(scrollRel.mult(-95));
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
  window = glfwCreateWindow(1020, 700, "5V", NULL, NULL);
	assert(window);
	glfwMakeContextCurrent(window);

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


	// Set up NanoVG
	vg = nvgCreateGL2(NVG_ANTIALIAS);
	assert(vg);

	// Set up Blendish
	bndSetFont(loadFont("res/DejaVuSans.ttf"));
	bndSetIconImage(loadImage("res/blender_icons16.png"));
}

void guiDestroy() {
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
	double lastTime = 0.0;
	while(!glfwWindowShouldClose(window)) {
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

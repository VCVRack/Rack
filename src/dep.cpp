// This source file compiles those annoying implementation-in-header libraries

#define GLEW_STATIC
#include <GL/glew.h>
#include <nanovg.h>
#define NANOVG_GL2_IMPLEMENTATION
// #define NANOVG_GL3_IMPLEMENTATION
// #define NANOVG_GLES2_IMPLEMENTATION
// #define NANOVG_GLES3_IMPLEMENTATION
#include <nanovg_gl.h>
// Hack to get framebuffer objects working on OpenGL 2 (we blindly assume the extension is supported)
#define NANOVG_FBO_VALID
#include <nanovg_gl_utils.h>
#define BLENDISH_IMPLEMENTATION
#include <blendish.h>
#define NANOSVG_IMPLEMENTATION
#define NANOSVG_ALL_COLOR_KEYWORDS
#include <nanosvg.h>

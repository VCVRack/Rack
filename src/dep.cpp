// This source file compiles those annoying implementation-in-header libraries

#include <common.hpp> // for fopen_u8

#define GLEW_STATIC
#define GLEW_NO_GLU
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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

// Public Domain. See "unlicense" statement at the end of this file.

// ABOUT
//
// dr_2d is a simple library for drawing simple 2D graphics.
//
//
//
// USAGE
//
// This is a single-file library. To use it, do something like the following in one .c file.
//   #define DR_2D_IMPLEMENTATION
//   #include "dr_2d.h"
//
// You can then #include dr_2d.h in other parts of the program as you would with any other header file.
//
//
//
// QUICK NOTES
//
// - Drawing must be done inside a dr2d_begin_draw() and dr2d_end_draw() pair. Rationale: 1) required for compatibility
//   with GDI's BeginPaint() and EndPaint() APIs; 2) gives implementations opportunity to save and restore state, such as
//   OpenGL state and whatnot.
// - This library is not thread safe.
//
//
//
// OPTIONS
//
// #define DR2D_NO_GDI
//   Excludes the GDI back-end.
//
// #define DR2D_NO_CAIRO
//   Excludes the Cairo back-end.
//
//
//
// TODO
// - Document resource management.

#ifndef dr_2d_h
#define dr_2d_h

#include <stdlib.h>
#include <stdbool.h>

#if defined(__WIN32__) || defined(_WIN32) || defined(_WIN64)
#include <windows.h>

// No Cairo on Win32 builds.
#ifndef DR2D_NO_CAIRO
#define DR2D_NO_CAIRO
#endif
#else
// No GDI on non-Win32 builds.
#ifndef DR2D_NO_GDI
#define DR2D_NO_GDI
#endif
#endif

#ifndef DR2D_MAX_FONT_FAMILY_LENGTH
#define DR2D_MAX_FONT_FAMILY_LENGTH   128
#endif


#ifdef __cplusplus
extern "C" {
#endif


/////////////////////////////////////////////////////////////////
//
// CORE 2D API
//
/////////////////////////////////////////////////////////////////

typedef unsigned char dr2d_byte;

typedef struct dr2d_context dr2d_context;
typedef struct dr2d_surface dr2d_surface;
typedef struct dr2d_font dr2d_font;
typedef struct dr2d_image dr2d_image;
typedef struct dr2d_color dr2d_color;
typedef struct dr2d_font_metrics dr2d_font_metrics;
typedef struct dr2d_glyph_metrics dr2d_glyph_metrics;
typedef struct dr2d_drawing_callbacks dr2d_drawing_callbacks;


/// Structure representing an RGBA color. Color components are specified in the range of 0 - 255.
struct dr2d_color
{
    dr2d_byte r;
    dr2d_byte g;
    dr2d_byte b;
    dr2d_byte a;
};

struct dr2d_font_metrics
{
    int ascent;
    int descent;
    int lineHeight;
    int spaceWidth;
};

struct dr2d_glyph_metrics
{
    int width;
    int height;
    int originX;
    int originY;
    int advanceX;
    int advanceY;
};

typedef enum
{
    dr2d_font_weight_medium = 0,
    dr2d_font_weight_thin,
    dr2d_font_weight_extra_light,
    dr2d_font_weight_light,
    dr2d_font_weight_semi_light,
    dr2d_font_weight_book,
    dr2d_font_weight_semi_bold,
    dr2d_font_weight_bold,
    dr2d_font_weight_extra_bold,
    dr2d_font_weight_heavy,
    dr2d_font_weight_extra_heavy,

    dr2d_font_weight_normal  = dr2d_font_weight_medium,
    dr2d_font_weight_default = dr2d_font_weight_medium

} dr2d_font_weight;

typedef enum
{
    dr2d_font_slant_none = 0,
    dr2d_font_slant_italic,
    dr2d_font_slant_oblique

} dr2d_font_slant;

typedef enum
{
    dr2d_image_format_rgba8,
    dr2d_image_format_bgra8,
    dr2d_image_format_argb8,
} dr2d_image_format;


#define DR2D_IMAGE_DRAW_BACKGROUND      (1 << 0)
#define DR2D_IMAGE_HINT_NO_ALPHA        (1 << 1)

#define DR2D_READ                       (1 << 0)
#define DR2D_WRITE                      (1 << 1)

#define DR2D_FONT_NO_CLEARTYPE          (1 << 0)

typedef struct
{
    /// The destination position on the x axis. This is ignored if the DR2D_IMAGE_ALIGN_CENTER option is set.
    float dstX;

    /// The destination position on the y axis. This is ignored if the DR2D_IMAGE_ALIGN_CENTER option is set.
    float dstY;

    /// The destination width.
    float dstWidth;

    /// The destination height.
    float dstHeight;


    /// The source offset on the x axis.
    float srcX;

    /// The source offset on the y axis.
    float srcY;

    /// The source width.
    float srcWidth;

    /// The source height.
    float srcHeight;


    /// The foreground tint color. This is not applied to the background color, and the alpha component is ignored.
    dr2d_color foregroundTint;

    /// The background color. Only used if the DR2D_IMAGE_DRAW_BACKGROUND option is set.
    dr2d_color backgroundColor;


    /// Flags for controlling how the image should be drawn.
    unsigned int options;

} dr2d_draw_image_args;


typedef bool              (* dr2d_on_create_context_proc)                  (dr2d_context* pContext, const void* pUserData);
typedef void              (* dr2d_on_delete_context_proc)                  (dr2d_context* pContext);
typedef bool              (* dr2d_on_create_surface_proc)                  (dr2d_surface* pSurface, float width, float height);
typedef void              (* dr2d_on_delete_surface_proc)                  (dr2d_surface* pSurface);
typedef bool              (* dr2d_on_create_font_proc)                     (dr2d_font* pFont);
typedef void              (* dr2d_on_delete_font_proc)                     (dr2d_font* pFont);
typedef bool              (* dr2d_on_create_image_proc)                    (dr2d_image* pImage, unsigned int stride, const void* pData);
typedef void              (* dr2d_on_delete_image_proc)                    (dr2d_image* pImage);
typedef void              (* dr2d_begin_draw_proc)                         (dr2d_surface* pSurface);
typedef void              (* dr2d_end_draw_proc)                           (dr2d_surface* pSurface);
typedef void              (* dr2d_clear_proc)                              (dr2d_surface* pSurface, dr2d_color color);
typedef void              (* dr2d_draw_rect_proc)                          (dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color);
typedef void              (* dr2d_draw_rect_outline_proc)                  (dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float outlineWidth);
typedef void              (* dr2d_draw_rect_with_outline_proc)             (dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float outlineWidth, dr2d_color outlineColor);
typedef void              (* dr2d_draw_round_rect_proc)                    (dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float width);
typedef void              (* dr2d_draw_round_rect_outline_proc)            (dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float width, float outlineWidth);
typedef void              (* dr2d_draw_round_rect_with_outline_proc)       (dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float width, float outlineWidth, dr2d_color outlineColor);
typedef void              (* dr2d_draw_text_proc)                          (dr2d_surface* pSurface, dr2d_font* pFont, const char* text, size_t textSizeInBytes, float posX, float posY, dr2d_color color, dr2d_color backgroundColor);
typedef void              (* dr2d_draw_image_proc)                         (dr2d_surface* pSurface, dr2d_image* pImage, dr2d_draw_image_args* pArgs);
typedef void              (* dr2d_set_clip_proc)                           (dr2d_surface* pSurface, float left, float top, float right, float bottom);
typedef void              (* dr2d_get_clip_proc)                           (dr2d_surface* pSurface, float* pLeftOut, float* pTopOut, float* pRightOut, float* pBottomOut);
typedef dr2d_image_format (* dr2d_get_optimal_image_format_proc)           (dr2d_context* pContext);
typedef void*             (* dr2d_map_image_data_proc)                     (dr2d_image* pImage, unsigned int accessFlags);
typedef void              (* dr2d_unmap_image_data_proc)                   (dr2d_image* pImage);
typedef bool              (* dr2d_get_font_metrics_proc)                   (dr2d_font* pFont, dr2d_font_metrics* pMetricsOut);
typedef bool              (* dr2d_get_glyph_metrics_proc)                  (dr2d_font* pFont, unsigned int utf32, dr2d_glyph_metrics* pMetricsOut);
typedef bool              (* dr2d_measure_string_proc)                     (dr2d_font* pFont, const char* text, size_t textSizeInBytes, float* pWidthOut, float* pHeightOut);
typedef bool              (* dr2d_get_text_cursor_position_from_point_proc)(dr2d_font* pFont, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosXOut, size_t* pCharacterIndexOut);
typedef bool              (* dr2d_get_text_cursor_position_from_char_proc) (dr2d_font* pFont, const char* text, size_t characterIndex, float* pTextCursorPosXOut);


struct dr2d_drawing_callbacks
{
    dr2d_on_create_context_proc on_create_context;
    dr2d_on_delete_context_proc on_delete_context;
    dr2d_on_create_surface_proc on_create_surface;
    dr2d_on_delete_surface_proc on_delete_surface;
    dr2d_on_create_font_proc    on_create_font;
    dr2d_on_delete_font_proc    on_delete_font;
    dr2d_on_create_image_proc   on_create_image;
    dr2d_on_delete_image_proc   on_delete_image;

    dr2d_begin_draw_proc                   begin_draw;
    dr2d_end_draw_proc                     end_draw;
    dr2d_clear_proc                        clear;
    dr2d_draw_rect_proc                    draw_rect;
    dr2d_draw_rect_outline_proc            draw_rect_outline;
    dr2d_draw_rect_with_outline_proc       draw_rect_with_outline;
    dr2d_draw_round_rect_proc              draw_round_rect;
    dr2d_draw_round_rect_outline_proc      draw_round_rect_outline;
    dr2d_draw_round_rect_with_outline_proc draw_round_rect_with_outline;
    dr2d_draw_text_proc                    draw_text;
    dr2d_draw_image_proc                   draw_image;
    dr2d_set_clip_proc                     set_clip;
    dr2d_get_clip_proc                     get_clip;

    dr2d_get_optimal_image_format_proc     get_optimal_image_format;
    dr2d_map_image_data_proc               map_image_data;
    dr2d_unmap_image_data_proc             unmap_image_data;

    dr2d_get_font_metrics_proc                    get_font_metrics;
    dr2d_get_glyph_metrics_proc                   get_glyph_metrics;
    dr2d_measure_string_proc                      measure_string;
    dr2d_get_text_cursor_position_from_point_proc get_text_cursor_position_from_point;
    dr2d_get_text_cursor_position_from_char_proc  get_text_cursor_position_from_char;
};

struct dr2d_image
{
    /// A pointer to the context that owns the image.
    dr2d_context* pContext;

    /// The width of the image.
    unsigned int width;

    /// The height of the image.
    unsigned int height;

    /// The format of the image data.
    dr2d_image_format format;

    /// Whether or not the image's data is already mapped.
    bool isMapped;

    /// The extra bytes. The size of this buffer is equal to pContext->imageExtraBytes.
    dr2d_byte pExtraData[1];
};

struct dr2d_font
{
    /// A pointer to the context that owns the font.
    dr2d_context* pContext;

    /// The font family.
    char family[DR2D_MAX_FONT_FAMILY_LENGTH];

    /// The size of the font.
    unsigned int size;

    /// The font's weight.
    dr2d_font_weight weight;

    /// The font's slant.
    dr2d_font_slant slant;

    /// The font's rotation, in degrees.
    float rotation;

    /// Flags. Can be a combination of the following.
    ///   DR2D_FONT_NO_CLEARTYPE
    unsigned int flags;

    /// The extra bytes. The size of this buffer is equal to pContext->fontExtraBytes.
    dr2d_byte pExtraData[1];
};

struct dr2d_surface
{
    /// A pointer to the context that owns the surface.
    dr2d_context* pContext;

    /// The width of the surface.
    float width;

    /// The height of the surface.
    float height;

    /// The extra bytes. The size of this buffer is equal to pContext->surfaceExtraBytes.
    dr2d_byte pExtraData[1];
};

struct dr2d_context
{
    /// The drawing callbacks.
    dr2d_drawing_callbacks drawingCallbacks;

    /// The number of extra bytes to allocate for each image.
    size_t imageExtraBytes;

    /// The number of extra bytes to allocate for each font.
    size_t fontExtraBytes;

    /// The number of extra bytes to allocate for each surface.
    size_t surfaceExtraBytes;

    /// The number of extra bytes to allocate for the context.
    size_t contextExtraBytes;

    /// The extra bytes.
    dr2d_byte pExtraData[1];
};



/// Creats a context.
dr2d_context* dr2d_create_context(dr2d_drawing_callbacks drawingCallbacks, size_t contextExtraBytes, size_t surfaceExtraBytes, size_t fontExtraBytes, size_t imageExtraBytes, const void* pUserData);

/// Deletes the given context.
void dr2d_delete_context(dr2d_context* pContext);

/// Retrieves a pointer to the given context's extra data buffer.
void* dr2d_get_context_extra_data(dr2d_context* pContext);


/// Creates a surface.
dr2d_surface* dr2d_create_surface(dr2d_context* pContext, float width, float height);

/// Deletes the given surface.
void dr2d_delete_surface(dr2d_surface* pSurface);

/// Retrieves the width of the surface.
float dr2d_get_surface_width(const dr2d_surface* pSurface);

/// Retrieves the height of the surface.
float dr2d_get_surface_height(const dr2d_surface* pSurface);

/// Retrieves a pointer to the given surface's extra data buffer.
void* dr2d_get_surface_extra_data(dr2d_surface* pSurface);



//// Drawing ////

/// Marks the beginning of a paint operation.
void dr2d_begin_draw(dr2d_surface* pSurface);

/// Marks the end of a paint operation.
void dr2d_end_draw(dr2d_surface* pSurface);

/// Clears the given surface with the given color.
void dr2d_clear(dr2d_surface* pSurface, dr2d_color color);

/// Draws a filled rectangle without an outline.
void dr2d_draw_rect(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color);

/// Draws the outline of the given rectangle.
void dr2d_draw_rect_outline(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float outlineWidth);

/// Draws a filled rectangle with an outline.
void dr2d_draw_rect_with_outline(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float outlineWidth, dr2d_color outlineColor);

/// Draws a filled rectangle without an outline with rounded corners.
void dr2d_draw_round_rect(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float radius);

/// Draws the outline of the given rectangle with rounded corners.
void dr2d_draw_round_rect_outline(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float radius, float outlineWidth);

/// Draws a filled rectangle with an outline.
void dr2d_draw_round_rect_with_outline(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float radius, float outlineWidth, dr2d_color outlineColor);

/// Draws a run of text.
void dr2d_draw_text(dr2d_surface* pSurface, dr2d_font* pFont, const char* text, size_t textSizeInBytes, float posX, float posY, dr2d_color color, dr2d_color backgroundColor);

/// Draws an image.
void dr2d_draw_image(dr2d_surface* pSurface, dr2d_image* pImage, dr2d_draw_image_args* pArgs);

/// Sets the clipping rectangle.
void dr2d_set_clip(dr2d_surface* pSurface, float left, float top, float right, float bottom);

/// Retrieves the clipping rectangle.
void dr2d_get_clip(dr2d_surface* pSurface, float* pLeftOut, float* pTopOut, float* pRightOut, float* pBottomOut);


/// Creates a font that can be passed to dr2d_draw_text().
dr2d_font* dr2d_create_font(dr2d_context* pContext, const char* family, unsigned int size, dr2d_font_weight weight, dr2d_font_slant slant, float rotation, unsigned int flags);

/// Deletes a font that was previously created with dr2d_create_font()
void dr2d_delete_font(dr2d_font* pFont);

/// Retrieves a pointer to the given font's extra data buffer.
void* dr2d_get_font_extra_data(dr2d_font* pFont);

/// Retrieves the size of the given font.
unsigned int dr2d_get_font_size(dr2d_font* pFont);

/// Retrieves the metrics of the given font.
bool dr2d_get_font_metrics(dr2d_font* pFont, dr2d_font_metrics* pMetricsOut);

/// Retrieves the metrics of the glyph for the given character when rendered with the given font.
bool dr2d_get_glyph_metrics(dr2d_font* pFont, unsigned int utf32, dr2d_glyph_metrics* pMetricsOut);

/// Retrieves the dimensions of the given string when drawn with the given font.
bool dr2d_measure_string(dr2d_font* pFont, const char* text, size_t textSizeInBytes, float* pWidthOut, float* pHeightOut);

/// Retrieves the position to place a text cursor based on the given point for the given string when drawn with the given font.
bool dr2d_get_text_cursor_position_from_point(dr2d_font* pFont, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosXOut, size_t* pCharacterIndexOut);

/// Retrieves the position to palce a text cursor based on the character at the given index for the given string when drawn with the given font.
bool dr2d_get_text_cursor_position_from_char(dr2d_font* pFont, const char* text, size_t characterIndex, float* pTextCursorPosXOut);


/// Creates an image that can be passed to dr2d_draw_image().
///
/// @remarks
///     The dimensions and format of an image are immutable. If these need to change, then the image needs to be deleted and re-created.
///     @par
///     If pData is NULL, the default image data is undefined.
///     @par
///     Use dr2d_map_image_data() and dr2d_unmap_image_data() to update or retrieve image data.
dr2d_image* dr2d_create_image(dr2d_context* pContext, unsigned int width, unsigned int height, dr2d_image_format format, unsigned int stride, const void* pData);

/// Deletes the given image.
void dr2d_delete_image(dr2d_image* pImage);

/// Retrieves a pointer to the given image's extra data buffer.
void* dr2d_get_image_extra_data(dr2d_image* pImage);

/// Retrieves the size of the given image.
void dr2d_get_image_size(dr2d_image* pImage, unsigned int* pWidthOut, unsigned int* pHeightOut);

/// Retrieves a pointer to a buffer representing the given image's data.
///
/// Call dr2d_unmap_image_data() when you are done with this function.
///
/// Use this function to access an image's data. The returned pointer does not necessarilly point to the image's actual data, so when
/// writing to this pointer, nothing is guaranteed to be updated until dr2d_unmap_image_data() is called.
///
/// The returned data will contain the image data at the time of the mapping.
///
/// This will fail if the image's data is already mapped.
void* dr2d_map_image_data(dr2d_image* pImage, unsigned int accessFlags);

/// Unmaps the given image's data.
///
/// A flush is done at this point to ensure the actual underlying image data is updated.
void dr2d_unmap_image_data(dr2d_image* pImage);

/// Retrieves the optimal image format for the given context. This depends on the backend.
dr2d_image_format dr2d_get_optimal_image_format(dr2d_context* pContext);


/////////////////////////////////////////////////////////////////
//
// UTILITY API
//
/////////////////////////////////////////////////////////////////

/// Creates a color object from a set of RGBA color components.
dr2d_color dr2d_rgba(dr2d_byte r, dr2d_byte g, dr2d_byte b, dr2d_byte a);

/// Creates a fully opaque color object from a set of RGB color components.
dr2d_color dr2d_rgb(dr2d_byte r, dr2d_byte g, dr2d_byte b);




/////////////////////////////////////////////////////////////////
//
// WINDOWS GDI 2D API
//
// When using GDI as the rendering back-end you will usually want to only call drawing functions in response to a WM_PAINT message.
//
/////////////////////////////////////////////////////////////////
#ifndef DR2D_NO_GDI

/// Creates a 2D context with GDI as the backend and, optionally, a custom HDC.
///
/// hDC [in, optional] The main device context. Can be null.
dr2d_context* dr2d_create_context_gdi(HDC hDC);

/// Creates a surface that draws directly to the given window.
///
/// @remarks
///     When using this kind of surface, the internal HBITMAP is not used.
dr2d_surface* dr2d_create_surface_gdi_HWND(dr2d_context* pContext, HWND hWnd);
dr2d_surface* dr2d_create_surface_gdi_HDC(dr2d_context* pContext, HDC hDC);

/// Retrieves the internal HDC that we have been rendering to for the given surface.
///
/// @remarks
///     This assumes the given surface was created from a context that was created from dr2d_create_context_gdi()
HDC dr2d_get_HDC(dr2d_surface* pSurface);

/// Retrieves the internal HBITMAP object that we have been rendering to.
///
/// @remarks
///     This assumes the given surface was created from a context that was created from dr2d_create_context_gdi().
HBITMAP dr2d_get_HBITMAP(dr2d_surface* pSurface);

/// Retrieves the internal HFONT object from the given dr2d_font object.
HFONT dr2d_get_HFONT(dr2d_font* pFont);

#endif  // GDI



/////////////////////////////////////////////////////////////////
//
// CAIRO 2D API
//
/////////////////////////////////////////////////////////////////
#ifndef DR2D_NO_CAIRO
#include <cairo/cairo.h>

/// Creates a 2D context with Cairo as the backend.
dr2d_context* dr2d_create_context_cairo();

/// Creates a surface that draws directly to the given cairo context.
dr2d_surface* dr2d_create_surface_cairo(dr2d_context* pContext, cairo_t* cr);

/// Retrieves the internal cairo_surface_t object from the given surface.
///
/// @remarks
///     This assumes the given surface was created from a context that was created with dr2d_create_context_cairo().
cairo_surface_t* dr2d_get_cairo_surface_t(dr2d_surface* pSurface);

/// Retrieves the internal cairo_t object from the given surface.
cairo_t* dr2d_get_cairo_t(dr2d_surface* pSurface);

#endif  // Cairo


#ifdef __cplusplus
}
#endif

#endif  //dr_2d_h


///////////////////////////////////////////////////////////////////////////////
//
// IMPLEMENTATION
//
///////////////////////////////////////////////////////////////////////////////
#ifdef DR_2D_IMPLEMENTATION
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#ifndef DR2D_PRIVATE
#define DR2D_PRIVATE static
#endif

static int dr2d_strcpy_s(char* dst, size_t dstSizeInBytes, const char* src)
{
#ifdef _WIN32
    return strcpy_s(dst, dstSizeInBytes, src);
#else
    if (dst == 0) {
        return EINVAL;
    }
    if (dstSizeInBytes == 0) {
        return ERANGE;
    }
    if (src == 0) {
        dst[0] = '\0';
        return EINVAL;
    }

    size_t i;
    for (i = 0; i < dstSizeInBytes && src[i] != '\0'; ++i) {
        dst[i] = src[i];
    }

    if (i < dstSizeInBytes) {
        dst[i] = '\0';
        return 0;
    }

    dst[0] = '\0';
    return ERANGE;
#endif
}


dr2d_context* dr2d_create_context(dr2d_drawing_callbacks drawingCallbacks, size_t contextExtraBytes, size_t surfaceExtraBytes, size_t fontExtraBytes, size_t imageExtraBytes, const void* pUserData)
{
    dr2d_context* pContext = (dr2d_context*)malloc(sizeof(dr2d_context) + contextExtraBytes);
    if (pContext != NULL)
    {
        pContext->drawingCallbacks  = drawingCallbacks;
        pContext->imageExtraBytes   = imageExtraBytes;
        pContext->fontExtraBytes    = fontExtraBytes;
        pContext->surfaceExtraBytes = surfaceExtraBytes;
        pContext->contextExtraBytes = contextExtraBytes;
        memset(pContext->pExtraData, 0, contextExtraBytes);

        // The create_context callback is optional. If it is set to NULL, we just finish up here and return successfully. Otherwise
        // we want to call the create_context callback and check it's return value. If it's return value if false it means there
        // was an error and we need to return null.
        if (pContext->drawingCallbacks.on_create_context != NULL)
        {
            if (!pContext->drawingCallbacks.on_create_context(pContext, pUserData))
            {
                // An error was thrown from the callback.
                free(pContext);
                pContext = NULL;
            }
        }
    }

    return pContext;
}

void dr2d_delete_context(dr2d_context* pContext)
{
    if (pContext != NULL) {
        if (pContext->drawingCallbacks.on_delete_context != NULL) {
            pContext->drawingCallbacks.on_delete_context(pContext);
        }

        free(pContext);
    }
}

void* dr2d_get_context_extra_data(dr2d_context* pContext)
{
    return pContext->pExtraData;
}


dr2d_surface* dr2d_create_surface(dr2d_context* pContext, float width, float height)
{
    if (pContext != NULL)
    {
        dr2d_surface* pSurface = (dr2d_surface*)malloc(sizeof(dr2d_surface) + pContext->surfaceExtraBytes);
        if (pSurface != NULL)
        {
            pSurface->pContext = pContext;
            pSurface->width    = width;
            pSurface->height   = height;
            memset(pSurface->pExtraData, 0, pContext->surfaceExtraBytes);

            // The create_surface callback is optional. If it is set to NULL, we just finish up here and return successfully. Otherwise
            // we want to call the create_surface callback and check it's return value. If it's return value if false it means there
            // was an error and we need to return null.
            if (pContext->drawingCallbacks.on_create_surface != NULL)
            {
                if (!pContext->drawingCallbacks.on_create_surface(pSurface, width, height))
                {
                    free(pSurface);
                    pSurface = NULL;
                }
            }
        }

        return pSurface;
    }

    return NULL;
}

void dr2d_delete_surface(dr2d_surface* pSurface)
{
    if (pSurface != NULL)
    {
        assert(pSurface->pContext != NULL);

        if (pSurface->pContext->drawingCallbacks.on_delete_surface != NULL) {
            pSurface->pContext->drawingCallbacks.on_delete_surface(pSurface);
        }

        free(pSurface);
    }
}

float dr2d_get_surface_width(const dr2d_surface* pSurface)
{
    if (pSurface != NULL) {
        return pSurface->width;
    }

    return 0;
}

float dr2d_get_surface_height(const dr2d_surface* pSurface)
{
    if (pSurface != NULL) {
        return pSurface->height;
    }

    return 0;
}

void* dr2d_get_surface_extra_data(dr2d_surface* pSurface)
{
    if (pSurface != NULL) {
        return pSurface->pExtraData;
    }

    return NULL;
}


void dr2d_begin_draw(dr2d_surface* pSurface)
{
    if (pSurface != NULL)
    {
        assert(pSurface->pContext != NULL);

        if (pSurface->pContext->drawingCallbacks.begin_draw != NULL) {
            pSurface->pContext->drawingCallbacks.begin_draw(pSurface);
        }
    }
}

void dr2d_end_draw(dr2d_surface* pSurface)
{
    if (pSurface != NULL)
    {
        assert(pSurface->pContext != NULL);

        if (pSurface->pContext->drawingCallbacks.end_draw != NULL) {
            pSurface->pContext->drawingCallbacks.end_draw(pSurface);
        }
    }
}

void dr2d_clear(dr2d_surface * pSurface, dr2d_color color)
{
    if (pSurface != NULL)
    {
        assert(pSurface->pContext != NULL);

        if (pSurface->pContext->drawingCallbacks.clear != NULL) {
            pSurface->pContext->drawingCallbacks.clear(pSurface, color);
        }
    }
}

void dr2d_draw_rect(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color)
{
    if (pSurface != NULL)
    {
        assert(pSurface->pContext != NULL);

        if (pSurface->pContext->drawingCallbacks.draw_rect != NULL) {
            pSurface->pContext->drawingCallbacks.draw_rect(pSurface, left, top, right, bottom, color);
        }
    }
}

void dr2d_draw_rect_outline(dr2d_surface * pSurface, float left, float top, float right, float bottom, dr2d_color color, float outlineWidth)
{
    if (pSurface != NULL)
    {
        assert(pSurface->pContext != NULL);

        if (pSurface->pContext->drawingCallbacks.draw_rect_outline != NULL) {
            pSurface->pContext->drawingCallbacks.draw_rect_outline(pSurface, left, top, right, bottom, color, outlineWidth);
        }
    }
}

void dr2d_draw_rect_with_outline(dr2d_surface * pSurface, float left, float top, float right, float bottom, dr2d_color color, float outlineWidth, dr2d_color outlineColor)
{
    if (pSurface != NULL)
    {
        assert(pSurface->pContext != NULL);

        if (pSurface->pContext->drawingCallbacks.draw_rect_with_outline != NULL) {
            pSurface->pContext->drawingCallbacks.draw_rect_with_outline(pSurface, left, top, right, bottom, color, outlineWidth, outlineColor);
        }
    }
}

void dr2d_draw_round_rect(dr2d_surface * pSurface, float left, float top, float right, float bottom, dr2d_color color, float radius)
{
    if (pSurface != NULL)
    {
        assert(pSurface->pContext != NULL);

        if (pSurface->pContext->drawingCallbacks.draw_round_rect != NULL) {
            pSurface->pContext->drawingCallbacks.draw_round_rect(pSurface, left, top, right, bottom, color, radius);
        }
    }
}

void dr2d_draw_round_rect_outline(dr2d_surface * pSurface, float left, float top, float right, float bottom, dr2d_color color, float radius, float outlineWidth)
{
    if (pSurface != NULL)
    {
        assert(pSurface->pContext != NULL);

        if (pSurface->pContext->drawingCallbacks.draw_round_rect_outline != NULL) {
            pSurface->pContext->drawingCallbacks.draw_round_rect_outline(pSurface, left, top, right, bottom, color, radius, outlineWidth);
        }
    }
}

void dr2d_draw_round_rect_with_outline(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float radius, float outlineWidth, dr2d_color outlineColor)
{
    if (pSurface != NULL)
    {
        assert(pSurface->pContext != NULL);

        if (pSurface->pContext->drawingCallbacks.draw_round_rect_with_outline != NULL) {
            pSurface->pContext->drawingCallbacks.draw_round_rect_with_outline(pSurface, left, top, right, bottom, color, radius, outlineWidth, outlineColor);
        }
    }
}

void dr2d_draw_text(dr2d_surface* pSurface, dr2d_font* pFont, const char* text, size_t textSizeInBytes, float posX, float posY, dr2d_color color, dr2d_color backgroundColor)
{
    if (pSurface != NULL)
    {
        assert(pSurface->pContext != NULL);

        if (pSurface->pContext->drawingCallbacks.draw_text != NULL) {
            pSurface->pContext->drawingCallbacks.draw_text(pSurface, pFont, text, textSizeInBytes, posX, posY, color, backgroundColor);
        }
    }
}

void dr2d_draw_image(dr2d_surface* pSurface, dr2d_image* pImage, dr2d_draw_image_args* pArgs)
{
    if (pSurface == NULL || pImage == NULL || pArgs == NULL) {
        return;
    }

    assert(pSurface->pContext != NULL);

    if (pSurface->pContext->drawingCallbacks.draw_image) {
        pSurface->pContext->drawingCallbacks.draw_image(pSurface, pImage, pArgs);
    }
}

void dr2d_set_clip(dr2d_surface* pSurface, float left, float top, float right, float bottom)
{
    if (pSurface != NULL)
    {
        assert(pSurface->pContext != NULL);

        if (pSurface->pContext->drawingCallbacks.set_clip != NULL) {
            pSurface->pContext->drawingCallbacks.set_clip(pSurface, left, top, right, bottom);
        }
    }
}

void dr2d_get_clip(dr2d_surface* pSurface, float* pLeftOut, float* pTopOut, float* pRightOut, float* pBottomOut)
{
    if (pSurface != NULL)
    {
        assert(pSurface->pContext != NULL);

        if (pSurface->pContext->drawingCallbacks.get_clip != NULL) {
            pSurface->pContext->drawingCallbacks.get_clip(pSurface, pLeftOut, pTopOut, pRightOut, pBottomOut);
        }
    }
}

dr2d_font* dr2d_create_font(dr2d_context* pContext, const char* family, unsigned int size, dr2d_font_weight weight, dr2d_font_slant slant, float rotation, unsigned int flags)
{
    if (pContext == NULL) {
        return NULL;
    }

    dr2d_font* pFont = (dr2d_font*)malloc(sizeof(dr2d_font) + pContext->fontExtraBytes);
    if (pFont == NULL) {
        return NULL;
    }

    pFont->pContext  = pContext;
    pFont->family[0] = '\0';
    pFont->size      = size;
    pFont->weight    = weight;
    pFont->slant     = slant;
    pFont->rotation  = rotation;
    pFont->flags     = flags;

    if (family != NULL) {
        dr2d_strcpy_s(pFont->family, sizeof(pFont->family), family);
    }

    if (pContext->drawingCallbacks.on_create_font != NULL) {
        if (!pContext->drawingCallbacks.on_create_font(pFont)) {
            free(pFont);
            return NULL;
        }
    }

    return pFont;
}

void dr2d_delete_font(dr2d_font* pFont)
{
    if (pFont == NULL) {
        return;
    }

    assert(pFont->pContext != NULL);

    if (pFont->pContext->drawingCallbacks.on_delete_font != NULL) {
        pFont->pContext->drawingCallbacks.on_delete_font(pFont);
    }

    free(pFont);
}

void* dr2d_get_font_extra_data(dr2d_font* pFont)
{
    if (pFont == NULL) {
        return NULL;
    }

    return pFont->pExtraData;
}

unsigned int dr2d_get_font_size(dr2d_font* pFont)
{
    if (pFont == NULL) {
        return 0;
    }

    return pFont->size;
}

bool dr2d_get_font_metrics(dr2d_font* pFont, dr2d_font_metrics* pMetricsOut)
{
    if (pFont == NULL) {
        return false;
    }

    assert(pFont->pContext != NULL);

    if (pFont->pContext->drawingCallbacks.get_font_metrics != NULL) {
        return pFont->pContext->drawingCallbacks.get_font_metrics(pFont, pMetricsOut);
    }

    return false;
}

bool dr2d_get_glyph_metrics(dr2d_font* pFont, unsigned int utf32, dr2d_glyph_metrics* pMetricsOut)
{
    if (pFont == NULL || pMetricsOut == NULL) {
        return false;
    }

    assert(pFont->pContext != NULL);

    if (pFont->pContext->drawingCallbacks.get_glyph_metrics != NULL) {
        return pFont->pContext->drawingCallbacks.get_glyph_metrics(pFont, utf32, pMetricsOut);
    }

    return false;
}

bool dr2d_measure_string(dr2d_font* pFont, const char* text, size_t textSizeInBytes, float* pWidthOut, float* pHeightOut)
{
    if (pFont == NULL) {
        return false;
    }

    assert(pFont->pContext != NULL);

    if (pFont->pContext->drawingCallbacks.measure_string != NULL) {
        return pFont->pContext->drawingCallbacks.measure_string(pFont, text, textSizeInBytes, pWidthOut, pHeightOut);
    }

    return false;
}

bool dr2d_get_text_cursor_position_from_point(dr2d_font* pFont, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosXOut, size_t* pCharacterIndexOut)
{
    if (pFont == NULL) {
        return false;
    }

    assert(pFont->pContext != NULL);

    if (pFont->pContext->drawingCallbacks.get_text_cursor_position_from_point) {
        return pFont->pContext->drawingCallbacks.get_text_cursor_position_from_point(pFont, text, textSizeInBytes, maxWidth, inputPosX, pTextCursorPosXOut, pCharacterIndexOut);
    }

    return false;
}

bool dr2d_get_text_cursor_position_from_char(dr2d_font* pFont, const char* text, size_t characterIndex, float* pTextCursorPosXOut)
{
    if (pFont == NULL) {
        return false;
    }

    assert(pFont->pContext != NULL);

    if (pFont->pContext->drawingCallbacks.get_text_cursor_position_from_char) {
        return pFont->pContext->drawingCallbacks.get_text_cursor_position_from_char(pFont, text, characterIndex, pTextCursorPosXOut);
    }

    return false;
}


dr2d_image* dr2d_create_image(dr2d_context* pContext, unsigned int width, unsigned int height, dr2d_image_format format, unsigned int stride, const void* pData)
{
    if (pContext == NULL || width == 0 || height == 0) {
        return NULL;
    }

    dr2d_image* pImage = (dr2d_image*)malloc(sizeof(dr2d_image) + pContext->imageExtraBytes);
    if (pImage == NULL) {
        return NULL;
    }

    pImage->pContext = pContext;
    pImage->width    = width;
    pImage->height   = height;
    pImage->format   = format;
    pImage->isMapped = false;

    if (pContext->drawingCallbacks.on_create_image != NULL) {
        if (!pContext->drawingCallbacks.on_create_image(pImage, stride, pData)) {
            free(pImage);
            return NULL;
        }
    }

    return pImage;
}

void dr2d_delete_image(dr2d_image* pImage)
{
    if (pImage == NULL) {
        return;
    }

    assert(pImage->pContext != NULL);

    if (pImage->pContext->drawingCallbacks.on_delete_image != NULL) {
        pImage->pContext->drawingCallbacks.on_delete_image(pImage);
    }

    free(pImage);
}

void* dr2d_get_image_extra_data(dr2d_image* pImage)
{
    if (pImage == NULL) {
        return NULL;
    }

    return pImage->pExtraData;
}

void dr2d_get_image_size(dr2d_image* pImage, unsigned int* pWidthOut, unsigned int* pHeightOut)
{
    if (pImage == NULL) {
        return;
    }

    if (pWidthOut) {
        *pWidthOut = pImage->width;
    }
    if (pHeightOut) {
        *pHeightOut = pImage->height;
    }
}

void* dr2d_map_image_data(dr2d_image* pImage, unsigned int accessFlags)
{
    if (pImage == NULL || pImage->pContext->drawingCallbacks.map_image_data == NULL || pImage->pContext->drawingCallbacks.unmap_image_data == NULL || pImage->isMapped) {
        return NULL;
    }

    void* pImageData = pImage->pContext->drawingCallbacks.map_image_data(pImage, accessFlags);
    if (pImageData != NULL) {
        pImage->isMapped = true;
    }

    return pImageData;
}

void dr2d_unmap_image_data(dr2d_image* pImage)
{
    if (pImage == NULL || pImage->pContext->drawingCallbacks.unmap_image_data == NULL || !pImage->isMapped) {
        return;
    }

    pImage->pContext->drawingCallbacks.unmap_image_data(pImage);
    pImage->isMapped = false;
}

dr2d_image_format dr2d_get_optimal_image_format(dr2d_context* pContext)
{
    if (pContext == NULL || pContext->drawingCallbacks.get_optimal_image_format == NULL) {
        return dr2d_image_format_rgba8;
    }

    return pContext->drawingCallbacks.get_optimal_image_format(pContext);
}



/////////////////////////////////////////////////////////////////
//
// UTILITY API
//
/////////////////////////////////////////////////////////////////

dr2d_color dr2d_rgba(dr2d_byte r, dr2d_byte g, dr2d_byte b, dr2d_byte a)
{
    dr2d_color color;
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;

    return color;
}

dr2d_color dr2d_rgb(dr2d_byte r, dr2d_byte g, dr2d_byte b)
{
    dr2d_color color;
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = 255;

    return color;
}



/////////////////////////////////////////////////////////////////
//
// PRIVATE UTILITY API
//
/////////////////////////////////////////////////////////////////

// RGBA8 <-> BGRA8 swap with alpha pre-multiply.
void dr2d__rgba8_bgra8_swap__premul(const void* pSrc, void* pDst, unsigned int width, unsigned int height, unsigned int srcStride, unsigned int dstStride)
{
    assert(pSrc != NULL);
    assert(pDst != NULL);

    const unsigned int srcStride32 = srcStride/4;
    const unsigned int dstStride32 = dstStride/4;

    const unsigned int* pSrcRow = (const unsigned int*)pSrc;
          unsigned int* pDstRow = (unsigned int*)pDst;

    for (unsigned int iRow = 0; iRow < height; ++iRow)
    {
        for (unsigned int iCol = 0; iCol < width; ++iCol)
        {
            unsigned int srcTexel = pSrcRow[iCol];
            unsigned int srcTexelA = (srcTexel & 0xFF000000) >> 24;
            unsigned int srcTexelB = (srcTexel & 0x00FF0000) >> 16;
            unsigned int srcTexelG = (srcTexel & 0x0000FF00) >> 8;
            unsigned int srcTexelR = (srcTexel & 0x000000FF) >> 0;

            srcTexelB = (unsigned int)(srcTexelB * (srcTexelA / 255.0f));
            srcTexelG = (unsigned int)(srcTexelG * (srcTexelA / 255.0f));
            srcTexelR = (unsigned int)(srcTexelR * (srcTexelA / 255.0f));

            pDstRow[iCol] = (srcTexelR << 16) | (srcTexelG << 8) | (srcTexelB << 0) | (srcTexelA << 24);
        }

        pSrcRow += srcStride32;
        pDstRow += dstStride32;
    }
}


/////////////////////////////////////////////////////////////////
//
// WINDOWS GDI 2D API
//
/////////////////////////////////////////////////////////////////
#ifndef DR2D_NO_GDI

typedef struct
{
    /// The device context that owns every surface HBITMAP object. All drawing is done through this DC.
    HDC hDC;

    /// The buffer used to store wchar strings when converting from char* to wchar_t* strings. We just use a global buffer for
    /// this so we can avoid unnecessary allocations.
    wchar_t* wcharBuffer;

    /// The size of wcharBuffer (including the null terminator).
    unsigned int wcharBufferLength;

    /// The cache of glyph character positions.
    int* pGlyphCache;
    size_t glyphCacheSize;

    /// Whether or not the context owns the device context.
    bool ownsDC;

} gdi_context_data;

typedef struct
{
    /// The window to draw to. The can be null, which is the case when creating the surface with dr2d_create_surface(). When this
    /// is non-null the size of the surface is always tied to the window.
    HWND hWnd;

    /// The HDC to use when drawing to the surface.
    HDC hDC;

    /// The intermediate DC to use when drawing bitmaps.
    HDC hIntermediateDC;

    /// The PAINTSTRUCT object that is filled by BeginPaint(). Only used when hWnd is non-null.
    PAINTSTRUCT ps;

    /// The bitmap to render to. This is created with GDI's CreateDIBSection() API, using the DC above. This is only used
    /// when hDC is NULL, which is the default.
    HBITMAP hBitmap;

    /// A pointer to the raw bitmap data. This is allocated CreateDIBSection().
    void* pBitmapData;


    /// The stock DC brush.
    HGDIOBJ hStockDCBrush;

    /// The stock null brush.
    HGDIOBJ hStockNullBrush;

    /// The stock DC pen.
    HGDIOBJ hStockDCPen;

    /// The stock null pen.
    HGDIOBJ hStockNullPen;


    /// The pen that was active at the start of drawing. This is restored at the end of drawing.
    HGDIOBJ hPrevPen;

    /// The brush that was active at the start of drawing.
    HGDIOBJ hPrevBrush;

    /// The brush color at the start of drawing.
    COLORREF prevBrushColor;

    /// The previous font.
    HGDIOBJ hPrevFont;

    /// The previous text background mode.
    int prevBkMode;

    /// The previous text background color.
    COLORREF prevBkColor;


} gdi_surface_data;

typedef struct
{
    /// A handle to the Win32 font.
    HFONT hFont;

    /// The font metrics for faster retrieval. We cache the metrics when the font is loaded.
    dr2d_font_metrics metrics;

} gdi_font_data;

typedef struct
{
    /// A handle to the primary bitmap object.
    HBITMAP hSrcBitmap;

    /// A pointer to the raw bitmap data.
    unsigned int* pSrcBitmapData;

    /// A handle to the secondary bitmap object that we use when we need to change the color values of
    /// the primary bitmap before drawing.
    HBITMAP hIntermediateBitmap;

    /// A pointer to the raw data of the intermediate bitmap.
    unsigned int* pIntermediateBitmapData;


    /// A pointer to the mapped data. This is null when the image data is not mapped.
    void* pMappedImageData;

    /// The mapping flags.
    unsigned int mapAccessFlags;

} gdi_image_data;


bool dr2d_on_create_context_gdi(dr2d_context* pContext, const void* pUserData);
void dr2d_on_delete_context_gdi(dr2d_context* pContext);
bool dr2d_on_create_surface_gdi(dr2d_surface* pSurface, float width, float height);
void dr2d_on_delete_surface_gdi(dr2d_surface* pSurface);
bool dr2d_on_create_font_gdi(dr2d_font* pFont);
void dr2d_on_delete_font_gdi(dr2d_font* pFont);
bool dr2d_on_create_image_gdi(dr2d_image* pImage, unsigned int stride, const void* pData);
void dr2d_on_delete_image_gdi(dr2d_image* pImage);

void dr2d_begin_draw_gdi(dr2d_surface* pSurface);
void dr2d_end_draw_gdi(dr2d_surface* pSurface);
void dr2d_clear_gdi(dr2d_surface* pSurface, dr2d_color color);
void dr2d_draw_rect_gdi(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color);
void dr2d_draw_rect_outline_gdi(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float outlineWidth);
void dr2d_draw_rect_with_outline_gdi(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float outlineWidth, dr2d_color outlineColor);
void dr2d_draw_round_rect_gdi(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float radius);
void dr2d_draw_round_rect_outline_gdi(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float radius, float outlineWidth);
void dr2d_draw_round_rect_with_outline_gdi(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float radius, float outlineWidth, dr2d_color outlineColor);
void dr2d_draw_text_gdi(dr2d_surface* pSurface, dr2d_font* pFont, const char* text, size_t textSizeInBytes, float posX, float posY, dr2d_color color, dr2d_color backgroundColor);
void dr2d_draw_image_gdi(dr2d_surface* pSurface, dr2d_image* pImage, dr2d_draw_image_args* pArgs);
void dr2d_set_clip_gdi(dr2d_surface* pSurface, float left, float top, float right, float bottom);
void dr2d_get_clip_gdi(dr2d_surface* pSurface, float* pLeftOut, float* pTopOut, float* pRightOut, float* pBottomOut);

dr2d_image_format dr2d_get_optimal_image_format_gdi(dr2d_context* pContext);
void* dr2d_map_image_data_gdi(dr2d_image* pImage, unsigned accessFlags);
void dr2d_unmap_image_data_gdi(dr2d_image* pImage);

bool dr2d_get_font_metrics_gdi(dr2d_font* pFont, dr2d_font_metrics* pMetricsOut);
bool dr2d_get_glyph_metrics_gdi(dr2d_font* pFont, unsigned int utf32, dr2d_glyph_metrics* pGlyphMetrics);
bool dr2d_measure_string_gdi(dr2d_font* pFont, const char* text, size_t textSizeInBytes, float* pWidthOut, float* pHeightOut);
bool dr2d_get_text_cursor_position_from_point_gdi(dr2d_font* pFont, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosXOut, size_t* pCharacterIndexOut);
bool dr2d_get_text_cursor_position_from_char_gdi(dr2d_font* pFont, const char* text, size_t characterIndex, float* pTextCursorPosXOut);

/// Converts a char* to a wchar_t* string.
wchar_t* dr2d_to_wchar_gdi(dr2d_context* pContext, const char* text, size_t textSizeInBytes, unsigned int* characterCountOut);

/// Converts a UTF-32 character to a UTF-16.
static int dr2d_utf32_to_utf16(unsigned int utf32, unsigned short utf16[2])
{
    if (utf16 == NULL) {
        return 0;
    }

    if (utf32 < 0xD800 || (utf32 >= 0xE000 && utf32 <= 0xFFFF))
    {
        utf16[0] = (unsigned short)utf32;
        utf16[1] = 0;
        return 1;
    }
    else
    {
        if (utf32 >= 0x10000 && utf32 <= 0x10FFFF)
        {
            utf16[0] = (unsigned short)(0xD7C0 + (unsigned short)(utf32 >> 10));
            utf16[1] = (unsigned short)(0xDC00 + (unsigned short)(utf32 & 0x3FF));
            return 2;
        }
        else
        {
            // Invalid.
            utf16[0] = 0;
            utf16[0] = 0;
            return 0;
        }
    }
}

dr2d_context* dr2d_create_context_gdi(HDC hDC)
{
    dr2d_drawing_callbacks callbacks;
    callbacks.on_create_context                   = dr2d_on_create_context_gdi;
    callbacks.on_delete_context                   = dr2d_on_delete_context_gdi;
    callbacks.on_create_surface                   = dr2d_on_create_surface_gdi;
    callbacks.on_delete_surface                   = dr2d_on_delete_surface_gdi;
    callbacks.on_create_font                      = dr2d_on_create_font_gdi;
    callbacks.on_delete_font                      = dr2d_on_delete_font_gdi;
    callbacks.on_create_image                     = dr2d_on_create_image_gdi;
    callbacks.on_delete_image                     = dr2d_on_delete_image_gdi;

    callbacks.begin_draw                          = dr2d_begin_draw_gdi;
    callbacks.end_draw                            = dr2d_end_draw_gdi;
    callbacks.clear                               = dr2d_clear_gdi;
    callbacks.draw_rect                           = dr2d_draw_rect_gdi;
    callbacks.draw_rect_outline                   = dr2d_draw_rect_outline_gdi;
    callbacks.draw_rect_with_outline              = dr2d_draw_rect_with_outline_gdi;
    callbacks.draw_round_rect                     = dr2d_draw_round_rect_gdi;
    callbacks.draw_round_rect_outline             = dr2d_draw_round_rect_outline_gdi;
    callbacks.draw_round_rect_with_outline        = dr2d_draw_round_rect_with_outline_gdi;
    callbacks.draw_text                           = dr2d_draw_text_gdi;
    callbacks.draw_image                          = dr2d_draw_image_gdi;
    callbacks.set_clip                            = dr2d_set_clip_gdi;
    callbacks.get_clip                            = dr2d_get_clip_gdi;

    callbacks.get_optimal_image_format            = dr2d_get_optimal_image_format_gdi;
    callbacks.map_image_data                      = dr2d_map_image_data_gdi;
    callbacks.unmap_image_data                    = dr2d_unmap_image_data_gdi;

    callbacks.get_font_metrics                    = dr2d_get_font_metrics_gdi;
    callbacks.get_glyph_metrics                   = dr2d_get_glyph_metrics_gdi;
    callbacks.measure_string                      = dr2d_measure_string_gdi;
    callbacks.get_text_cursor_position_from_point = dr2d_get_text_cursor_position_from_point_gdi;
    callbacks.get_text_cursor_position_from_char  = dr2d_get_text_cursor_position_from_char_gdi;

    return dr2d_create_context(callbacks, sizeof(gdi_context_data), sizeof(gdi_surface_data), sizeof(gdi_font_data), sizeof(gdi_image_data), &hDC);
}

dr2d_surface* dr2d_create_surface_gdi_HWND(dr2d_context* pContext, HWND hWnd)
{
    dr2d_surface* pSurface = dr2d_create_surface(pContext, 0, 0);
    if (pSurface != NULL) {
        gdi_surface_data* pGDIData = (gdi_surface_data*)dr2d_get_surface_extra_data(pSurface);
        if (pGDIData != NULL) {
            pGDIData->hWnd = hWnd;
        }
    }

    return pSurface;
}

dr2d_surface* dr2d_create_surface_gdi_HDC(dr2d_context* pContext, HDC hDC)
{
    dr2d_surface* pSurface = dr2d_create_surface(pContext, 0, 0);
    if (pSurface != NULL) {
        gdi_surface_data* pGDIData = (gdi_surface_data*)dr2d_get_surface_extra_data(pSurface);
        if (pGDIData != NULL) {
            pGDIData->hDC = hDC;
        }
    }

    return pSurface;
}

HDC dr2d_get_HDC(dr2d_surface* pSurface)
{
    if (pSurface != NULL) {
        gdi_surface_data* pGDIData = (gdi_surface_data*)dr2d_get_surface_extra_data(pSurface);
        if (pGDIData != NULL) {
            return pGDIData->hDC;
        }
    }

    return NULL;
}

HBITMAP dr2d_get_HBITMAP(dr2d_surface* pSurface)
{
    if (pSurface != NULL) {
        gdi_surface_data* pGDIData = (gdi_surface_data*)dr2d_get_surface_extra_data(pSurface);
        if (pGDIData != NULL) {
            return pGDIData->hBitmap;
        }
    }

    return NULL;
}

HFONT dr2d_get_HFONT(dr2d_font* pFont)
{
    gdi_font_data* pGDIData = (gdi_font_data*)dr2d_get_font_extra_data(pFont);
    if (pGDIData == NULL) {
        return NULL;
    }

    return pGDIData->hFont;
}


bool dr2d_on_create_context_gdi(dr2d_context* pContext, const void* pUserData)
{
    assert(pContext != NULL);

    HDC hDC = NULL;
    if (pUserData != NULL) {
        hDC = *(HDC*)pUserData;
    }

    bool ownsDC = false;
    if (hDC == NULL) {
        hDC = CreateCompatibleDC(GetDC(GetDesktopWindow()));
        ownsDC = true;
    }


    // We need to create the DC that all of our rendering commands will be drawn to.
    gdi_context_data* pGDIData = (gdi_context_data*)dr2d_get_context_extra_data(pContext);
    if (pGDIData == NULL) {
        return false;
    }

    pGDIData->hDC = hDC;
    if (pGDIData->hDC == NULL) {
        return false;
    }

    pGDIData->ownsDC = ownsDC;


    // We want to use the advanced graphics mode so that GetTextExtentPoint32() performs the conversions for font rotation for us.
    SetGraphicsMode(pGDIData->hDC, GM_ADVANCED);


    pGDIData->wcharBuffer       = NULL;
    pGDIData->wcharBufferLength = 0;

    pGDIData->pGlyphCache = NULL;
    pGDIData->glyphCacheSize = 0;

    return true;
}

void dr2d_on_delete_context_gdi(dr2d_context* pContext)
{
    assert(pContext != NULL);

    gdi_context_data* pGDIData = (gdi_context_data*)dr2d_get_context_extra_data(pContext);
    if (pGDIData != NULL)
    {
        free(pGDIData->pGlyphCache);
        pGDIData->glyphCacheSize = 0;

        free(pGDIData->wcharBuffer);
        pGDIData->wcharBuffer       = 0;
        pGDIData->wcharBufferLength = 0;

        if (pGDIData->ownsDC) {
            DeleteDC(pGDIData->hDC);
        }

        pGDIData->hDC = NULL;
    }
}

bool dr2d_on_create_surface_gdi(dr2d_surface* pSurface, float width, float height)
{
    assert(pSurface != NULL);

    gdi_context_data* pGDIContextData = (gdi_context_data*)dr2d_get_context_extra_data(pSurface->pContext);
    if (pGDIContextData == NULL) {
        return false;
    }

    gdi_surface_data* pGDISurfaceData = (gdi_surface_data*)dr2d_get_surface_extra_data(pSurface);
    if (pGDISurfaceData == NULL) {
        return false;
    }

    HDC hDC = pGDIContextData->hDC;
    if (hDC == NULL) {
        return false;
    }

    HDC hIntermediateDC = CreateCompatibleDC(hDC);
    if (hIntermediateDC == NULL) {
        return false;
    }

    pGDISurfaceData->hIntermediateDC = hIntermediateDC;
    pGDISurfaceData->hWnd = NULL;


    if (width != 0 && height != 0)
    {
        pGDISurfaceData->hDC = hDC;

        BITMAPINFO bmi;
        ZeroMemory(&bmi, sizeof(bmi));
        bmi.bmiHeader.biSize        = sizeof(bmi.bmiHeader);
        bmi.bmiHeader.biWidth       = (LONG)width;
        bmi.bmiHeader.biHeight      = (LONG)height;
        bmi.bmiHeader.biPlanes      = 1;
        bmi.bmiHeader.biBitCount    = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        pGDISurfaceData->hBitmap = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, &pGDISurfaceData->pBitmapData, NULL, 0);
        if (pGDISurfaceData->hBitmap == NULL) {
            return false;
        }
    }
    else
    {
        pGDISurfaceData->hBitmap = NULL;
        pGDISurfaceData->hDC     = NULL;
    }


    return true;
}

void dr2d_on_delete_surface_gdi(dr2d_surface* pSurface)
{
    assert(pSurface != NULL);

    gdi_surface_data* pGDIData = (gdi_surface_data*)dr2d_get_surface_extra_data(pSurface);
    if (pGDIData != NULL)
    {
        DeleteObject(pGDIData->hBitmap);
        pGDIData->hBitmap = NULL;

        DeleteDC(pGDIData->hIntermediateDC);
        pGDIData->hIntermediateDC = NULL;
    }
}

bool dr2d_on_create_font_gdi(dr2d_font* pFont)
{
    assert(pFont != NULL);

    gdi_font_data* pGDIData = (gdi_font_data*)dr2d_get_font_extra_data(pFont);
    if (pGDIData == NULL) {
        return false;
    }


    LONG weightGDI = FW_REGULAR;
    switch (pFont->weight)
    {
    case dr2d_font_weight_medium:      weightGDI = FW_MEDIUM;     break;
    case dr2d_font_weight_thin:        weightGDI = FW_THIN;       break;
    case dr2d_font_weight_extra_light: weightGDI = FW_EXTRALIGHT; break;
    case dr2d_font_weight_light:       weightGDI = FW_LIGHT;      break;
    case dr2d_font_weight_semi_bold:   weightGDI = FW_SEMIBOLD;   break;
    case dr2d_font_weight_bold:        weightGDI = FW_BOLD;       break;
    case dr2d_font_weight_extra_bold:  weightGDI = FW_EXTRABOLD;  break;
    case dr2d_font_weight_heavy:       weightGDI = FW_HEAVY;      break;
    default: break;
    }

	BYTE slantGDI = FALSE;
    if (pFont->slant == dr2d_font_slant_italic || pFont->slant == dr2d_font_slant_oblique) {
        slantGDI = TRUE;
    }


	LOGFONTA logfont;
	memset(&logfont, 0, sizeof(logfont));



    logfont.lfHeight      = -(LONG)pFont->size;
	logfont.lfWeight      = weightGDI;
	logfont.lfItalic      = slantGDI;
	logfont.lfCharSet     = DEFAULT_CHARSET;
	//logfont.lfQuality     = (pFont->size > 36) ? ANTIALIASED_QUALITY : CLEARTYPE_QUALITY;
    logfont.lfQuality     = (pFont->flags & DR2D_FONT_NO_CLEARTYPE) ? ANTIALIASED_QUALITY : CLEARTYPE_QUALITY;
    logfont.lfEscapement  = (LONG)pFont->rotation * 10;
    logfont.lfOrientation = (LONG)pFont->rotation * 10;

    size_t familyLength = strlen(pFont->family);
	memcpy(logfont.lfFaceName, pFont->family, (familyLength < 31) ? familyLength : 31);


	pGDIData->hFont = CreateFontIndirectA(&logfont);
    if (pGDIData->hFont == NULL) {
        return false;
    }


    gdi_context_data* pGDIContextData = (gdi_context_data*)dr2d_get_context_extra_data(pFont->pContext);
    if (pGDIContextData == NULL) {
        return false;
    }

    // Cache the font metrics.
    HGDIOBJ hPrevFont = SelectObject(pGDIContextData->hDC, pGDIData->hFont);
    {
        TEXTMETRIC metrics;
        GetTextMetrics(pGDIContextData->hDC, &metrics);

        pGDIData->metrics.ascent     = metrics.tmAscent;
        pGDIData->metrics.descent    = metrics.tmDescent;
        pGDIData->metrics.lineHeight = metrics.tmHeight;


        const MAT2 transform = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};        // <-- Identity matrix

        GLYPHMETRICS spaceMetrics;
        DWORD bitmapBufferSize = GetGlyphOutlineW(pGDIContextData->hDC, ' ', GGO_NATIVE, &spaceMetrics, 0, NULL, &transform);
        if (bitmapBufferSize == GDI_ERROR) {
			pGDIData->metrics.spaceWidth = 4;
        } else {
            pGDIData->metrics.spaceWidth = spaceMetrics.gmCellIncX;
        }
    }
    SelectObject(pGDIContextData->hDC, hPrevFont);


    return true;
}

void dr2d_on_delete_font_gdi(dr2d_font* pFont)
{
    assert(pFont != NULL);

    gdi_font_data* pGDIData = (gdi_font_data*)dr2d_get_font_extra_data(pFont);
    if (pGDIData == NULL) {
        return;
    }

    DeleteObject(pGDIData->hFont);
}

bool dr2d_on_create_image_gdi(dr2d_image* pImage, unsigned int stride, const void* pData)
{
    assert(pImage != NULL);

    gdi_image_data* pGDIData = (gdi_image_data*)dr2d_get_image_extra_data(pImage);
    if (pGDIData == NULL) {
        return false;
    }

    gdi_context_data* pGDIContextData = (gdi_context_data*)dr2d_get_context_extra_data(pImage->pContext);
    if (pGDIContextData == NULL) {
        return false;
    }


    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize        = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth       = pImage->width;
    bmi.bmiHeader.biHeight      = pImage->height;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;   // Only supporting 32-bit formats.
    bmi.bmiHeader.biCompression = BI_RGB;
    pGDIData->hSrcBitmap = CreateDIBSection(pGDIContextData->hDC, &bmi, DIB_RGB_COLORS, (void**)&pGDIData->pSrcBitmapData, NULL, 0);
    if (pGDIData->hSrcBitmap == NULL) {
        return false;
    }

    pGDIData->hIntermediateBitmap = CreateDIBSection(pGDIContextData->hDC, &bmi, DIB_RGB_COLORS, (void**)&pGDIData->pIntermediateBitmapData, NULL, 0);
    if (pGDIData->hIntermediateBitmap == NULL) {
        DeleteObject(pGDIData->hSrcBitmap);
        return false;
    }


    // We need to convert the data so it renders correctly with AlphaBlend().
    if (pData != NULL) {
        dr2d__rgba8_bgra8_swap__premul(pData, pGDIData->pSrcBitmapData, pImage->width, pImage->height, stride, pImage->width*4);
    }

    // Flush GDI to let it know we are finished with the bitmap object's data.
    GdiFlush();


    pGDIData->pMappedImageData = NULL;
    pGDIData->mapAccessFlags = 0;

    // At this point everything should be good.
    return true;
}

void dr2d_on_delete_image_gdi(dr2d_image* pImage)
{
    assert(pImage != NULL);

    gdi_image_data* pGDIData = (gdi_image_data*)dr2d_get_image_extra_data(pImage);
    if (pGDIData == NULL) {
        return;
    }

    DeleteObject(pGDIData->hSrcBitmap);
    pGDIData->hSrcBitmap = NULL;

    DeleteObject(pGDIData->hIntermediateBitmap);
    pGDIData->hIntermediateBitmap = NULL;
}


void dr2d_begin_draw_gdi(dr2d_surface* pSurface)
{
    assert(pSurface != NULL);

    gdi_surface_data* pGDIData = (gdi_surface_data*)dr2d_get_surface_extra_data(pSurface);
    if (pGDIData != NULL) {
        if (pGDIData->hWnd != NULL) {
            pGDIData->hDC = BeginPaint(pGDIData->hWnd, &pGDIData->ps);
        } else {
            SelectObject(dr2d_get_HDC(pSurface), pGDIData->hBitmap);
        }

        HDC hDC = dr2d_get_HDC(pSurface);

        pGDIData->hStockDCBrush   = GetStockObject(DC_BRUSH);
        pGDIData->hStockNullBrush = GetStockObject(NULL_BRUSH);
        pGDIData->hStockDCPen     = GetStockObject(DC_PEN);
        pGDIData->hStockNullPen   = GetStockObject(NULL_PEN);

        // Retrieve the defaults so they can be restored later.
        pGDIData->hPrevPen       = GetCurrentObject(hDC, OBJ_PEN);
        pGDIData->hPrevBrush     = GetCurrentObject(hDC, OBJ_BRUSH);
        pGDIData->prevBrushColor = GetDCBrushColor(hDC);
        pGDIData->hPrevFont      = GetCurrentObject(hDC, OBJ_FONT);
        pGDIData->prevBkMode     = GetBkMode(hDC);
        pGDIData->prevBkColor    = GetBkColor(hDC);
    }
}

void dr2d_end_draw_gdi(dr2d_surface* pSurface)
{
    assert(pSurface != NULL);

    gdi_surface_data* pGDIData = (gdi_surface_data*)dr2d_get_surface_extra_data(pSurface);
    if (pGDIData != NULL) {
        HDC hDC = dr2d_get_HDC(pSurface);

        SelectClipRgn(hDC, NULL);

        SelectObject(hDC, pGDIData->hPrevPen);
        SelectObject(hDC, pGDIData->hPrevBrush);
        SetDCBrushColor(hDC, pGDIData->prevBrushColor);
        SelectObject(hDC, pGDIData->hPrevFont);
        SetBkMode(hDC, pGDIData->prevBkMode);
        SetBkColor(hDC, pGDIData->prevBkColor);

        if (pGDIData->hWnd != NULL) {
            EndPaint(pGDIData->hWnd, &pGDIData->ps);
        }
    }
}

void dr2d_clear_gdi(dr2d_surface* pSurface, dr2d_color color)
{
    assert(pSurface != NULL);

    dr2d_draw_rect_gdi(pSurface, 0, 0, pSurface->width, pSurface->height, color);
}

void dr2d_draw_rect_gdi(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color)
{
    assert(pSurface != NULL);

    gdi_surface_data* pGDIData = (gdi_surface_data*)dr2d_get_surface_extra_data(pSurface);
    if (pGDIData != NULL)
    {
        HDC hDC = dr2d_get_HDC(pSurface);

        SelectObject(hDC, pGDIData->hStockNullPen);
        SelectObject(hDC, pGDIData->hStockDCBrush);
        SetDCBrushColor(hDC, RGB(color.r, color.g, color.b));

        // Now draw the rectangle. The documentation for this says that the width and height is 1 pixel less when the pen is null. Therefore we will
        // increase the width and height by 1 since we have got the pen set to null.
        Rectangle(hDC, (int)left, (int)top, (int)right + 1, (int)bottom + 1);
    }
}

void dr2d_draw_rect_outline_gdi(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float outlineWidth)
{
    assert(pSurface != NULL);

    gdi_surface_data* pGDIData = (gdi_surface_data*)dr2d_get_surface_extra_data(pSurface);
    if (pGDIData != NULL)
    {
        HDC hDC = dr2d_get_HDC(pSurface);

        SelectObject(hDC, pGDIData->hStockNullPen);
        SelectObject(hDC, pGDIData->hStockDCBrush);
        SetDCBrushColor(hDC, RGB(color.r, color.g, color.b));

        // Now draw the rectangle. The documentation for this says that the width and height is 1 pixel less when the pen is null. Therefore we will
        // increase the width and height by 1 since we have got the pen set to null.

        Rectangle(hDC, (int)left,                   (int)top,                     (int)(left + outlineWidth + 1),  (int)(bottom + 1));              // Left.
        Rectangle(hDC, (int)(right - outlineWidth), (int)top,                     (int)(right + 1),                (int)(bottom + 1));              // Right.
        Rectangle(hDC, (int)(left + outlineWidth),  (int)top,                     (int)(right - outlineWidth + 1), (int)(top + outlineWidth + 1));  // Top
        Rectangle(hDC, (int)(left + outlineWidth),  (int)(bottom - outlineWidth), (int)(right - outlineWidth + 1), (int)(bottom + 1));              // Bottom
    }
}

void dr2d_draw_rect_with_outline_gdi(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float outlineWidth, dr2d_color outlineColor)
{
    assert(pSurface != NULL);

    gdi_surface_data* pGDIData = (gdi_surface_data*)dr2d_get_surface_extra_data(pSurface);
    if (pGDIData != NULL)
    {
        HDC hDC = dr2d_get_HDC(pSurface);

        HPEN hPen = CreatePen(PS_SOLID | PS_INSIDEFRAME, (int)outlineWidth, RGB(outlineColor.r, outlineColor.g, outlineColor.b));
        if (hPen != NULL)
        {
            SelectObject(hDC, hPen);
            SelectObject(hDC, pGDIData->hStockDCBrush);
            SetDCBrushColor(hDC, RGB(color.r, color.g, color.b));

            Rectangle(hDC, (int)left, (int)top, (int)right, (int)bottom);

            DeleteObject(hPen);
        }
    }
}

void dr2d_draw_round_rect_gdi(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float radius)
{
    assert(pSurface != NULL);

    gdi_surface_data* pGDIData = (gdi_surface_data*)dr2d_get_surface_extra_data(pSurface);
    if (pGDIData != NULL)
    {
        HDC hDC = dr2d_get_HDC(pSurface);

        SelectObject(hDC, pGDIData->hStockNullPen);
        SelectObject(hDC, pGDIData->hStockDCBrush);
        SetDCBrushColor(hDC, RGB(color.r, color.g, color.b));

        RoundRect(hDC, (int)left, (int)top, (int)right + 1, (int)bottom + 1, (int)(radius*2), (int)(radius*2));
    }
}

void dr2d_draw_round_rect_outline_gdi(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float radius, float outlineWidth)
{
    assert(pSurface != NULL);

    gdi_surface_data* pGDIData = (gdi_surface_data*)dr2d_get_surface_extra_data(pSurface);
    if (pGDIData != NULL)
    {
        HDC hDC = dr2d_get_HDC(pSurface);

        HPEN hPen = CreatePen(PS_SOLID | PS_INSIDEFRAME, (int)outlineWidth, RGB(color.r, color.g, color.b));
        if (hPen != NULL)
        {
            SelectObject(hDC, pGDIData->hStockNullBrush);
            SelectObject(hDC, hPen);

            RoundRect(hDC, (int)left, (int)top, (int)right, (int)bottom, (int)(radius*2), (int)(radius*2));

            DeleteObject(hPen);
        }
    }
}

void dr2d_draw_round_rect_with_outline_gdi(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float radius, float outlineWidth, dr2d_color outlineColor)
{
    assert(pSurface != NULL);

    gdi_surface_data* pGDIData = (gdi_surface_data*)dr2d_get_surface_extra_data(pSurface);
    if (pGDIData != NULL)
    {
        HDC hDC = dr2d_get_HDC(pSurface);

        HPEN hPen = CreatePen(PS_SOLID | PS_INSIDEFRAME, (int)outlineWidth, RGB(outlineColor.r, outlineColor.g, outlineColor.b));
        if (hPen != NULL)
        {
            SelectObject(hDC, hPen);
            SelectObject(hDC, pGDIData->hStockDCBrush);
            SetDCBrushColor(hDC, RGB(color.r, color.g, color.b));

            RoundRect(hDC, (int)left, (int)top, (int)right, (int)bottom, (int)(radius*2), (int)(radius*2));

            DeleteObject(hPen);
        }
    }
}

void dr2d_draw_text_gdi(dr2d_surface* pSurface, dr2d_font* pFont, const char* text, size_t textSizeInBytes, float posX, float posY, dr2d_color color, dr2d_color backgroundColor)
{
    gdi_font_data* pGDIFontData = (gdi_font_data*)dr2d_get_font_extra_data(pFont);
    if (pGDIFontData == NULL) {
        return;
    }


    HDC hDC = dr2d_get_HDC(pSurface);

    HFONT hFontGDI = pGDIFontData->hFont;
    if (hFontGDI != NULL)
    {
        // We actually want to use the W version of TextOut because otherwise unicode doesn't work properly.

        unsigned int textWLength;
        wchar_t* textW = dr2d_to_wchar_gdi(pSurface->pContext, text, textSizeInBytes, &textWLength);
        if (textW != NULL)
        {
            SelectObject(hDC, hFontGDI);

            UINT options = 0;
            RECT rect = {0, 0, 0, 0};

            if (backgroundColor.a == 0) {
                SetBkMode(hDC, TRANSPARENT);
            } else {
                SetBkMode(hDC, OPAQUE);
                SetBkColor(hDC, RGB(backgroundColor.r, backgroundColor.g, backgroundColor.b));

                // There is an issue with the way GDI draws the background of a string of text. When ClearType is enabled, the rectangle appears
                // to be wider than it is supposed to be. As a result, drawing text right next to each other results in the most recent one being
                // drawn slightly on top of the previous one. To fix this we need to use ExtTextOut() with the ETO_CLIPPED parameter enabled.
                options |= ETO_CLIPPED;

                SIZE textSize = {0, 0};
                GetTextExtentPoint32W(hDC, textW, textWLength, &textSize);
                rect.left   = (LONG)posX;
                rect.top    = (LONG)posY;
                rect.right  = (LONG)(posX + textSize.cx);
                rect.bottom = (LONG)(posY + textSize.cy);
            }

            SetTextColor(hDC, RGB(color.r, color.g, color.b));

            ExtTextOutW(hDC, (int)posX, (int)posY, options, &rect, textW, textWLength, NULL);
        }
    }
}

void dr2d_draw_image_gdi(dr2d_surface* pSurface, dr2d_image* pImage, dr2d_draw_image_args* pArgs)
{
    gdi_image_data* pGDIImageData = (gdi_image_data*)dr2d_get_image_extra_data(pImage);
    if (pGDIImageData == NULL) {
        return;
    }

    gdi_surface_data* pGDISurfaceData = (gdi_surface_data*)dr2d_get_surface_extra_data(pSurface);
    if (pGDISurfaceData == NULL) {
        return;
    }

    bool drawFlipped = false;
    HBITMAP hSrcBitmap = NULL;

    if ((pArgs->options & DR2D_IMAGE_DRAW_BACKGROUND) == 0 && (pArgs->options & DR2D_IMAGE_HINT_NO_ALPHA) != 0 && pArgs->foregroundTint.r == 255 && pArgs->foregroundTint.g == 255 && pArgs->foregroundTint.b == 255)
    {
        // Fast path. No tint, no background, no alpha.
        hSrcBitmap = pGDIImageData->hSrcBitmap;
        drawFlipped = true;
    }
    else
    {
        // Slow path. We need to manually change the color values of the intermediate bitmap and use that as the source when drawing it. This is also flipped.
        unsigned int* pSrcBitmapData = pGDIImageData->pSrcBitmapData;
        unsigned int* pDstBitmapData = pGDIImageData->pIntermediateBitmapData;
        for (unsigned int iRow = 0; iRow < pImage->height; ++iRow)
        {
            for (unsigned int iCol = 0; iCol < pImage->width; ++iCol)
            {
                unsigned int  srcTexel = *(pSrcBitmapData + (iRow * pImage->width) + iCol);
                unsigned int* dstTexel =  (pDstBitmapData + ((pImage->height - iRow - 1) * pImage->width) + iCol);

                unsigned int srcTexelA = (srcTexel & 0xFF000000) >> 24;
                unsigned int srcTexelR = (unsigned int)(((srcTexel & 0x00FF0000) >> 16) * (pArgs->foregroundTint.r / 255.0f));
                unsigned int srcTexelG = (unsigned int)(((srcTexel & 0x0000FF00) >> 8)  * (pArgs->foregroundTint.g / 255.0f));
                unsigned int srcTexelB = (unsigned int)(((srcTexel & 0x000000FF) >> 0)  * (pArgs->foregroundTint.b / 255.0f));

                if (srcTexelR > 255) srcTexelR = 255;
                if (srcTexelG > 255) srcTexelG = 255;
                if (srcTexelB > 255) srcTexelB = 255;

                if ((pArgs->options & DR2D_IMAGE_DRAW_BACKGROUND) != 0)
                {
                    srcTexelB += (unsigned int)(pArgs->backgroundColor.b * ((255 - srcTexelA) / 255.0f));
                    srcTexelG += (unsigned int)(pArgs->backgroundColor.g * ((255 - srcTexelA) / 255.0f));
                    srcTexelR += (unsigned int)(pArgs->backgroundColor.r * ((255 - srcTexelA) / 255.0f));
                    srcTexelA = 0xFF;
                }

                *dstTexel = (srcTexelR << 16) | (srcTexelG << 8) | (srcTexelB << 0) | (srcTexelA << 24);
            }
        }

        // Flush GDI to let it know we are finished with the bitmap object's data.
        GdiFlush();

        // If we have drawn the background manually we don't need to do an alpha blend.
        if ((pArgs->options & DR2D_IMAGE_DRAW_BACKGROUND) != 0) {
            pArgs->options |= DR2D_IMAGE_HINT_NO_ALPHA;
        }

        hSrcBitmap = pGDIImageData->hIntermediateBitmap;
    }

    HGDIOBJ hPrevBitmap = SelectObject(pGDISurfaceData->hIntermediateDC, hSrcBitmap);
    if ((pArgs->options & DR2D_IMAGE_HINT_NO_ALPHA) != 0)
    {
        if (drawFlipped) {
            StretchBlt(pGDISurfaceData->hDC, (int)pArgs->dstX, (int)pArgs->dstY + (int)pArgs->dstHeight - 1, (int)pArgs->dstWidth, -(int)pArgs->dstHeight, pGDISurfaceData->hIntermediateDC, (int)pArgs->srcX, (int)pArgs->srcY, (int)pArgs->srcWidth, (int)pArgs->srcHeight, SRCCOPY);
        } else {
            StretchBlt(pGDISurfaceData->hDC, (int)pArgs->dstX, (int)pArgs->dstY, (int)pArgs->dstWidth, (int)pArgs->dstHeight, pGDISurfaceData->hIntermediateDC, (int)pArgs->srcX, (int)pArgs->srcY, (int)pArgs->srcWidth, (int)pArgs->srcHeight, SRCCOPY);
        }
    }
    else
    {
        assert(drawFlipped == false);   // <-- Error if this is hit.
        BLENDFUNCTION blend = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
        AlphaBlend(pGDISurfaceData->hDC, (int)pArgs->dstX, (int)pArgs->dstY, (int)pArgs->dstWidth, (int)pArgs->dstHeight, pGDISurfaceData->hIntermediateDC, (int)pArgs->srcX, (int)pArgs->srcY, (int)pArgs->srcWidth, (int)pArgs->srcHeight, blend);
    }
    SelectObject(pGDISurfaceData->hIntermediateDC, hPrevBitmap);
}

void dr2d_set_clip_gdi(dr2d_surface* pSurface, float left, float top, float right, float bottom)
{
    assert(pSurface != NULL);

    gdi_surface_data* pGDIData = (gdi_surface_data*)dr2d_get_surface_extra_data(pSurface);
    if (pGDIData != NULL)
    {
        HDC hDC = dr2d_get_HDC(pSurface);

        SelectClipRgn(hDC, NULL);
        IntersectClipRect(hDC, (int)left, (int)top, (int)right, (int)bottom);
    }
}

void dr2d_get_clip_gdi(dr2d_surface* pSurface, float* pLeftOut, float* pTopOut, float* pRightOut, float* pBottomOut)
{
    assert(pSurface != NULL);

    gdi_surface_data* pGDIData = (gdi_surface_data*)dr2d_get_surface_extra_data(pSurface);
    if (pGDIData != NULL)
    {
        RECT rect;
        GetClipBox(dr2d_get_HDC(pSurface), &rect);

        if (pLeftOut != NULL) {
            *pLeftOut = (float)rect.left;
        }
        if (pTopOut != NULL) {
            *pTopOut = (float)rect.top;
        }
        if (pRightOut != NULL) {
            *pRightOut = (float)rect.right;
        }
        if (pBottomOut != NULL) {
            *pBottomOut = (float)rect.bottom;
        }
    }
}


dr2d_image_format dr2d_get_optimal_image_format_gdi(dr2d_context* pContext)
{
    (void)pContext;
    return dr2d_image_format_bgra8;
}

void* dr2d_map_image_data_gdi(dr2d_image* pImage, unsigned accessFlags)
{
    assert(pImage != NULL);

    gdi_image_data* pGDIImageData = (gdi_image_data*)dr2d_get_image_extra_data(pImage);
    if (pGDIImageData == NULL) {
        return NULL;
    }

    assert(pGDIImageData->pMappedImageData == NULL);    // This function should never be called while the image is already mapped.

    pGDIImageData->mapAccessFlags = accessFlags;

    if (pImage->format == dr2d_image_format_bgra8)
    {
        pGDIImageData->pMappedImageData = pGDIImageData->pSrcBitmapData;
    }
    else
    {
        pGDIImageData->pMappedImageData = malloc(pImage->width * pImage->height * 4);
        if (pGDIImageData->pMappedImageData == NULL) {
            return NULL;
        }

        for (unsigned int iRow = 0; iRow < pImage->height; ++iRow)
        {
            const unsigned int iRowSrc = pImage->height - (iRow + 1);
            const unsigned int iRowDst = iRow;

            for (unsigned int iCol = 0; iCol < pImage->width; ++iCol)
            {
                unsigned int  srcTexel = ((const unsigned int*)(pGDIImageData->pSrcBitmapData))[    (iRowSrc * pImage->width) + iCol];
                unsigned int* dstTexel = ((      unsigned int*)(pGDIImageData->pMappedImageData)) + (iRowDst * pImage->width) + iCol;

                unsigned int srcTexelA = (srcTexel & 0xFF000000) >> 24;
                unsigned int srcTexelB = (srcTexel & 0x00FF0000) >> 16;
                unsigned int srcTexelG = (srcTexel & 0x0000FF00) >> 8;
                unsigned int srcTexelR = (srcTexel & 0x000000FF) >> 0;

                srcTexelB = (unsigned int)(srcTexelB * (srcTexelA / 255.0f));
                srcTexelG = (unsigned int)(srcTexelG * (srcTexelA / 255.0f));
                srcTexelR = (unsigned int)(srcTexelR * (srcTexelA / 255.0f));

                *dstTexel = (srcTexelR << 16) | (srcTexelG << 8) | (srcTexelB << 0) | (srcTexelA << 24);
            }
        }
    }

    return pGDIImageData->pMappedImageData;
}

void dr2d_unmap_image_data_gdi(dr2d_image* pImage)
{
    assert(pImage != NULL);

    gdi_image_data* pGDIImageData = (gdi_image_data*)dr2d_get_image_extra_data(pImage);
    if (pGDIImageData == NULL) {
        return;
    }

    assert(pGDIImageData->pMappedImageData != NULL);    // This function should never be called while the image is not mapped.

    if (pImage->format == dr2d_image_format_bgra8)
    {
        // It's in the native format, so just do a flush.
        GdiFlush();
    }
    else
    {
        // Update the actual image data if applicable.
        if (pGDIImageData->mapAccessFlags & DR2D_WRITE) {
            dr2d__rgba8_bgra8_swap__premul(pGDIImageData->pMappedImageData, pGDIImageData->pSrcBitmapData, pImage->width, pImage->height, pImage->width*4, pImage->width*4);
        }

        free(pGDIImageData->pMappedImageData);
    }

    pGDIImageData->pMappedImageData = NULL;
    pGDIImageData->mapAccessFlags = 0;
}


bool dr2d_get_font_metrics_gdi(dr2d_font* pFont, dr2d_font_metrics* pMetricsOut)
{
    assert(pFont != NULL);
    assert(pMetricsOut != NULL);

    gdi_font_data* pGDIFontData = (gdi_font_data*)dr2d_get_font_extra_data(pFont);
    if (pGDIFontData == NULL) {
        return false;
    }

    *pMetricsOut = pGDIFontData->metrics;
    return true;
}

bool dr2d_get_glyph_metrics_gdi(dr2d_font* pFont, unsigned int utf32, dr2d_glyph_metrics* pGlyphMetrics)
{
    assert(pFont != NULL);
    assert(pGlyphMetrics != NULL);

    gdi_font_data* pGDIFontData = (gdi_font_data*)dr2d_get_font_extra_data(pFont);
    if (pGDIFontData == NULL) {
        return false;
    }

    gdi_context_data* pGDIContextData = (gdi_context_data*)dr2d_get_context_extra_data(pFont->pContext);
    if (pGDIContextData == NULL) {
        return false;
    }


    SelectObject(pGDIContextData->hDC, pGDIFontData->hFont);


    const MAT2 transform = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};        // <-- Identity matrix

    unsigned short utf16[2];
    int utf16Len = dr2d_utf32_to_utf16(utf32, utf16);

    WCHAR glyphIndices[2];

    GCP_RESULTSW glyphResults;
    ZeroMemory(&glyphResults, sizeof(glyphResults));
    glyphResults.lStructSize = sizeof(glyphResults);
    glyphResults.lpGlyphs = glyphIndices;
    glyphResults.nGlyphs  = 2;
    if (GetCharacterPlacementW(pGDIContextData->hDC, (LPCWSTR)utf16, utf16Len, 0, &glyphResults, 0) != 0)
    {
        GLYPHMETRICS metrics;
        DWORD bitmapBufferSize = GetGlyphOutlineW(pGDIContextData->hDC, glyphIndices[0], GGO_NATIVE | GGO_GLYPH_INDEX, &metrics, 0, NULL, &transform);
        if (bitmapBufferSize != GDI_ERROR)
        {
            pGlyphMetrics->width    = metrics.gmBlackBoxX;
            pGlyphMetrics->height   = metrics.gmBlackBoxY;
            pGlyphMetrics->originX  = metrics.gmptGlyphOrigin.x;
            pGlyphMetrics->originY  = metrics.gmptGlyphOrigin.y;
            pGlyphMetrics->advanceX = metrics.gmCellIncX;
            pGlyphMetrics->advanceY = metrics.gmCellIncY;

            return true;
        }
    }

    return false;
}

bool dr2d_measure_string_gdi(dr2d_font* pFont, const char* text, size_t textSizeInBytes, float* pWidthOut, float* pHeightOut)
{
    assert(pFont != NULL);

    gdi_font_data* pGDIFontData = (gdi_font_data*)dr2d_get_font_extra_data(pFont);
    if (pGDIFontData == NULL) {
        return false;
    }

    gdi_context_data* pGDIContextData = (gdi_context_data*)dr2d_get_context_extra_data(pFont->pContext);
    if (pGDIContextData == NULL) {
        return false;
    }


    SelectObject(pGDIContextData->hDC, pGDIFontData->hFont);

    unsigned int textWLength;
    wchar_t* textW = dr2d_to_wchar_gdi(pFont->pContext, text, textSizeInBytes, &textWLength);
    if (textW != NULL)
    {
        SIZE sizeWin32;
        if (GetTextExtentPoint32W(pGDIContextData->hDC, textW, textWLength, &sizeWin32))
        {
            if (pWidthOut != NULL) {
                *pWidthOut = (float)sizeWin32.cx;
            }
            if (pHeightOut != NULL) {
                *pHeightOut = (float)sizeWin32.cy;
            }

            return true;
        }
    }

    return false;
}

bool dr2d_get_text_cursor_position_from_point_gdi(dr2d_font* pFont, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosXOut, size_t* pCharacterIndexOut)
{
    bool successful = false;

    assert(pFont != NULL);

    gdi_font_data* pGDIFontData = (gdi_font_data*)dr2d_get_font_extra_data(pFont);
    if (pGDIFontData == NULL) {
        return false;
    }

    gdi_context_data* pGDIContextData = (gdi_context_data*)dr2d_get_context_extra_data(pFont->pContext);
    if (pGDIContextData == NULL) {
        return false;
    }


    SelectObject(pGDIContextData->hDC, pGDIFontData->hFont);


    GCP_RESULTSW results;
    ZeroMemory(&results, sizeof(results));
    results.lStructSize = sizeof(results);
    results.nGlyphs     = (UINT)textSizeInBytes;

    unsigned int textWLength;
    wchar_t* textW = dr2d_to_wchar_gdi(pFont->pContext, text, textSizeInBytes, &textWLength);
    if (textW != NULL)
    {
        if (results.nGlyphs > pGDIContextData->glyphCacheSize) {
            free(pGDIContextData->pGlyphCache);
            pGDIContextData->pGlyphCache = (int*)malloc(sizeof(int) * results.nGlyphs);
            pGDIContextData->glyphCacheSize = results.nGlyphs;
        }

        results.lpCaretPos = pGDIContextData->pGlyphCache;
        if (results.lpCaretPos != NULL)
        {
            if (GetCharacterPlacementW(pGDIContextData->hDC, textW, results.nGlyphs, (int)maxWidth, &results, GCP_MAXEXTENT | GCP_USEKERNING) != 0)
            {
                float textCursorPosX = 0;
                unsigned int iChar;
                for (iChar = 0; iChar < results.nGlyphs; ++iChar)
                {
                    float charBoundsLeft  = charBoundsLeft = (float)results.lpCaretPos[iChar];
                    float charBoundsRight = 0;
                    if (iChar < results.nGlyphs - 1) {
                        charBoundsRight = (float)results.lpCaretPos[iChar + 1];
                    } else {
                        charBoundsRight = maxWidth;
                    }

                    if (inputPosX >= charBoundsLeft && inputPosX <= charBoundsRight)
                    {
                        // The input position is somewhere on top of this character. If it's positioned on the left side of the character, set the output
                        // value to the character at iChar. Otherwise it should be set to the character at iChar + 1.
                        float charBoundsRightHalf = charBoundsLeft + ceilf(((charBoundsRight - charBoundsLeft) / 2.0f));
                        if (inputPosX <= charBoundsRightHalf) {
                            break;
                        } else {
                            textCursorPosX = charBoundsRight;
                            iChar += 1;
                            break;
                        }
                    }

                    textCursorPosX = charBoundsRight;
                }

                if (pTextCursorPosXOut) {
                    *pTextCursorPosXOut = textCursorPosX;
                }
                if (pCharacterIndexOut) {
                    *pCharacterIndexOut = iChar;
                }

                successful = true;
            }
        }
    }

    return successful;
}

bool dr2d_get_text_cursor_position_from_char_gdi(dr2d_font* pFont, const char* text, size_t characterIndex, float* pTextCursorPosXOut)
{
    bool successful = false;

    assert(pFont != NULL);

    gdi_font_data* pGDIFontData = (gdi_font_data*)dr2d_get_font_extra_data(pFont);
    if (pGDIFontData == NULL) {
        return false;
    }

    gdi_context_data* pGDIContextData = (gdi_context_data*)dr2d_get_context_extra_data(pFont->pContext);
    if (pGDIContextData == NULL) {
        return false;
    }


    SelectObject(pGDIContextData->hDC, pGDIFontData->hFont);


    GCP_RESULTSW results;
    ZeroMemory(&results, sizeof(results));
    results.lStructSize = sizeof(results);
    results.nGlyphs     = (DWORD)(characterIndex + 1);

    unsigned int textWLength;
    wchar_t* textW = dr2d_to_wchar_gdi(pFont->pContext, text, (int)results.nGlyphs, &textWLength);
    if (textW != NULL)
    {
        if (results.nGlyphs > pGDIContextData->glyphCacheSize) {
            free(pGDIContextData->pGlyphCache);
            pGDIContextData->pGlyphCache = (int*)malloc(sizeof(int) * results.nGlyphs);
            pGDIContextData->glyphCacheSize = results.nGlyphs;
        }

        results.lpCaretPos = pGDIContextData->pGlyphCache;
        if (results.lpCaretPos != NULL)
        {
            if (GetCharacterPlacementW(pGDIContextData->hDC, textW, results.nGlyphs, 0, &results, GCP_USEKERNING) != 0)
            {
                if (pTextCursorPosXOut) {
                    *pTextCursorPosXOut = (float)results.lpCaretPos[characterIndex];
                }

                successful = true;
            }
        }
    }

    return successful;
}


wchar_t* dr2d_to_wchar_gdi(dr2d_context* pContext, const char* text, size_t textSizeInBytes, unsigned int* characterCountOut)
{
    if (pContext == NULL || text == NULL) {
        return NULL;
    }

    gdi_context_data* pGDIData = (gdi_context_data*)dr2d_get_context_extra_data(pContext);
    if (pGDIData == NULL) {
        return NULL;
    }

    int wcharCount = 0;


    // We first try to copy the string into the already-allocated buffer. If it fails we fall back to the slow path which requires
    // two conversions.
    if (pGDIData->wcharBuffer == NULL) {
        goto fallback;
    }

    wcharCount = MultiByteToWideChar(CP_UTF8, 0, text, (int)textSizeInBytes, pGDIData->wcharBuffer, pGDIData->wcharBufferLength);
    if (wcharCount != 0) {
        if (characterCountOut) *characterCountOut = wcharCount;
        return pGDIData->wcharBuffer;
    }

    

fallback:;
    wcharCount = MultiByteToWideChar(CP_UTF8, 0, text, (int)textSizeInBytes, NULL, 0);
    if (wcharCount == 0) {
        return NULL;
    }

    if (pGDIData->wcharBufferLength < (unsigned int)wcharCount + 1) {
        free(pGDIData->wcharBuffer);
        pGDIData->wcharBuffer       = (wchar_t*)malloc(sizeof(wchar_t) * (wcharCount + 1));
        pGDIData->wcharBufferLength = wcharCount + 1;
    }

    wcharCount = MultiByteToWideChar(CP_UTF8, 0, text, (int)textSizeInBytes, pGDIData->wcharBuffer, pGDIData->wcharBufferLength);
    if (wcharCount == 0) {
        return NULL;
    }


    if (characterCountOut != NULL) {
        *characterCountOut = wcharCount;
    }

    return pGDIData->wcharBuffer;
}

#endif  // GDI


/////////////////////////////////////////////////////////////////
//
// CAIRO 2D API
//
/////////////////////////////////////////////////////////////////
#ifndef DR2D_NO_CAIRO

typedef struct
{
    cairo_surface_t* pCairoSurface;
    cairo_t* pCairoContext;

    float clipRectLeft;
    float clipRectTop;
    float clipRectRight;
    float clipRectBottom;

} cairo_surface_data;

typedef struct
{
    cairo_font_face_t* pFace;
    cairo_scaled_font_t* pFont;

    // The font metrics. This is initialized when the font is created.
    dr2d_font_metrics metrics;

} cairo_font_data;

typedef struct
{
    /// Images in Cairo are implemented as surfaces.
    cairo_surface_t* pCairoSurface;

    /// A pointer to the raw data.
    unsigned char* pData;

} cairo_image_data;

bool dr2d_on_create_context_cairo(dr2d_context* pContext, const void* pUserData);
void dr2d_on_delete_context_cairo(dr2d_context* pContext);
bool dr2d_on_create_surface_cairo(dr2d_surface* pSurface, float width, float height);
void dr2d_on_delete_surface_cairo(dr2d_surface* pSurface);
bool dr2d_on_create_font_cairo(dr2d_font* pFont);
void dr2d_on_delete_font_cairo(dr2d_font* pFont);
bool dr2d_on_create_image_cairo(dr2d_image* pImage, unsigned int stride, const void* pData);
void dr2d_on_delete_image_cairo(dr2d_image* pImage);

void dr2d_begin_draw_cairo(dr2d_surface* pSurface);
void dr2d_end_draw_cairo(dr2d_surface* pSurface);
void dr2d_clear_cairo(dr2d_surface* pSurface, dr2d_color color);
void dr2d_draw_rect_cairo(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color);
void dr2d_draw_rect_outline_cairo(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float outlineWidth);
void dr2d_draw_rect_with_outline_cairo(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float outlineWidth, dr2d_color outlineColor);
void dr2d_draw_round_rect_cairo(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float radius);
void dr2d_draw_round_rect_outline_cairo(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float radius, float outlineWidth);
void dr2d_draw_round_rect_with_outline_cairo(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float radius, float outlineWidth, dr2d_color outlineColor);
void dr2d_draw_text_cairo(dr2d_surface* pSurface, dr2d_font* pFont, const char* text, size_t textSizeInBytes, float posX, float posY, dr2d_color color, dr2d_color backgroundColor);
void dr2d_draw_image_cairo(dr2d_surface* pSurface, dr2d_image* pImage, dr2d_draw_image_args* pArgs);
void dr2d_set_clip_cairo(dr2d_surface* pSurface, float left, float top, float right, float bottom);
void dr2d_get_clip_cairo(dr2d_surface* pSurface, float* pLeftOut, float* pTopOut, float* pRightOut, float* pBottomOut);

dr2d_image_format dr2d_get_optimal_image_format_cairo(dr2d_context* pContext);
void* dr2d_map_image_data_cairo(dr2d_image* pImage, unsigned accessFlags);
void dr2d_unmap_image_data_cairo(dr2d_image* pImage);

bool dr2d_get_font_metrics_cairo(dr2d_font* pFont, dr2d_font_metrics* pMetricsOut);
bool dr2d_get_glyph_metrics_cairo(dr2d_font* pFont, unsigned int utf32, dr2d_glyph_metrics* pGlyphMetrics);
bool dr2d_measure_string_cairo(dr2d_font* pFont, const char* text, size_t textSizeInBytes, float* pWidthOut, float* pHeightOut);
bool dr2d_get_text_cursor_position_from_point_cairo(dr2d_font* pFont, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosXOut, size_t* pCharacterIndexOut);
bool dr2d_get_text_cursor_position_from_char_cairo(dr2d_font* pFont, const char* text, size_t characterIndex, float* pTextCursorPosXOut);


dr2d_context* dr2d_create_context_cairo()
{
    dr2d_drawing_callbacks callbacks;
    callbacks.on_create_context                   = dr2d_on_create_context_cairo;
    callbacks.on_delete_context                   = dr2d_on_delete_context_cairo;
    callbacks.on_create_surface                   = dr2d_on_create_surface_cairo;
    callbacks.on_delete_surface                   = dr2d_on_delete_surface_cairo;
    callbacks.on_create_font                      = dr2d_on_create_font_cairo;
    callbacks.on_delete_font                      = dr2d_on_delete_font_cairo;
    callbacks.on_create_image                     = dr2d_on_create_image_cairo;
    callbacks.on_delete_image                     = dr2d_on_delete_image_cairo;

    callbacks.begin_draw                          = dr2d_begin_draw_cairo;
    callbacks.end_draw                            = dr2d_end_draw_cairo;
    callbacks.clear                               = dr2d_clear_cairo;
    callbacks.draw_rect                           = dr2d_draw_rect_cairo;
    callbacks.draw_rect_outline                   = dr2d_draw_rect_outline_cairo;
    callbacks.draw_rect_with_outline              = dr2d_draw_rect_with_outline_cairo;
    callbacks.draw_round_rect                     = dr2d_draw_round_rect_cairo;
    callbacks.draw_round_rect_outline             = dr2d_draw_round_rect_outline_cairo;
    callbacks.draw_round_rect_with_outline        = dr2d_draw_round_rect_with_outline_cairo;
    callbacks.draw_text                           = dr2d_draw_text_cairo;
    callbacks.draw_image                          = dr2d_draw_image_cairo;
    callbacks.set_clip                            = dr2d_set_clip_cairo;
    callbacks.get_clip                            = dr2d_get_clip_cairo;

    callbacks.map_image_data                      = dr2d_map_image_data_cairo;
    callbacks.unmap_image_data                    = dr2d_unmap_image_data_cairo;

    callbacks.get_font_metrics                    = dr2d_get_font_metrics_cairo;
    callbacks.get_glyph_metrics                   = dr2d_get_glyph_metrics_cairo;
    callbacks.measure_string                      = dr2d_measure_string_cairo;
    callbacks.get_text_cursor_position_from_point = dr2d_get_text_cursor_position_from_point_cairo;
    callbacks.get_text_cursor_position_from_char  = dr2d_get_text_cursor_position_from_char_cairo;


    return dr2d_create_context(callbacks, 0, sizeof(cairo_surface_data), sizeof(cairo_font_data), sizeof(cairo_image_data), NULL);
}

dr2d_surface* dr2d_create_surface_cairo(dr2d_context* pContext, cairo_t* cr)
{
    if (cr == NULL) {
        return NULL;
    }

    dr2d_surface* pSurface = dr2d_create_surface(pContext, 0, 0);
    if (pSurface != NULL) {
        cairo_surface_data* pCairoData = (cairo_surface_data*)dr2d_get_surface_extra_data(pSurface);
        if (pCairoData != NULL) {
            pCairoData->pCairoContext = cairo_reference(cr);
            pCairoData->pCairoSurface = cairo_surface_reference(cairo_get_target(cr));
        }
    }

    return pSurface;
}

cairo_surface_t* dr2d_get_cairo_surface_t(dr2d_surface* pSurface)
{
    cairo_surface_data* pCairoData = dr2d_get_surface_extra_data(pSurface);
    if (pCairoData != NULL) {
        return pCairoData->pCairoSurface;
    }

    return NULL;
}

cairo_t* dr2d_get_cairo_t(dr2d_surface* pSurface)
{
    cairo_surface_data* pCairoData = dr2d_get_surface_extra_data(pSurface);
    if (pCairoData != NULL) {
        return pCairoData->pCairoContext;
    }

    return NULL;
}


bool dr2d_on_create_context_cairo(dr2d_context* pContext, const void* pUserData)
{
    assert(pContext != NULL);
    (void)pContext;
    (void)pUserData;

    return true;
}

void dr2d_on_delete_context_cairo(dr2d_context* pContext)
{
    assert(pContext != NULL);
    (void)pContext;
}

bool dr2d_on_create_surface_cairo(dr2d_surface* pSurface, float width, float height)
{
    assert(pSurface != NULL);

    cairo_surface_data* pCairoData = dr2d_get_surface_extra_data(pSurface);
    if (pCairoData == NULL) {
        return false;
    }

    if (width != 0 && height != 0) {
        pCairoData->pCairoSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, (int)width, (int)height);
        if (pCairoData->pCairoSurface == NULL) {
            return false;
        }

        pCairoData->pCairoContext = cairo_create(pCairoData->pCairoSurface);
        if (pCairoData->pCairoContext == NULL) {
            cairo_surface_destroy(pCairoData->pCairoSurface);
            return false;
        }
    } else {
        pCairoData->pCairoSurface = NULL;
        pCairoData->pCairoContext = NULL;
    }


    return true;
}

void dr2d_on_delete_surface_cairo(dr2d_surface* pSurface)
{
    assert(pSurface != NULL);

    cairo_surface_data* pCairoData = dr2d_get_surface_extra_data(pSurface);
    if (pCairoData != NULL)
    {
        cairo_destroy(pCairoData->pCairoContext);
        cairo_surface_destroy(pCairoData->pCairoSurface);
    }
}

bool dr2d_on_create_font_cairo(dr2d_font* pFont)
{
    cairo_font_data* pCairoFont = dr2d_get_font_extra_data(pFont);
    if (pCairoFont == NULL) {
        return false;
    }

    cairo_font_slant_t cairoSlant = CAIRO_FONT_SLANT_NORMAL;
    if (pFont->slant == dr2d_font_slant_italic) {
        cairoSlant = CAIRO_FONT_SLANT_ITALIC;
    } else if (pFont->slant == dr2d_font_slant_oblique) {
        cairoSlant = CAIRO_FONT_SLANT_OBLIQUE;
    }

    cairo_font_weight_t cairoWeight = CAIRO_FONT_WEIGHT_NORMAL;
    if (pFont->weight == dr2d_font_weight_bold || pFont->weight == dr2d_font_weight_semi_bold || pFont->weight == dr2d_font_weight_extra_bold || pFont->weight == dr2d_font_weight_heavy) {
        cairoWeight = CAIRO_FONT_WEIGHT_BOLD;
    }

    pCairoFont->pFace = cairo_toy_font_face_create(pFont->family, cairoSlant, cairoWeight);
    if (pCairoFont->pFace == NULL) {
        return false;
    }

    cairo_matrix_t fontMatrix;
    cairo_matrix_init_scale(&fontMatrix, (double)pFont->size, (double)pFont->size);
    cairo_matrix_rotate(&fontMatrix, pFont->rotation * (3.14159265 / 180.0));

    cairo_matrix_t ctm;
    cairo_matrix_init_identity(&ctm);

    cairo_font_options_t* options = cairo_font_options_create();
    cairo_font_options_set_antialias(options, CAIRO_ANTIALIAS_SUBPIXEL);    // TODO: Control this with option flags in pFont.

    pCairoFont->pFont = cairo_scaled_font_create(pCairoFont->pFace, &fontMatrix, &ctm, options);
    if (pCairoFont->pFont == NULL) {
        cairo_font_face_destroy(pCairoFont->pFace);
        return false;
    }


    // Metrics.
    cairo_font_extents_t fontMetrics;
    cairo_scaled_font_extents(pCairoFont->pFont, &fontMetrics);

    pCairoFont->metrics.ascent     = fontMetrics.ascent;
    pCairoFont->metrics.descent    = fontMetrics.descent;
    //pCairoFont->metrics.lineHeight = fontMetrics.height;
    pCairoFont->metrics.lineHeight = fontMetrics.ascent + fontMetrics.descent;

    // The width of a space needs to be retrieved via glyph metrics.
    const char space[] = " ";
    cairo_text_extents_t spaceMetrics;
    cairo_scaled_font_text_extents(pCairoFont->pFont, space, &spaceMetrics);
    pCairoFont->metrics.spaceWidth = spaceMetrics.x_advance;

    return true;
}

void dr2d_on_delete_font_cairo(dr2d_font* pFont)
{
    cairo_font_data* pCairoFont = dr2d_get_font_extra_data(pFont);
    if (pCairoFont == NULL) {
        return;
    }

    cairo_scaled_font_destroy(pCairoFont->pFont);
    cairo_font_face_destroy(pCairoFont->pFace);
}

bool dr2d_on_create_image_cairo(dr2d_image* pImage, unsigned int stride, const void* pData)
{
    cairo_image_data* pCairoImage = dr2d_get_image_extra_data(pImage);
    if (pCairoImage == NULL) {
        return false;
    }

    size_t dataSize = pImage->height * pImage->width * 4;
    pCairoImage->pData = malloc(dataSize);
    if (pCairoImage->pData == NULL) {
        return false;
    }

    if (pData != NULL)
    {
        for (unsigned int iRow = 0; iRow < pImage->height; ++iRow)
        {
            const unsigned int iRowSrc = iRow; //pImage->height - (iRow + 1);
            const unsigned int iRowDst = iRow;

            for (unsigned int iCol = 0; iCol < pImage->width; ++iCol)
            {
                unsigned int  srcTexel = ((const unsigned int*)(pData             ))[  (iRowSrc * (stride/4))    + iCol];
                unsigned int* dstTexel = ((      unsigned int*)(pCairoImage->pData)) + (iRowDst * pImage->width) + iCol;

                unsigned int srcTexelA = (srcTexel & 0xFF000000) >> 24;
                unsigned int srcTexelB = (srcTexel & 0x00FF0000) >> 16;
                unsigned int srcTexelG = (srcTexel & 0x0000FF00) >> 8;
                unsigned int srcTexelR = (srcTexel & 0x000000FF) >> 0;

                srcTexelB = (unsigned int)(srcTexelB * (srcTexelA / 255.0f));
                srcTexelG = (unsigned int)(srcTexelG * (srcTexelA / 255.0f));
                srcTexelR = (unsigned int)(srcTexelR * (srcTexelA / 255.0f));

                *dstTexel = (srcTexelR << 16) | (srcTexelG << 8) | (srcTexelB << 0) | (srcTexelA << 24);
            }
        }
    }

    pCairoImage->pCairoSurface = cairo_image_surface_create_for_data(pCairoImage->pData, CAIRO_FORMAT_ARGB32, (int)pImage->width, (int)pImage->height, (int)pImage->width*4);
    if (pCairoImage->pCairoSurface == NULL) {
        free(pCairoImage->pData);
        return false;
    }

    return true;
}

void dr2d_on_delete_image_cairo(dr2d_image* pImage)
{
    cairo_image_data* pCairoImage = dr2d_get_image_extra_data(pImage);
    if (pCairoImage == NULL) {
        return;
    }

    cairo_surface_destroy(pCairoImage->pCairoSurface);
    free(pCairoImage->pData);
}


void dr2d_begin_draw_cairo(dr2d_surface* pSurface)
{
    assert(pSurface != NULL);

    cairo_surface_data* pCairoData = dr2d_get_surface_extra_data(pSurface);
    if (pCairoData == NULL) {
        return;
    }

    cairo_set_antialias(pCairoData->pCairoContext, CAIRO_ANTIALIAS_NONE);
}

void dr2d_end_draw_cairo(dr2d_surface* pSurface)
{
    assert(pSurface != NULL);

    cairo_surface_data* pCairoData = dr2d_get_surface_extra_data(pSurface);
    if (pCairoData == NULL) {
        return;
    }

    cairo_set_antialias(pCairoData->pCairoContext, CAIRO_ANTIALIAS_DEFAULT);
}

void dr2d_clear_cairo(dr2d_surface* pSurface, dr2d_color color)
{
    // TODO: I forget... is this supposed to ignore the current clip? If so, this needs to be changed so that
    //       the clip is reset first then restored afterwards.
    dr2d_draw_rect_cairo(pSurface, 0, 0, dr2d_get_surface_width(pSurface), dr2d_get_surface_height(pSurface), color);
}

void dr2d_draw_rect_cairo(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color)
{
    assert(pSurface != NULL);

    cairo_surface_data* pCairoData = dr2d_get_surface_extra_data(pSurface);
    if (pCairoData != NULL)
    {
        cairo_set_source_rgba(pCairoData->pCairoContext, color.r / 255.0, color.g / 255.0, color.b / 255.0, color.a / 255.0);
        cairo_rectangle(pCairoData->pCairoContext, left, top, (right - left), (bottom - top));
        cairo_fill(pCairoData->pCairoContext);
    }
}

void dr2d_draw_rect_outline_cairo(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float outlineWidth)
{
    cairo_surface_data* pCairoData = dr2d_get_surface_extra_data(pSurface);
    if (pCairoData == NULL) {
        return;
    }

    cairo_t* cr = pCairoData->pCairoContext;

    cairo_set_source_rgba(cr, color.r / 255.0, color.g / 255.0, color.b / 255.0, color.a / 255.0);

    // We do this as 4 separate rectangles.
    cairo_rectangle(cr, left, top, outlineWidth, bottom - top);                                                     // Left
    cairo_fill(cr);
    cairo_rectangle(cr, right - outlineWidth, top, outlineWidth, bottom - top);                                     // Right
    cairo_fill(cr);
    cairo_rectangle(cr, left + outlineWidth, top, right - left - (outlineWidth*2), outlineWidth);                   // Top
    cairo_fill(cr);
    cairo_rectangle(cr, left + outlineWidth, bottom - outlineWidth, right - left - (outlineWidth*2), outlineWidth); // Bottom
    cairo_fill(cr);
}

void dr2d_draw_rect_with_outline_cairo(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float outlineWidth, dr2d_color outlineColor)
{
    dr2d_draw_rect_cairo(pSurface, left + outlineWidth, top + outlineWidth, right - outlineWidth, bottom - outlineWidth, color);
    dr2d_draw_rect_outline_cairo(pSurface, left, top, right, bottom, outlineColor, outlineWidth);
}

void dr2d_draw_round_rect_cairo(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float radius)
{
    // FIXME: This does not draw rounded corners.
    (void)radius;

    dr2d_draw_rect_cairo(pSurface, left, top, right, bottom, color);
}

void dr2d_draw_round_rect_outline_cairo(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float radius, float outlineWidth)
{
    // FIXME: This does not draw rounded corners.
    (void)radius;

    dr2d_draw_rect_outline_cairo(pSurface, left, top, right, bottom, color, outlineWidth);
}

void dr2d_draw_round_rect_with_outline_cairo(dr2d_surface* pSurface, float left, float top, float right, float bottom, dr2d_color color, float radius, float outlineWidth, dr2d_color outlineColor)
{
    // FIXME: This does not draw rounded corners.
    (void)radius;

    dr2d_draw_rect_with_outline_cairo(pSurface, left, top, right, bottom, color, outlineWidth, outlineColor);
}

void dr2d_draw_text_cairo(dr2d_surface* pSurface, dr2d_font* pFont, const char* text, size_t textSizeInBytes, float posX, float posY, dr2d_color color, dr2d_color backgroundColor)
{
    cairo_surface_data* pCairoSurface = dr2d_get_surface_extra_data(pSurface);
    if (pCairoSurface == NULL) {
        return;
    }

    cairo_t* cr = pCairoSurface->pCairoContext;


    cairo_font_data* pCairoFont = dr2d_get_font_extra_data(pFont);
    if (pCairoFont == NULL) {
        return;
    }

    // Cairo expends null terminated strings, however the input string is not guaranteed to be null terminated.
    char* textNT;
    if (textSizeInBytes != (size_t)-1) {
        textNT = malloc(textSizeInBytes + 1);
        memcpy(textNT, text, textSizeInBytes);
        textNT[textSizeInBytes] = '\0';
    } else {
        textNT = (char*)text;
    }


    cairo_set_scaled_font(cr, pCairoFont->pFont);



    // Background.
    cairo_text_extents_t textMetrics;
    cairo_text_extents(cr, textNT, &textMetrics);
    cairo_set_source_rgba(cr, backgroundColor.r / 255.0, backgroundColor.g / 255.0, backgroundColor.b / 255.0, backgroundColor.a / 255.0);
    cairo_rectangle(cr, posX, posY, textMetrics.x_advance, pCairoFont->metrics.lineHeight);
    cairo_fill(cr);


    // Text.
    cairo_move_to(cr, posX, posY + pCairoFont->metrics.ascent);
    cairo_set_source_rgba(cr, color.r / 255.0, color.g / 255.0, color.b / 255.0, color.a / 255.0);
    cairo_show_text(cr, textNT);


    if (textNT != text) {
        free(textNT);
    }
}

void dr2d_draw_image_cairo(dr2d_surface* pSurface, dr2d_image* pImage, dr2d_draw_image_args* pArgs)
{
    cairo_surface_data* pCairoSurface = dr2d_get_surface_extra_data(pSurface);
    if (pCairoSurface == NULL) {
        return;
    }

    cairo_image_data* pCairoImage = dr2d_get_image_extra_data(pImage);
    if (pCairoImage == NULL) {
        return;
    }

    cairo_t* cr = pCairoSurface->pCairoContext;

    cairo_save(cr);
    cairo_translate(cr, pArgs->dstX, pArgs->dstY);

    // Background.
    if ((pArgs->options & DR2D_IMAGE_DRAW_BACKGROUND) != 0)
    {
        cairo_set_source_rgba(cr, pArgs->backgroundColor.r / 255.0, pArgs->backgroundColor.g / 255.0, pArgs->backgroundColor.b / 255.0, pArgs->backgroundColor.a / 255.0);
        cairo_rectangle(cr, 0, 0, pArgs->dstWidth, pArgs->dstHeight);
        cairo_fill(cr);
    }

    if (pArgs->foregroundTint.r == 255 && pArgs->foregroundTint.g == 255 && pArgs->foregroundTint.b == 255 && pArgs->foregroundTint.a == 255) {
        cairo_scale(cr, pArgs->dstWidth / pArgs->srcWidth, pArgs->dstHeight / pArgs->srcHeight);
        cairo_set_source_surface(cr, pCairoImage->pCairoSurface, pArgs->srcX, pArgs->srcY);
        cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_NEAREST);
        cairo_paint(cr);
    } else {
        // Slower path. The image needs to be tinted. We create a temporary image for this.
        // NOTE: This is incorrect. It's just a temporary solution until I figure out a better way.
        cairo_surface_t* pTempImageSurface = cairo_surface_create_similar_image(pCairoImage->pCairoSurface, CAIRO_FORMAT_ARGB32,
            cairo_image_surface_get_width(pCairoImage->pCairoSurface), cairo_image_surface_get_height(pCairoImage->pCairoSurface));
        if (pTempImageSurface != NULL) {
            cairo_t* cr2 = cairo_create(pTempImageSurface);

            cairo_set_operator(cr2, CAIRO_OPERATOR_SOURCE);
            cairo_set_source_surface(cr2, pCairoImage->pCairoSurface, 0, 0);
            cairo_pattern_set_filter(cairo_get_source(cr2), CAIRO_FILTER_NEAREST);
            cairo_paint(cr2);

            // Tint.
            cairo_set_operator(cr2, CAIRO_OPERATOR_ATOP);
            cairo_set_source_rgba(cr2, pArgs->foregroundTint.r / 255.0, pArgs->foregroundTint.g / 255.0, pArgs->foregroundTint.b / 255.0, 1);
            cairo_rectangle(cr2, 0, 0, pArgs->dstWidth, pArgs->dstHeight);
            cairo_fill(cr2);

            /*cairo_set_operator(cr2, CAIRO_OPERATOR_MULTIPLY);
            cairo_set_source_surface(cr2, pCairoImage->pCairoSurface, 0, 0);
            cairo_pattern_set_filter(cairo_get_source(cr2), CAIRO_FILTER_NEAREST);
            cairo_paint(cr2);*/

            // Draw the temporary surface onto the main surface.
            cairo_scale(cr, pArgs->dstWidth / pArgs->srcWidth, pArgs->dstHeight / pArgs->srcHeight);
            cairo_set_source_surface(cr, pTempImageSurface, pArgs->srcX, pArgs->srcY);
            cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_NEAREST);
            //cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
            cairo_paint(cr);

            cairo_destroy(cr2);
            cairo_surface_destroy(pTempImageSurface);
        }
    }

    cairo_restore(cr);
}

void dr2d_set_clip_cairo(dr2d_surface* pSurface, float left, float top, float right, float bottom)
{
    cairo_surface_data* pCairoData = dr2d_get_surface_extra_data(pSurface);
    if (pCairoData == NULL) {
        return;
    }

    pCairoData->clipRectLeft   = left;
    pCairoData->clipRectTop    = top;
    pCairoData->clipRectRight  = right;
    pCairoData->clipRectBottom = bottom;

    cairo_reset_clip(pCairoData->pCairoContext);
    cairo_rectangle(pCairoData->pCairoContext, left, top, right - left, bottom - top);
    cairo_clip(pCairoData->pCairoContext);
}

void dr2d_get_clip_cairo(dr2d_surface* pSurface, float* pLeftOut, float* pTopOut, float* pRightOut, float* pBottomOut)
{
    (void)pSurface;
    (void)pLeftOut;
    (void)pTopOut;
    (void)pRightOut;
    (void)pBottomOut;

    cairo_surface_data* pCairoData = dr2d_get_surface_extra_data(pSurface);
    if (pCairoData == NULL) {
        return;
    }

    if (pLeftOut)   { *pLeftOut   = pCairoData->clipRectLeft;   }
    if (pTopOut)    { *pTopOut    = pCairoData->clipRectTop;    }
    if (pRightOut)  { *pRightOut  = pCairoData->clipRectRight;  }
    if (pBottomOut) { *pBottomOut = pCairoData->clipRectBottom; }
}


dr2d_image_format dr2d_get_optimal_image_format_cairo(dr2d_context* pContext)
{
    (void)pContext;
    return dr2d_image_format_argb8;
}

void* dr2d_map_image_data_cairo(dr2d_image* pImage, unsigned accessFlags)
{
    (void)pImage;
    (void)accessFlags;
    return NULL;
}

void dr2d_unmap_image_data_cairo(dr2d_image* pImage)
{
    (void)pImage;
}


bool dr2d_get_font_metrics_cairo(dr2d_font* pFont, dr2d_font_metrics* pMetricsOut)
{
    cairo_font_data* pCairoFont = dr2d_get_font_extra_data(pFont);
    if (pCairoFont == NULL) {
        return false;
    }

    if (pMetricsOut) {
        *pMetricsOut = pCairoFont->metrics;
    }

    return true;
}

static size_t dr2d__utf32_to_utf8(unsigned int utf32, char* utf8, size_t utf8Size)
{
    // NOTE: This function is untested.

    size_t utf8ByteCount = 0;
    if (utf32 < 0x80) {
        utf8ByteCount = 1;
    } else if (utf32 < 0x800) {
        utf8ByteCount = 2;
    } else if (utf32 < 0x10000) {
        utf8ByteCount = 3;
    } else if (utf32 < 0x110000) {
        utf8ByteCount = 4;
    }

    if (utf8ByteCount > utf8Size) {
        if (utf8 != NULL && utf8Size > 0) {
            utf8[0] = '\0';
        }
        return 0;
    }

    utf8 += utf8ByteCount;
    if (utf8ByteCount < utf8Size) {
        utf8[0] = '\0'; // Null terminate.
    }

    const unsigned char firstByteMark[7] = {0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};
    switch (utf8ByteCount)
    {
        case 4: *--utf8 = (char)((utf32 | 0x80) & 0xBF); utf32 >>= 6;
        case 3: *--utf8 = (char)((utf32 | 0x80) & 0xBF); utf32 >>= 6;
        case 2: *--utf8 = (char)((utf32 | 0x80) & 0xBF); utf32 >>= 6;
        case 1: *--utf8 = (char)(utf32 | firstByteMark[utf8ByteCount]);
        default: break;
    }

    return utf8ByteCount;
}

bool dr2d_get_glyph_metrics_cairo(dr2d_font* pFont, unsigned int utf32, dr2d_glyph_metrics* pGlyphMetrics)
{
    cairo_font_data* pCairoFont = dr2d_get_font_extra_data(pFont);
    if (pCairoFont == NULL) {
        return false;
    }

    // The UTF-32 code point needs to be converted to a UTF-8 character.
    char utf8[16];
    size_t utf8len = dr2d__utf32_to_utf8(utf32, utf8, sizeof(utf8)); // This will null-terminate.
    if (utf8len == 0) {
        return false;   // Error converting UTF-32 to UTF-8.
    }


    cairo_text_extents_t glyphExtents;
    cairo_scaled_font_text_extents(pCairoFont->pFont, utf8, &glyphExtents);

    if (pGlyphMetrics)
    {
        pGlyphMetrics->width    = glyphExtents.width;
        pGlyphMetrics->height   = glyphExtents.height;
        pGlyphMetrics->originX  = glyphExtents.x_bearing;
        pGlyphMetrics->originY  = glyphExtents.y_bearing;
        pGlyphMetrics->advanceX = glyphExtents.x_advance;
        pGlyphMetrics->advanceY = glyphExtents.y_advance;
    }

    return true;
}

bool dr2d_measure_string_cairo(dr2d_font* pFont, const char* text, size_t textSizeInBytes, float* pWidthOut, float* pHeightOut)
{
    cairo_font_data* pCairoFont = dr2d_get_font_extra_data(pFont);
    if (pCairoFont == NULL) {
        return false;
    }


    // Cairo expends null terminated strings, however the input string is not guaranteed to be null terminated.
    char* textNT;
    if (textSizeInBytes != (size_t)-1) {
        textNT = malloc(textSizeInBytes + 1);
        if (textNT == NULL) {
            return false;
        }
        memcpy(textNT, text, textSizeInBytes);
        textNT[textSizeInBytes] = '\0';
    } else {
        textNT = (char*)text;
    }


    cairo_text_extents_t textMetrics;
    cairo_scaled_font_text_extents(pCairoFont->pFont, textNT, &textMetrics);

    if (pWidthOut) {
        *pWidthOut = textMetrics.x_advance;
    }
    if (pHeightOut) {
        //*pHeightOut = textMetrics.height;
        *pHeightOut = pCairoFont->metrics.ascent + pCairoFont->metrics.descent;
    }


    if (textNT != text) {
        free(textNT);
    }

    return true;
}

bool dr2d_get_text_cursor_position_from_point_cairo(dr2d_font* pFont, const char* text, size_t textSizeInBytes, float maxWidth, float inputPosX, float* pTextCursorPosXOut, size_t* pCharacterIndexOut)
{
    cairo_font_data* pCairoFont = dr2d_get_font_extra_data(pFont);
    if (pCairoFont == NULL) {
        return false;
    }

    cairo_glyph_t* pGlyphs = NULL;
    int glyphCount = 0;
    cairo_status_t result = cairo_scaled_font_text_to_glyphs(pCairoFont->pFont, 0, 0, text, textSizeInBytes, &pGlyphs, &glyphCount, NULL, NULL, NULL);
    if (result != CAIRO_STATUS_SUCCESS) {
        return false;
    }

    float cursorPosX = 0;
    int charIndex = 0;

    // We just iterate over each glyph until we find the one sitting under <inputPosX>.
    float runningPosX = 0;
    for (int iGlyph = 0; iGlyph < glyphCount; ++iGlyph)
    {
        cairo_text_extents_t glyphMetrics;
        cairo_scaled_font_glyph_extents(pCairoFont->pFont, pGlyphs + iGlyph, 1, &glyphMetrics);

        float glyphLeft  = runningPosX;
        float glyphRight = glyphLeft + glyphMetrics.x_advance;

        // Are we sitting on top of inputPosX?
        if (inputPosX >= glyphLeft && inputPosX <= glyphRight)
        {
            float glyphHalf = glyphLeft + ceilf(((glyphRight - glyphLeft) / 2.0f));
            if (inputPosX <= glyphHalf) {
                cursorPosX = glyphLeft;
                charIndex  = iGlyph;
            } else {
                cursorPosX = glyphRight;
                charIndex  = iGlyph + 1;
            }

            break;
        }
        else
        {
            // Have we moved past maxWidth?
            if (glyphRight > maxWidth)
            {
                cursorPosX = maxWidth;
                charIndex  = iGlyph;
                break;
            }
            else
            {
                runningPosX = glyphRight;

                cursorPosX = runningPosX;
                charIndex  = iGlyph;
            }
        }
    }

    cairo_glyph_free(pGlyphs);

    if (pTextCursorPosXOut) {
        *pTextCursorPosXOut = cursorPosX;
    }
    if (pCharacterIndexOut) {
        *pCharacterIndexOut = charIndex;
    }

    return true;
}

bool dr2d_get_text_cursor_position_from_char_cairo(dr2d_font* pFont, const char* text, size_t characterIndex, float* pTextCursorPosXOut)
{
    cairo_font_data* pCairoFont = dr2d_get_font_extra_data(pFont);
    if (pCairoFont == NULL) {
        return false;
    }

    cairo_glyph_t* pGlyphs = NULL;
    int glyphCount = 0;
    cairo_status_t result = cairo_scaled_font_text_to_glyphs(pCairoFont->pFont, 0, 0, text, -1, &pGlyphs, &glyphCount, NULL, NULL, NULL);
    if (result != CAIRO_STATUS_SUCCESS) {
        return false;
    }

    float cursorPosX = 0;

    // We just iterate over each glyph until we find the one sitting under <inputPosX>.
    for (int iGlyph = 0; iGlyph < glyphCount; ++iGlyph)
    {
        if (iGlyph == (int)characterIndex) {
            break;
        }

        cairo_text_extents_t glyphMetrics;
        cairo_scaled_font_glyph_extents(pCairoFont->pFont, pGlyphs + iGlyph, 1, &glyphMetrics);

        cursorPosX += glyphMetrics.x_advance;
    }

    cairo_glyph_free(pGlyphs);

    if (pTextCursorPosXOut) {
        *pTextCursorPosXOut = cursorPosX;
    }

    return true;
}
#endif  // Cairo
#endif

/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/


#pragma once
#include "nanovg.h"

/**
 * Renders animated noise image sing nanogl
 */
// 0 = random pixel grey level
// 1 = random white, bk, transparent.
const int method = 0;
class NoiseDrawer
{
public:
    NoiseDrawer(NVGcontext *vg, float width, float height)
        : _width(width), _height(height)
    {
        makeImage(vg);
        _vg = vg;
    }

    ~NoiseDrawer()
    {
        nvgDeleteImage(_vg, _image);
        _image = 0;
    }

    void draw(NVGcontext *vg, float colorX, float colorY, float colorWidth, float colorHeight);

private:
    /**
     * Holds an image (gl texture) in memory for
     * the lifetime of the plugin instance.
     */
    int _image = 0;
    NVGcontext* _vg = nullptr;
    const int _width, _height;

    int frameCount = 0;
    float randomX = 0;
    float randomY = 0;

    void makeImage(NVGcontext *vg);
};


inline void NoiseDrawer::makeImage(NVGcontext *vg)
{
    // let's synthesize some white noise
    const int memSize = _width * _height * 4;
    unsigned char * memImage = new unsigned char[memSize];

    for (int row = 0; row < _height; ++row) {
        for (int col = 0; col < _width; ++col) {
            unsigned char * pix = memImage + (4 * (row*_width + col));
            if (method == 0) {
                int value = int((255.f * rand()) / float(RAND_MAX));
                pix[0] = value;
                pix[1] = value;
                pix[2] = value;
                pix[3] = 255;  // opaque, for now
            } else if (method == 1) {
                //generate 0 .. 100
                int rnd = int((100.f * rand()) / float(RAND_MAX));
                int value, alpha;
                if (rnd > 75) {
                    value = 0;
                    alpha = 255;
                } else if (rnd > 75) {
                    value = 255;
                    alpha = 255;
                } else {
                    value = 128;
                    alpha = 0;
                }
                pix[0] = value;
                pix[1] = value;
                pix[2] = value;
                pix[3] = alpha;
            }
        }
    }

    _image = nvgCreateImageRGBA(vg,
        _width,
        _height,
        NVG_IMAGE_REPEATX | NVG_IMAGE_REPEATY,
        memImage);

    assert(_image != 0);
    delete[] memImage;
}


inline void NoiseDrawer::draw(NVGcontext *vg,
    float drawX,
    float drawY,
    float drawWidth,
    float drawHeight)
{
    assert(_image);

    // noise looks slightly better with anti-alias off?
    nvgShapeAntiAlias(vg, false);

    // Don't update the noise position every frame. Old TV is only 
    // 30 fps, and this looks better even slower.
    if (frameCount++ > 3) {
        randomX = (float) rand() * _width / float(RAND_MAX);
        randomY = (float) rand() * _height / float(RAND_MAX);
        frameCount = 0;
    }

    // Clever trick. We don't draw new noise every time, we just
    // draw the same noise image from a random offset.
    NVGpaint paint = nvgImagePattern(vg,
        randomX, randomY,
        _width, _height,
        0, _image, 0.15f);

    nvgBeginPath(vg);
    nvgRect(vg, drawX, drawY, drawWidth, drawHeight);

    nvgFillPaint(vg, paint);
    nvgFill(vg);
}
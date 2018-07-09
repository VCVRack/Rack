#include "rack.hpp"

#include "BufferedDrawFunction.hpp"

template <int H>
struct GraduatedFader : SVGFader
{
  int slider_height = 41;
  int slider_width  = 20;
  int widget_width  = 28;
  FramebufferWidget *notches;

  GraduatedFader()
  {
    background->svg = NULL;
    background->wrap(); 
    background->box.pos = Vec( 0, 0 );
    
    handle->svg = SVG::load( assetPlugin( plugin, "res/BaconSliderHandle.svg" ) );
    handle->wrap();

    maxHandlePos = Vec( (widget_width-slider_width)/2, 0 );
    minHandlePos = Vec( (widget_width-slider_width)/2, (H-slider_height) );
    box.size = Vec( widget_width, H );

    notches = new BufferedDrawFunctionWidget<GraduatedFader<H>>( Vec( 0, 0 ), box.size, this,
                                                                 &GraduatedFader<H>::drawBackground );
  }
  
  void draw( NVGcontext *vg ) override
  {
    notches->draw( vg );
    SVGFader::draw( vg );
  }

  void drawBackground( NVGcontext *vg )
  {
    int nStrokes = 10;
    int slideTop = slider_height / 2;
    int slideHeight = H - slider_height;
    int slideBump = 5;
    int slotWidth = 1;
    
    
#ifdef DEBUG_NOTCHES
    nvgBeginPath( vg );
    nvgRect( vg, 0, 0, widget_width, H );
    nvgFillColor( vg, COLOR_RED );
    nvgFill( vg );
#endif
    
    float dx = (1.0 * slideHeight) / nStrokes;
    
    // Firest the gray highlights
    for( int i=0; i<= nStrokes; ++i )
      {
        nvgBeginPath( vg );
        nvgRect( vg, 1, slideTop + dx * i, widget_width-2, 1 );
        nvgFillColor( vg, nvgRGBA( 200, 200, 200, 255 ) );
        nvgFill( vg );
      }
    
    // and now the black notches
    for( int i=0; i<= nStrokes; ++i )
      {
        nvgBeginPath( vg );
        nvgRect( vg, 1, slideTop + dx * i-1, widget_width-2, 1.5 );
        nvgFillColor( vg, nvgRGBA( 100, 100, 100, 255 ));
        nvgFill( vg );
      }
    
    // OK so now we want to draw the vertical line
    nvgBeginPath( vg );
    nvgRect( vg, widget_width/2 - slotWidth, slideTop - slideBump, 2 * slotWidth + 1, slideHeight + 2 * slideBump );
    nvgFillColor( vg, COLOR_BLACK );
    nvgFill( vg );
  }

  void onZoom( EventZoom &e ) override
  {
    // Need this because I don't add it as a child, since the base class does something funky with that
    notches->onZoom( e );
    SVGFader::onZoom( e );
  }

};


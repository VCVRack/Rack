#ifndef INCLUDE_COMPONENTS_HPP
#define INCLUDE_COMPONENTS_HPP

#include "rack.hpp"

#include <map>
#include <vector>
#include <string>
#include <tuple>
#include <functional>
#include <locale>
#include <thread>

#include "GraduatedFader.hpp"
#include "BufferedDrawFunction.hpp"

using namespace rack;


template <typename T, int px = 4>
struct SevenSegmentLight : T {
  int lx, ly, ppl;
  std::vector< Rect > unscaledLoc;
  int elementsByNum[ 10 ][ 7 ] = {
    { 1, 1, 1, 1, 1, 1, 0 },
    { 0, 1, 1, 0, 0, 0, 0 },
    { 1, 1, 0, 1, 1, 0, 1 },
    { 1, 1, 1, 1, 0, 0, 1 },
    { 0, 1, 1, 0, 0, 1, 1 },
    { 1, 0, 1, 1, 0, 1, 1 },
    { 1, 0, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 0, 0, 0, 0 },
    { 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 0, 1, 1 }
  };

  const static int sx = px * 6 + 2;
  const static int sy = px * 10 + 2; // match with lx-1 and ly-1 below
  int pvalue;

  int decimalPos;
  
  BufferedDrawFunctionWidget<SevenSegmentLight<T, px>> *buffer;
  
  SevenSegmentLight( )
  {
    lx = 7;
    ly = 11;
    ppl = px;
    pvalue = 0;
    this->box.size = Vec( sx, sy );
    decimalPos = 1;

    // https://en.wikipedia.org/wiki/Seven-segment_display#/media/File:7_segment_display_labeled.svg
    unscaledLoc.push_back( Rect( Vec( 2, 1 ), Vec( 3, 1 ) ) );
    unscaledLoc.push_back( Rect( Vec( 5, 2 ), Vec( 1, 3 ) ) );
    unscaledLoc.push_back( Rect( Vec( 5, 6 ), Vec( 1, 3 ) ) );
    unscaledLoc.push_back( Rect( Vec( 2, 9 ), Vec( 3, 1 ) ) );
    unscaledLoc.push_back( Rect( Vec( 1, 6 ), Vec( 1, 3 ) ) );
    unscaledLoc.push_back( Rect( Vec( 1, 2 ), Vec( 1, 3 ) ) );
    unscaledLoc.push_back( Rect( Vec( 2, 5 ), Vec( 3, 1 ) ) );

    buffer = new BufferedDrawFunctionWidget<SevenSegmentLight<T,px>>( Vec( 0, 0 ), this->box.size,
                                                                      this,
                                                                      &SevenSegmentLight<T,px>::drawSegments );
    this->addChild( buffer );
  }

  void draw( NVGcontext *vg ) override
  {
    float fvalue = this->module->lights[ this->firstLightId ].value;
    int value = int( fvalue / decimalPos ) % 10;

    if( value != pvalue )
      {
        buffer->dirty = true;
      }

    pvalue = value;
    
    buffer->draw( vg );
  }

  void drawSegments( NVGcontext *vg )
  {
    // This is now buffered to only be called when the value has changed
    int w = this->box.size.x;
    int h = this->box.size.y;
    
    nvgBeginPath( vg );
    nvgRect( vg, 0, 0, w, h );
    nvgFillColor( vg, nvgRGBA( 25, 35, 25, 255 ) );
    nvgFill( vg );
    
    
    int i=0;
    // float fvalue = this->module->lights[ this->firstLightId ].value;
    // int value = clamp( fvalue, 0.0f, 9.0f );

    int *ebn = elementsByNum[ pvalue ];

    NVGcolor oncol = this->baseColors[ 0 ];
    
    for( auto it = unscaledLoc.begin(); it < unscaledLoc.end(); ++it )
      {
        float y = it->pos.y - 0.5;
        float x = it->pos.x - 0.5;
        int ew = it->size.x;
        int eh = it->size.y;
        nvgBeginPath( vg );
        // New version with corners
        float x0 = x * ppl + 1;
        float y0 = y * ppl + 1;
        float w = ew * ppl;
        float h = eh * ppl;
        float tri = ppl / 2;
        
        if( eh == 1 )
          {
            // This is a sideways element
            nvgMoveTo( vg, x0, y0 );
            nvgLineTo( vg, x0 + w, y0 );
            nvgLineTo( vg, x0 + w + tri, y0 + tri );
            nvgLineTo( vg, x0 + w, y0 + h);
            nvgLineTo( vg, x0, y0 + h);
            nvgLineTo( vg, x0 - tri, y0 + tri );
            nvgClosePath( vg );
          }
        else
          {
            nvgMoveTo( vg, x0, y0 );
            nvgLineTo( vg, x0, y0 + h );
            nvgLineTo( vg, x0 + tri, y0 + h + tri );
            nvgLineTo( vg, x0 + w, y0 + h);
            nvgLineTo( vg, x0 + w, y0);
            nvgLineTo( vg, x0 + tri, y0 - tri );
          }


        // Old version nvgRect( vg, x * ppl + 1, y * ppl + 1, ew * ppl, eh * ppl );
        if( ebn[ i ] > 0 )
          {
            nvgFillColor( vg, oncol );
            nvgFill( vg );
          }
        else
          {
            nvgFillColor( vg, nvgRGBA( 50, 70, 50, 255 ) );
            nvgFill( vg );
                    
          }
        ++i;
      }
    
  }

  static SevenSegmentLight< T, px > *create(Vec pos, Module *module, int firstLightId, int decimal) {
    auto *o = ModuleLightWidget::create<SevenSegmentLight<T,px>>(pos, module, firstLightId);
    o->decimalPos = decimal;
    return o;
  }

};

template <typename colorClass, int px, int digits>
struct MultiDigitSevenSegmentLight : ModuleLightWidget
{
  typedef SevenSegmentLight< colorClass, px > LtClass;
  
  MultiDigitSevenSegmentLight() : ModuleLightWidget()
  {
    this->box.size = Vec( digits * LtClass::sx, LtClass::sy );
  }

  static MultiDigitSevenSegmentLight< colorClass, px, digits > *create(Vec pos, Module *module, int firstLightId) {
    auto *o = ModuleLightWidget::create<MultiDigitSevenSegmentLight<colorClass, px ,digits>>(pos, module, firstLightId);
    o->layout();
    return o;
  }

  void layout()
  {
    int dp = 1;
    for( int i=0; i<digits-1; ++i ) dp *= 10;
    
    for( int i=0; i<digits; ++i )
      {
        addChild(  LtClass::create( Vec( i * LtClass::sx, 0 ), module, firstLightId, dp ) );
        dp /= 10;
      }
  }

  void draw( NVGcontext *vg ) override
  {
    for( auto it = children.begin(); it != children.end(); ++it )
      {
        nvgSave( vg );
        nvgTranslate( vg, (*it)->box.pos.x, (*it)->box.pos.y );
        (*it)->draw( vg );
        nvgRestore( vg );
      }
  }

};

struct BaconBackground : virtual TransparentWidget
{
  static NVGcolor bg;
  static NVGcolor bgOutline;
  static NVGcolor highlight;

  typedef std::tuple< Rect, NVGcolor, bool > col_rect_t;
  std::vector< col_rect_t > rects;

  int memFont = -1;
  std::string title;

  enum LabelAt {
    ABOVE,
    BELOW,
    LEFT,
    RIGHT
  };

  enum LabelStyle {
    SIG_IN,
    SIG_OUT,
    OTHER
  };

  int cx() { return box.size.x / 2; }
  int cx( int w ) { return (box.size.x-w) / 2; }
  
  BaconBackground( Vec size, const char* lab );
  ~BaconBackground() { }

  BaconBackground *addLabel( Vec pos, const char* lab, int px )
  {
    return addLabel( pos, lab, px, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM, COLOR_BLACK );
  }
  BaconBackground *addLabel( Vec pos, const char* lab, int px, int align )
  {
    return addLabel( pos, lab, px, align, COLOR_BLACK );
  }
  BaconBackground *addLabel( Vec pos, const char* lab, int px, int align, NVGcolor col );

  BaconBackground *addPlugLabel( Vec plugPos, LabelStyle s, const char* ilabel ) {
    return addPlugLabel( plugPos, LabelAt::ABOVE, s, ilabel );
  }
  BaconBackground *addPlugLabel( Vec plugPos, LabelAt l, LabelStyle s, const char* ilabel );
  BaconBackground *addRoundedBorder( Vec pos, Vec sz );
  BaconBackground *addRoundedBorder( Vec pos, Vec sz, NVGcolor fill );

  BaconBackground *addLabelsForHugeKnob( Vec topLabelPos, const char* knobLabel, const char* zeroLabel, const char* oneLabel, Vec &putKnobHere );
  BaconBackground *addLabelsForLargeKnob( Vec topLabelPos, const char* knobLabel, const char* zeroLabel, const char* oneLabel, Vec &putKnobHere );

  BaconBackground *addFilledRect( Vec pos, Vec sz, NVGcolor fill )
  {
    Rect r;
    r.pos = pos; r.size = sz;
    rects.push_back( col_rect_t( r, fill, true ) );
    return this;
  }

  BaconBackground *addRect( Vec pos, Vec sz, NVGcolor fill )
  {
    Rect r;
    r.pos = pos; r.size = sz;
    rects.push_back( col_rect_t( r, fill, false ) );
    return this;
  }

  void draw( NVGcontext *vg ) override;

  FramebufferWidget *wrappedInFramebuffer();
};

struct BaconHelpButton : public SVGButton
{
  std::string url;
  BaconHelpButton( std::string urli ) : url( urli )
  {
    box.pos = Vec( 0, 0 );
    box.size = Vec( 20, 20 );
    setSVGs( SVG::load( assetPlugin( plugin, "res/HelpActiveSmall.svg" ) ), NULL );
    url = "https://github.com/baconpaul/BaconPlugs/blob/";
#ifdef RELEASE_BRANCH
    url += TO_STRING( RELEASE_BRANCH );
#else
    url += "master/";
#endif
    url += urli;
    info( "Help button configured to: %s", url.c_str() );
  }

  void onAction( EventAction &e ) override {
    std::thread t( [this]() {
        systemOpenBrowser(url.c_str() );
      } );
    t.detach();
  }
};

template< int NSteps, typename ColorModel >
struct NStepDraggableLEDWidget : public ParamWidget
{
  BufferedDrawFunctionWidget<NStepDraggableLEDWidget<NSteps, ColorModel>> *buffer;
  bool dragging;
  Vec lastDragPos;
  ColorModel cm;
  
  NStepDraggableLEDWidget()
  {
    box.size = Vec( 10, 200 );
    dragging = false;
    lastDragPos = Vec( -1, -1 );
    
    buffer = new BufferedDrawFunctionWidget<NStepDraggableLEDWidget<NSteps, ColorModel>>( Vec( 0, 0 ), this->box.size,
                                                                                          this,
                                                                                          &NStepDraggableLEDWidget<NSteps, ColorModel>::drawSegments );

  }

  int getStep()
  {
    float pvalue = this->module->params[ this->paramId ].value;
    int step = (int)pvalue;
    return step;
  }

  int impStep( float yp )
  {
    float py = (box.size.y - yp)/box.size.y;
    return (int)( py * NSteps );
  }
  
  void draw( NVGcontext *vg ) override
  {
    buffer->draw( vg );
  }

  void valueByMouse( float ey )
  {
    if( impStep( ey ) != getStep() )
    {
      buffer->dirty = true;
      setValue( impStep( ey ) );
    }
  }
  
  void onMouseDown( EventMouseDown &e ) override
  {
    ParamWidget::onMouseDown( e );
    valueByMouse( e.pos.y );
    dragging = true;
  }

  void onMouseUp( EventMouseUp &e ) override
  {
    ParamWidget::onMouseUp( e );
    valueByMouse( e.pos.y );
    dragging = false;
    lastDragPos = Vec( -1, -1 );
  }

  void onMouseMove( EventMouseMove &e ) override
  {
    ParamWidget::onMouseMove( e );
    if( dragging && ( e.pos.x != lastDragPos.x || e.pos.y != lastDragPos.y ))
      {
        valueByMouse( e.pos.y );
        lastDragPos = e.pos;
      }
  }

  void onMouseLeave( EventMouseLeave &e ) override
  {
    ParamWidget::onMouseLeave( e );
    dragging = false;
    lastDragPos = Vec( -1, -1 );
  }

  void drawSegments( NVGcontext *vg )
  {
    // This is now buffered to only be called when the value has changed
    int w = this->box.size.x;
    int h = this->box.size.y;
    
    nvgBeginPath( vg );
    nvgRect( vg, 0, 0, w, h );
    nvgFillColor( vg, nvgRGB( 40, 40, 40 ) );
    nvgFill( vg );

    float dy = box.size.y / NSteps;
    for( int i=0; i<NSteps; ++i )
      {
        nvgBeginPath( vg );
        nvgRect( vg, 1, i * dy + 1, w - 2, dy - 2 );
        nvgFillColor( vg, cm.elementColor( NSteps - 1 - i , NSteps, getStep() ) );
        nvgFill( vg );
      }
  }  

  
};

struct GreenFromZeroColorModel
{
  NVGcolor GREEN, BLACK;
  GreenFromZeroColorModel() : GREEN( nvgRGB( 10, 255, 10 ) ), BLACK( nvgRGB( 10, 10, 10 ) ) { }
  NVGcolor elementColor( int stepNo, int NSteps, int value )
  {
    if( stepNo <= value )
      return nvgRGB( 10, 155 + 1.0f * stepNo / NSteps * 100, 10 );
    else
      return BLACK;
  }
};


struct RedGreenFromMiddleColorModel
{
  NVGcolor GREEN, BLACK, RED;
  RedGreenFromMiddleColorModel() : GREEN( nvgRGB( 10, 255, 10 ) ), BLACK( nvgRGB( 10, 10, 10 ) ), RED( nvgRGB( 255, 10, 10 ) ) { }
  NVGcolor elementColor( int stepNo, int NSteps, int value )
  {
    // This has the 'midpoint' be 0 so we want to compare with NSteps/2
    if( value < NSteps / 2 ) {
      // We are in the bottom half.
      if( stepNo < value || stepNo >= NSteps / 2) return BLACK;
      else
        {
          int distance = NSteps / 2 - stepNo;
          return nvgRGB( 155 + 1.0f * distance / ( NSteps / 2 ) * 100 , 10, 10 );
        }
    } else {
      if( stepNo > value || stepNo < NSteps / 2) return BLACK;
      else
        {
          int distance = stepNo - NSteps / 2;
          return nvgRGB( 10, 155 + 1.0f * distance / ( NSteps / 2 ) * 100 , 10 );
        }
    }
  }
};

#include <iostream>
// Think hard about dirty state management ... later
// Pixel Sizing
// Share fontdata
struct DotMatrixLightTextWidget : public Component // Thanks http://scruss.com/blog/tag/font/
{
 
  typedef std::function< std::string( Module * )> stringGetter;
  typedef std::function< bool( Module * )> stringDirtyGetter;

  BufferedDrawFunctionWidget<DotMatrixLightTextWidget> *buffer;

  int charCount;
  std::string currentText;

  typedef std::map< char, std::vector< bool > > fontData_t;
  fontData_t fontData;

  float ledSize, padSize;
  

  DotMatrixLightTextWidget() : Component(), buffer( NULL ), currentText( "" )
  {
  }

  void setup()
  {
    ledSize = 2;
    padSize = 1;
    box.size = Vec( charCount * ( 5 * ledSize + padSize ) + 2 * padSize, 7 * ledSize + 4.5 * padSize );  // 5 x 7 data structure
    buffer = new BufferedDrawFunctionWidget< DotMatrixLightTextWidget >( Vec( 0, 0 ), this->box.size, this,
                                                                         &DotMatrixLightTextWidget::drawText );

    info( "BaconMusic loading DMP json: %s", assetPlugin( plugin, "res/Keypunch029.json" ).c_str() );
    
    json_t *json;
    json_error_t error;
    
    json = json_load_file(assetPlugin( plugin, "res/Keypunch029.json" ).c_str(), 0, &error);
    if(!json) {
      info( "JSON FILE not loaded\n" );
    }
    const char* key;
    json_t *value;
    json_object_foreach( json, key, value ) {
      fontData_t::mapped_type valmap;
      size_t index;
      json_t *aval;
      json_array_foreach( value, index, aval ) {
        std::string s( json_string_value( aval ) );
        for( const char* c = s.c_str(); *c != 0; ++c ) {
          valmap.push_back( *c == '#' );
        }
      }
      fontData[ key[ 0 ] ] = valmap;
    }
  }
  
  // create takes a function
  static DotMatrixLightTextWidget *create( Vec pos, Module *module, int charCount, stringDirtyGetter dgf, stringGetter gf )
  {
    DotMatrixLightTextWidget *r = Component::create<DotMatrixLightTextWidget>( pos, module );
    r->getfn = gf;
    r->dirtyfn = dgf;
    r->charCount = charCount;
    r->setup();
    return r;
  }

  stringDirtyGetter dirtyfn;
  stringGetter getfn;
  
  void draw( NVGcontext *vg ) override
  {
    if( dirtyfn( this->module ) )
      {
        currentText = getfn( this->module );
        buffer->dirty = true;
      }
    if( buffer )
      buffer->draw( vg );
  }

  void drawChar( NVGcontext *vg, Vec pos, char c )
  {
#define UPPERCASE(a) ((char)( ((a)>='a'&&(a)<='z') ? ((a)&~32) : (a) ))
     // fontData_t::iterator k = fontData.find( (cstd::toupper( c ) );
     fontData_t::iterator k = fontData.find( UPPERCASE(c) );

    if( k != fontData.end() ) {
      fontData_t::mapped_type blist = k->second;
      int row=0, col=0;
      for( auto v = blist.begin(); v != blist.end(); ++v )
        {
          if( *v )
            {
              float xo = (col+0.5) * ledSize + pos.x;
              float yo = (row+0.5) * ledSize + pos.y;
              nvgBeginPath( vg );
              // nvgRect( vg, xo, yo, ledSize, ledSize );
              nvgCircle( vg, xo + ledSize/2.0f, yo + ledSize/2.0f, ledSize/2.0f * 1.1 );
              nvgFillColor( vg, nvgRGBA( 25, 35, 25, 255 ) );
              nvgFill( vg );
              
              nvgBeginPath( vg );
              // nvgRect( vg, xo, yo, ledSize, ledSize );
              nvgCircle( vg, xo + ledSize/2.0f, yo + ledSize/2.0f, ledSize/2.0f );
              nvgFillColor( vg, COLOR_BLUE ); // Thanks for having such a nice blue, Rack!!
              nvgFill( vg );
            }
          
          col++;
          if( col == 5 ) {
            col = 0;
            row ++;
          }
        }
    }
    else {
    }
  }
  
  void drawText( NVGcontext *vg ) 
  {
    nvgBeginPath( vg );
    nvgRect( vg, 0, 0, box.size.x, box.size.y );
    nvgFillColor( vg, nvgRGBA( 15, 15, 55, 255 ) );
    nvgFill( vg );

    Vec cpos = Vec( padSize, padSize );
    for( const char* c = currentText.c_str(); *c != 0; ++c ) {
      drawChar( vg, cpos, *c );
      cpos.x += ledSize * 5 + padSize;
    }
  }

  void onZoom( EventZoom &e ) override
  {
    buffer->dirty = true;
  }
};

#include "SizeTable.hpp"


#endif

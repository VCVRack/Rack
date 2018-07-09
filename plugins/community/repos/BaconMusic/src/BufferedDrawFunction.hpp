#ifndef BUFFERED_DRAW_FUNCTION_INCLUDE
#define BUFFERED_DRAW_FUNCTION_INCLUDE

#include "rack.hpp"

#include <functional>
#include <vector>

using namespace rack;

template < typename T >
struct BufferedDrawFunctionWidget : virtual FramebufferWidget
{
  typedef std::function< void( T *, NVGcontext *) > drawfn_t;
  T *that;
  drawfn_t drawf;


  struct InternalBDW : TransparentWidget
  {
    T* that;
    drawfn_t drawf;
    InternalBDW( Rect box_, T* that_, drawfn_t draw_ ) : that( that_ ), drawf( draw_ )
    {
      box = box_;
    }
    void draw( NVGcontext *vg ) override
    {
      drawf( that, vg );
    }
  };
  
  BufferedDrawFunctionWidget( Vec pos, Vec sz, T* that_, drawfn_t draw_ ) : that( that_ ), drawf( draw_ )
  {
    box.pos = pos; box.size = sz;
    auto kidBox = Rect( Vec( 0, 0 ), box.size );
    InternalBDW *kid = new InternalBDW( kidBox, that, drawf );
    addChild( kid );
  }
};

struct BufferedDrawLambdaWidget : virtual FramebufferWidget
{
  typedef std::function< void( NVGcontext *) > drawfn_t;
  drawfn_t drawf;


  struct InternalBDW : TransparentWidget
  {
    drawfn_t drawf;
    InternalBDW( Rect box_, drawfn_t draw_ ) :  drawf( draw_ )
    {
      box = box_;
    }
    void draw( NVGcontext *vg ) override
    {
      drawf( vg );
    }
  };
  
  BufferedDrawLambdaWidget( Vec pos, Vec sz, drawfn_t draw_ ) :  drawf( draw_ )
  {
    box.pos = pos; box.size = sz;
    auto kidBox = Rect( Vec( 0, 0 ), box.size );
    InternalBDW *kid = new InternalBDW( kidBox, drawf );
    addChild( kid );
  }
};

#endif

#include "BaconPlugs.hpp"
#include <jansson.h>


struct InternalFontMgr
{
  static std::map< std::string, int > fontMap;
  static int get( NVGcontext *vg, std::string resName )
  {
    if( fontMap.find( resName ) == fontMap.end() )
      {
         fontMap[ resName ] = nvgCreateFont( vg, resName.c_str(), assetPlugin( plugin, resName.c_str() ).c_str() );
      }
    return fontMap[ resName ];
  }
};

std::map< std::string, int > InternalFontMgr::fontMap;

struct InternalRoundedBorder : virtual TransparentWidget
{
  bool doFill;
  NVGcolor fillColor;

  InternalRoundedBorder( Vec pos, Vec sz, NVGcolor fc )
  {
    box.pos = pos;
    box.size = sz;
    doFill = true;
    fillColor = fc;
  }

  InternalRoundedBorder( Vec pos, Vec sz )
  {
    box.pos = pos;
    box.size = sz;
    doFill = false;
  }

  void draw( NVGcontext *vg ) override
  {
    nvgBeginPath( vg );
    nvgRoundedRect( vg, 0, 0, box.size.x, box.size.y, 5 );
    if( doFill )
      {
        nvgFillColor( vg, fillColor );
        nvgFill( vg );
      }
        
    nvgStrokeColor( vg, COLOR_BLACK );
    nvgStroke( vg );
  }
};

struct InternalTextLabel : virtual TransparentWidget
{
  int memFont = -1;
  std::string label;
  int pxSize;
  int align;
  NVGcolor color;

  InternalTextLabel( Vec pos, const char* lab, int px, int al, NVGcolor col ) : label( lab ), pxSize( px ), align( al ), color( col )
  {
    box.pos = pos;
  }

  void draw( NVGcontext *vg ) override {
    if( memFont < 0 )
      memFont = InternalFontMgr::get( vg, "res/Monitorica-Bd.ttf" );

    nvgBeginPath( vg );
    nvgFontFaceId( vg, memFont );
    nvgFontSize( vg, pxSize );
    nvgFillColor( vg, color );
    nvgTextAlign( vg, align );
    nvgText( vg, 0, 0, label.c_str(), NULL );
  }
};

struct InternalPlugLabel : virtual TransparentWidget
{
  int memFont = -1;

  BaconBackground::LabelStyle  st;
  BaconBackground::LabelAt     at;
  std::string label;

  InternalPlugLabel( Vec portPos, BaconBackground::LabelAt l, BaconBackground::LabelStyle s, const char* ilabel );
  
  void draw( NVGcontext *vg ) override;

};


void BaconBackground::draw( NVGcontext *vg )
{
  if( memFont < 0 )
    memFont = InternalFontMgr::get( vg, "res/Monitorica-Bd.ttf" );

  nvgBeginPath( vg );
  nvgRect( vg, 0, 0, box.size.x, box.size.y );
  nvgFillColor( vg, BaconBackground::bg );
  nvgFill( vg );

  nvgBeginPath( vg );
  nvgMoveTo( vg, 0, 0 );
  nvgLineTo( vg, box.size.x, 0 );
  nvgLineTo( vg, box.size.x, box.size.y );
  nvgLineTo( vg, 0, box.size.y );
  nvgLineTo( vg, 0, 0 );
  nvgStrokeColor( vg, BaconBackground::bgOutline );
  nvgStroke( vg );

  nvgFontFaceId( vg, memFont );
  nvgFontSize( vg, 14 );
  nvgFillColor( vg, COLOR_BLACK );
  nvgStrokeColor( vg, COLOR_BLACK );
  nvgTextAlign( vg, NVG_ALIGN_CENTER|NVG_ALIGN_BOTTOM );
  nvgText( vg, box.size.x / 2, box.size.y - 5, "Bacon Music", NULL );

  nvgFontFaceId( vg, memFont );
  nvgFontSize( vg, 16 );
  nvgFillColor( vg, COLOR_BLACK );
  nvgStrokeColor( vg, COLOR_BLACK );
  nvgTextAlign( vg, NVG_ALIGN_CENTER|NVG_ALIGN_TOP );
  nvgText( vg, box.size.x / 2, 5, title.c_str(), NULL );

  for( auto w : children )
    {
      nvgTranslate( vg, w->box.pos.x, w->box.pos.y );
      w->draw( vg );
      nvgTranslate( vg, -w->box.pos.x, -w->box.pos.y );
    }

  for( auto it = rects.begin(); it != rects.end(); ++it )
    {
      col_rect_t tu = *it;
      Rect r     = std::get< 0 >(tu);
      NVGcolor c = std::get< 1 >(tu);
      bool f     = std::get< 2 >(tu);
      nvgBeginPath( vg );
      nvgRect( vg, r.pos.x, r.pos.y, r.size.x, r.size.y );
      if( f )
        {
          nvgFillColor( vg, c );
          nvgFill( vg );
        }
      else
        {
          nvgStrokeColor( vg, c );
          nvgStroke( vg );
        }
    }

}

InternalPlugLabel::InternalPlugLabel( Vec portPos, BaconBackground::LabelAt l, BaconBackground::LabelStyle s, const char* ilabel )
  :
  st( s ), at( l ), label( ilabel )
{
  box.size.x = 24 + 5;
  box.size.y = 24 + 5 + 20;
  
  // switch on position but for now just do above
  box.pos.x = portPos.x - 2.5;
  box.pos.y = portPos.y - 2.5 - 17;
}


void InternalPlugLabel::draw( NVGcontext *vg )
{
  if( memFont < 0 )
    memFont = InternalFontMgr::get( vg, "res/Monitorica-Bd.ttf" );

  NVGcolor txtCol = COLOR_BLACK;
  
  switch( st ) {
  case( BaconBackground::SIG_IN ) :
    {
      nvgBeginPath( vg );
      nvgRoundedRect( vg, 0, 0, box.size.x, box.size.y, 5 );
      nvgStrokeColor( vg, COLOR_BLACK );
      nvgStroke( vg );
      break;
    }
  case( BaconBackground::SIG_OUT ) :
    {
      nvgBeginPath( vg );
      nvgRoundedRect( vg, 0, 0, box.size.x, box.size.y, 5 );
      nvgFillColor( vg, BaconBackground::highlight );
      nvgFill( vg );

      nvgStrokeColor( vg, COLOR_BLACK );
      nvgStroke( vg );

      txtCol = COLOR_WHITE;
      break;
    }
  case( BaconBackground::OTHER ) :
    {
      nvgBeginPath( vg );
      nvgRoundedRect( vg, 0, 0, box.size.x, box.size.y, 5 ); 
      nvgStrokeColor( vg, COLOR_RED );
      nvgStroke( vg );
      break;
    }
  }
      
  nvgFontFaceId( vg, memFont );
  nvgFontSize( vg, 13 );
  nvgFillColor( vg, txtCol );
  nvgTextAlign( vg, NVG_ALIGN_CENTER|NVG_ALIGN_TOP );
  nvgText( vg, box.size.x / 2, 3, label.c_str(), NULL );

}



BaconBackground *BaconBackground::addLabel( Vec pos, const char* lab, int px, int align, NVGcolor col )
{
  addChild( new InternalTextLabel( pos, lab, px, align, col ) );
  return this;
}

BaconBackground *BaconBackground::addPlugLabel( Vec plugPos, LabelAt l, LabelStyle s, const char* ilabel )
{
  addChild( new InternalPlugLabel( plugPos, l, s, ilabel ) );
  return this;
}

BaconBackground *BaconBackground::addRoundedBorder( Vec pos, Vec sz )
{
  addChild( new InternalRoundedBorder( pos, sz ) );
  return this;
}

BaconBackground *BaconBackground::addRoundedBorder( Vec pos, Vec sz, NVGcolor fill )
{
  addChild( new InternalRoundedBorder( pos, sz, fill ) );
  return this;
}


NVGcolor BaconBackground::bg = nvgRGBA( 220, 220, 210, 255 );
NVGcolor BaconBackground::bgOutline = nvgRGBA( 180, 180, 170, 255 );
NVGcolor BaconBackground::highlight = nvgRGBA( 90, 90, 60, 255 );

BaconBackground::BaconBackground( Vec size, const char* lab ) : title( lab )
{
  box.pos = Vec( 0, 0 );
  box.size = size;
}

FramebufferWidget* BaconBackground::wrappedInFramebuffer()
{
  FramebufferWidget *fb = new FramebufferWidget();
  fb->box = box;
  fb->addChild( this );
  return fb;
}



BaconBackground *BaconBackground::addLabelsForHugeKnob( Vec topLabelPos,
                                                        const char* knobLabel,
                                                        const char* zeroLabel,
                                                        const char* oneLabel,
                                                        Vec &putKnobHere )
{
  addLabel( topLabelPos, knobLabel, 14, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE );
  addLabel( Vec( topLabelPos.x + 10, topLabelPos.y + 72 ),
            oneLabel, 13, NVG_ALIGN_LEFT | NVG_ALIGN_TOP );
  addLabel( Vec( topLabelPos.x - 10, topLabelPos.y + 72 ),
            zeroLabel, 13, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP );
  putKnobHere.y = topLabelPos.y + 10;
  putKnobHere.x = topLabelPos.x - 28;
  return this;
}

BaconBackground *BaconBackground::addLabelsForLargeKnob( Vec topLabelPos,
                                                         const char* knobLabel,
                                                         const char* zeroLabel,
                                                         const char* oneLabel,
                                                         Vec &putKnobHere )
{
  addLabel( topLabelPos, knobLabel, 14, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE );
  addLabel( Vec( topLabelPos.x + 10, topLabelPos.y + 48 ),
            oneLabel, 13, NVG_ALIGN_LEFT | NVG_ALIGN_TOP );
  addLabel( Vec( topLabelPos.x - 10, topLabelPos.y + 48 ),
            zeroLabel, 13, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP );
  putKnobHere.y = topLabelPos.y + 10;
  putKnobHere.x = topLabelPos.x - 18;
  return this;
}

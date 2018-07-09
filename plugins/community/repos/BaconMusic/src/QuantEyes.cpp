#include "BaconPlugs.hpp"
#include "dsp/digital.hpp"

#define SCALE_LENGTH 12

namespace rack_plugin_BaconMusic {

struct QuantEyes : virtual Module {
  enum ParamIds {
    ROOT_STEP,
    SCALE_PARAM,
    NUM_PARAMS = SCALE_PARAM + SCALE_LENGTH
  };

  enum InputIds {
    CV_INPUT,
    CV_INPUT_2,
    CV_INPUT_3,
    NUM_INPUTS
  };

  enum OutputIds {
    QUANTIZED_OUT,
    QUANTIZED_OUT_2,
    QUANTIZED_OUT_3,
    NUM_OUTPUTS
  };

  enum LightIds {
    ROOT_LIGHT,
    ACTIVE_NOTE_LIGHTS,
    SCALE_LIGHTS = ACTIVE_NOTE_LIGHTS + 3 * SCALE_LENGTH,
    NUM_LIGHTS = SCALE_LIGHTS + SCALE_LENGTH
  };


  int scaleState[ SCALE_LENGTH ];
  SchmittTrigger scaleTriggers[ SCALE_LENGTH ];

  QuantEyes() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS )
  {
    for( int i=0; i<SCALE_LENGTH; ++i ) scaleState[ i ] = 1;
  }

  void step() override
  {
    int root = clamp( params[ ROOT_STEP ].value, 0.0f, 12.0f );
    lights[ ROOT_LIGHT ].value = root;

    for( int i=0; i<SCALE_LENGTH; ++i )
      {
        if( scaleTriggers[ i ].process( params[ SCALE_PARAM + i ].value )  )
          {
            scaleState[ i ] = ! scaleState[ i ];
          }
        lights[ SCALE_LIGHTS + i ].value = scaleState[ i ];
        lights[ ACTIVE_NOTE_LIGHTS + i ].value = 0;
        lights[ ACTIVE_NOTE_LIGHTS + i + SCALE_LENGTH ].value = 0;
        lights[ ACTIVE_NOTE_LIGHTS + i + SCALE_LENGTH * 2 ].value = 0;
      }
    

    for( int i=0; i<3; ++i )
      {
        if( inputs[ CV_INPUT + i ].active )
          {
            float in = inputs[ CV_INPUT + i ].value;
            double octave, note;
            note = modf( in, &octave );
            float noteF = ( floor( note * SCALE_LENGTH ) + root );
            int noteI = (int)noteF % SCALE_LENGTH;
            
            if( noteF > SCALE_LENGTH-1 ) octave += 1.0;
            
            
            while( scaleState[ noteI ] == 0 && noteI > 0 ) noteI --;
            
            lights[ ACTIVE_NOTE_LIGHTS + i * SCALE_LENGTH + noteI ].value = 1;
        
            float out = 1.0 * noteI / SCALE_LENGTH + octave;
            outputs[ QUANTIZED_OUT + i ].value = out;
          }
      }
  }

  json_t *toJson() override {
    json_t *rootJ = json_object();
    json_t *scaleJ = json_array();
    for( int i=0; i<SCALE_LENGTH; ++i )
      {
        json_t *noteJ = json_integer( scaleState[ i ] );
        json_array_append_new( scaleJ, noteJ );
      }
    json_object_set_new( rootJ, "scaleState", scaleJ );

    return rootJ;
  }

  void fromJson( json_t *rootJ ) override {
    json_t* scaleJ =  json_object_get( rootJ, "scaleState" );
    if( scaleJ )
      for( int i=0; i<SCALE_LENGTH; ++i )
        {
          json_t *noteJ = json_array_get( scaleJ, i );
          if( noteJ )
            scaleState[ i ] = json_integer_value( noteJ );
        }
      
  }
  
};

struct QuantEyesWidget : ModuleWidget {
  QuantEyesWidget( QuantEyes *model );
};

QuantEyesWidget::QuantEyesWidget( QuantEyes *model ) : ModuleWidget( model )
{
  box.size = Vec( SCREW_WIDTH * 11, RACK_HEIGHT );

  BaconBackground *bg = new BaconBackground( box.size, "QuantEyes" );
  addChild( bg->wrappedInFramebuffer());

 
  int rx = 15, ry = 30, sp = 22, slope = 8;
  for( int i=0; i<SCALE_LENGTH; ++i )
    {
      char d[ 24 ];
      sprintf( d, "%d", i+1 );
      if( i==0 ) d[ 0 ] = 'R';
      int x0 = rx + (i + 0.5) * slope;
      int yp0 = (SCALE_LENGTH - i - 1) * sp;
      bg->addLabel( Vec( rx - 3, yp0 + ry + sp / 2), d, 12, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE );
      addParam( ParamWidget::create< LEDButton >( Vec( x0, yp0 + ry ), module, QuantEyes::SCALE_PARAM + i, 0, 1, 0 ) );
      addChild( ModuleLightWidget::create< MediumLight< BlueLight > >( Vec( x0 + 4, yp0 + ry + 4 ), module, QuantEyes::SCALE_LIGHTS + i ) );
      addChild( ModuleLightWidget::create< SmallLight< GreenLight > >( Vec( x0 + 20, yp0 + ry + 6 ), module, QuantEyes::ACTIVE_NOTE_LIGHTS + i ) );
      addChild( ModuleLightWidget::create< SmallLight< GreenLight > >( Vec( x0 + 28, yp0 + ry + 6 ), module, QuantEyes::ACTIVE_NOTE_LIGHTS + i + 12 ) );
      addChild( ModuleLightWidget::create< SmallLight< GreenLight > >( Vec( x0 + 36, yp0 + ry + 6 ), module, QuantEyes::ACTIVE_NOTE_LIGHTS + i + 24 ) );

      auto c = nvgRGBA( 225, 225, 225, 255 );
      if( i == 1 || i == 3 || i == 6 || i == 8 || i == 10 )
        c = nvgRGBA( 110, 110, 110, 255 );
      
      bg->addFilledRect( Vec( rx  , yp0 + ry + 7), Vec( i * slope + 39 , 4.5 ), c );
      bg->addRect( Vec( rx , yp0 + ry + 7), Vec( i * slope + 39 , 4.5 ), nvgRGBA( 70, 70, 70, 255 ) );
    }

  int xpospl = box.size.x - 24 - 9;
  Vec inP = Vec( xpospl - 32, RACK_HEIGHT - 60 );
  Vec outP = Vec( xpospl, RACK_HEIGHT - 60 );
  
  for( int i=0; i<3; ++i )
    {
      Vec off( 0, -55 * (2-i) );
      char buf[ 20 ];
      sprintf( buf, "in %d", i+1 );
      bg->addPlugLabel( inP.plus(off), BaconBackground::SIG_IN, buf );
      addInput( Port::create< PJ301MPort >( inP.plus(off), Port::INPUT,
                                           module,
                                           QuantEyes::CV_INPUT + i ) );

      sprintf( buf, "out %d", i+1 );
      bg->addPlugLabel( outP.plus(off), BaconBackground::SIG_OUT, buf );

      
      addOutput( Port::create< PJ301MPort >( outP.plus(off), Port::OUTPUT,
                                             module,
                                             QuantEyes::QUANTIZED_OUT + i) );

    }

  
  bg->addRoundedBorder( Vec( 10, box.size.y - 78 ), Vec ( 70, 49 ) );
  bg->addLabel( Vec( 45, box.size.y - 74 ), "Root CV", 12, NVG_ALIGN_CENTER | NVG_ALIGN_TOP );
  int ybot = box.size.y - 78 + 24 + 5 + 20;
  addParam( ParamWidget::create< RoundSmallBlackKnob >( Vec( 16, ybot - 3 - 28 ),
                                                module,
                                                QuantEyes::ROOT_STEP,
                                                0, 12, 0 ) );
  addChild( MultiDigitSevenSegmentLight< BlueLight, 2, 2 >::create( Vec( 47, ybot - 5 - 24 ),
                                                                    module,
                                                                    QuantEyes::ROOT_LIGHT ) );

}

} // namespace rack_plugin_BaconMusic

using namespace rack_plugin_BaconMusic;

RACK_PLUGIN_MODEL_INIT(BaconMusic, QuantEyes) {
   Model *modelQuantEyes = Model::create< QuantEyes, QuantEyesWidget > ("Bacon Music", "QuantEyes", "QuantEyes", QUANTIZER_TAG);
   return modelQuantEyes;
}

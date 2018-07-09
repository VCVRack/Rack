
#include "BaconPlugs.hpp"
#include <sstream>
#include <vector>
#include <string>
#include "dsp/digital.hpp"

#include "KSSynth.hpp"

namespace rack_plugin_BaconMusic {

struct KarplusStrongPoly : virtual Module {
  enum ParamIds {
    INITIAL_PACKET,
    FILTER_TYPE,
    FREQ_KNOB,
    ATTEN_KNOB,
    FILTER_KNOB_A,
    FILTER_KNOB_B,
    FILTER_KNOB_C,
    NUM_PARAMS 
  };

  enum InputIds {
    TRIGGER_GATE,
    INITIAL_PACKET_INPUT,
    FILTER_INPUT,
    FREQ_CV,
    ATTEN_CV,
    FILTER_CV_A,
    FILTER_CV_B,
    FILTER_CV_C,
    NUM_INPUTS 
  };

  enum OutputIds {
    SYNTH_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightIds {
    LIGHT_PACKET_KNOB,
    LIGHT_PACKET_CV,
    LIGHT_FILTER_KNOB,
    LIGHT_FILTER_CV,

    LIGHT_FILTER_A,
    LIGHT_FILTER_B,
    LIGHT_FILTER_C,
    
    NUM_LIGHTS
  };


  SchmittTrigger voiceTrigger;

  std::vector< KSSynth *> voices;
  const static int nVoices = 32;
  
  KarplusStrongPoly() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS )
  {
    for( int i=0; i<nVoices; ++i ) voices.push_back( new KSSynth(-2.0f, 2.0f, engineGetSampleRate() ) );
    
    initPacketStringDirty = true;
    currentInitialPacket = KSSynth::RANDOM;
    initPacketString = voices[ 0 ]->initPacketName( currentInitialPacket );

    filterStringDirty = true;
    currentFilter = KSSynth::WEIGHTED_ONE_SAMPLE;
    filterString = voices[ 0 ]->filterTypeName( currentFilter );

  }

  virtual ~KarplusStrongPoly()
  {
    for( auto syn : voices )
      delete syn;
  }
  
  int getNumPackets() { return voices[ 0 ]->numInitPackets(); }
  KSSynth::InitPacket currentInitialPacket;

  int getNumFilters() { return voices[ 0 ]->numFilterTypes(); }
  KSSynth::FilterType currentFilter;
  
  void step() override
  {
    int nextInitialPacket = currentInitialPacket;
    float ipi, ipiS;
    lights[ LIGHT_PACKET_CV ].value = inputs[INITIAL_PACKET_INPUT].active;
    lights[ LIGHT_PACKET_KNOB ].value = !inputs[INITIAL_PACKET_INPUT].active;

    lights[ LIGHT_FILTER_CV ].value = inputs[FILTER_INPUT].active;
    lights[ LIGHT_FILTER_KNOB ].value = !inputs[FILTER_INPUT].active;

    // For now, since we only have one filter, hardcode this
    lights[ LIGHT_FILTER_A ].value = 1;
    lights[ LIGHT_FILTER_B ].value = 0;
    lights[ LIGHT_FILTER_C ].value = 0;
    
    
    if( inputs[ INITIAL_PACKET_INPUT ].active )
      {
        ipi = inputs[ INITIAL_PACKET_INPUT ].value;
        ipiS = ipi * getNumPackets() / 10.0;
        nextInitialPacket = (int)(ipiS);
      }
    else
      {
        nextInitialPacket = (int)( params[ INITIAL_PACKET ].value );
      }
    
    if( nextInitialPacket != currentInitialPacket )
      {
        initPacketStringDirty = true;
        currentInitialPacket = (KSSynth::InitPacket)( nextInitialPacket );
        initPacketString = voices[ 0 ]->initPacketName( currentInitialPacket );
      }

    // Check a trigger here and find a voice
    bool newVoice = false;
    if( voiceTrigger.process( inputs[ TRIGGER_GATE ].value ) )
      {
        newVoice = true;
      }

    if( newVoice )
      {
        // find voice
        KSSynth *voice = NULL;
        for( auto syn: voices )
          if( ! syn->active )
            {
              voice = syn;
              break;
            }

        if( voice == NULL )
          {
            // info( "All voices are active: Running voice steal" );
            voice = voices[ 0 ];
            float ds = voice->sumDelaySquared;
            for( auto syn: voices )
              {
                if( syn->sumDelaySquared < ds )
                  {
                    ds = syn->sumDelaySquared;
                    voice = syn;
                  }
              }
          }
        
        // Capture parameters onto this voice and trigger it
        float pitch = params[ FREQ_KNOB ].value + 12.0f * inputs[ FREQ_CV ].value;
        float freq = 261.262f * powf( 2.0f, pitch / 12.0f );


        // For now, since we only have one filter, hardcode this
        voice->filtParamA = clamp( params[ FILTER_KNOB_A ].value + inputs[ FILTER_CV_A ].value * 0.1, 0.0f, 1.0f );
        voice->filtParamB = 0;
        voice->filtParamC = 0;
        
        float atten = params[ ATTEN_KNOB ].value + inputs[ ATTEN_CV ].value;
        voice->packet = currentInitialPacket;
        voice->filtAtten = atten;
        voice->trigger( freq );
      }
    
    float out = 0.0f;
    for( auto syn : voices )
      if( syn->active )
        out += syn->step();

    outputs[ SYNTH_OUTPUT ].value = out;
  }

  bool initPacketStringDirty;
  std::string initPacketString;
  
  static bool getInitialPacketStringDirty( Module *that )
  {
    return dynamic_cast<KarplusStrongPoly *>(that)->initPacketStringDirty;
  }
  static std::string getInitialPacketString( Module *that )
  {
    dynamic_cast<KarplusStrongPoly *>(that)->initPacketStringDirty = false;
    return dynamic_cast<KarplusStrongPoly *>(that)->initPacketString;
  }

  bool filterStringDirty;
  std::string filterString;
  static bool getFilterStringDirty( Module *that )
  {
    return dynamic_cast<KarplusStrongPoly *>(that)->filterStringDirty;
  }
  static std::string getFilterString( Module *that )
  {
    dynamic_cast<KarplusStrongPoly *>(that)->filterStringDirty = false;
    return dynamic_cast<KarplusStrongPoly *>(that)->filterString;
  }
  
};

struct KarplusStrongPolyWidget : ModuleWidget {
  KarplusStrongPolyWidget( KarplusStrongPoly *module);
};

KarplusStrongPolyWidget::KarplusStrongPolyWidget( KarplusStrongPoly *module ) : ModuleWidget( module )
{
  box.size = Vec( SCREW_WIDTH * 15, RACK_HEIGHT );

  BaconBackground *bg = new BaconBackground( box.size, "KarplusStrongPoly" );
 
  
  addChild( bg->wrappedInFramebuffer());

  float outy;
  float yh;
  int margin = 4;
  float gap = 13;
  int obuf = 10;
  
  outy = 35;

  float scale = 1.0;
  bool last = false;

  auto brd = [&](float ys)
    {
      // Add a downward pointing triangle here which means I need a draw glyph
      if( ! last )
        {
          int w = 70;
          addChild( new BufferedDrawLambdaWidget( Vec( bg->cx() - w/2, outy + ys + margin ),
                                                  Vec( w, gap ),
                                                  [=](NVGcontext *vg)
                                                  {
                                                    nvgBeginPath( vg );
                                                    nvgMoveTo( vg, 0, 0 );
                                                    nvgLineTo( vg, w/2, gap );
                                                    nvgLineTo( vg, w, 0 );
                                                    nvgClosePath( vg );
                                                    nvgStrokeColor( vg, COLOR_BLACK );
                                                    nvgStroke( vg );
                                                    nvgFillColor( vg, nvgRGB( 240 * scale, 240 * scale, 200 * scale ) );
                                                    nvgFill( vg );
                                                  }
                                                  )
                  );
        }
      bg->addRoundedBorder( Vec( obuf, outy - margin ),
                            Vec( box.size.x - 2 * obuf, ys + 2 * margin ),
                            nvgRGB( 240*scale, 240*scale, 200*scale ) );

      scale *= 0.92;
    };
  auto cl = [&](std::string lab, float ys)
    {
      bg->addLabel( Vec( obuf + margin, outy + ys / 2 ), lab.c_str(), 13, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT );
    };

  yh = SizeTable<PJ301MPort>::Y;
  brd( yh );
  cl( "Trigger", yh );
  addInput( Port::create< PJ301MPort >( Vec( box.size.x - obuf - margin - SizeTable<PJ301MPort>::X, outy ),
                                        Port::INPUT,
                                        module,
                                        KarplusStrongPoly::TRIGGER_GATE ) );

  outy += yh + 2 * margin + gap;
  yh = SizeTable<RoundBlackKnob >::Y;
  brd( yh );
  cl( "Freq", yh );
  int xp = box.size.x - margin - obuf - SizeTable<PJ301MPort>::X;
  addInput( Port::create< PJ301MPort >( Vec( xp, outy + diffY2c< RoundBlackKnob, PJ301MPort >() ),
                                        Port::INPUT,
                                        module,
                                        KarplusStrongPoly::FREQ_CV ) );

  xp -= SizeTable<RoundBlackKnob>::X + margin;
  addParam( ParamWidget::create< RoundBlackKnob >( Vec( xp, outy ), module,
                                                       KarplusStrongPoly::FREQ_KNOB,
                                                       -54.0f, 54.0f, 0.0f ) );



  outy += yh + 2 * margin + gap;


  yh = SizeTable<RoundBlackSnapKnob>::Y;
  brd( yh );
  cl( "Packet", yh );

  xp = 55;

  addChild( ModuleLightWidget::create< SmallLight< BlueLight > >( Vec( xp - 2, outy - 2 ),
                                                                  module,
                                                                  KarplusStrongPoly::LIGHT_PACKET_KNOB ) );
  addParam( ParamWidget::create< RoundBlackSnapKnob >( Vec( xp, outy ),
                                                       module,
                                                       KarplusStrongPoly::INITIAL_PACKET,
                                                       0,
                                                       module->getNumPackets()-1, 0 ) );


  xp += SizeTable<RoundBlackSnapKnob>::X + margin;
  addChild( ModuleLightWidget::create< SmallLight< BlueLight > >( Vec( xp - 2, outy - 2 + diffY2c<RoundBlackSnapKnob,PJ301MPort>()),
                                                                  module,
                                                                  KarplusStrongPoly::LIGHT_PACKET_CV ) );

  addInput( Port::create<PJ301MPort>( Vec( xp, outy + diffY2c<RoundBlackSnapKnob,PJ301MPort>() ),
                                      Port::INPUT, module, KarplusStrongPoly::INITIAL_PACKET_INPUT ) );
  xp += SizeTable<PJ301MPort>::X + margin;
  addChild( DotMatrixLightTextWidget::create( Vec( xp, outy + diffY2c<RoundBlackSnapKnob,DotMatrixLightTextWidget>() ),
                                              module, 8,
                                              KarplusStrongPoly::getInitialPacketStringDirty,
                                              KarplusStrongPoly::getInitialPacketString ) );


  outy += yh + 2 * margin + gap;

  yh = SizeTable<RoundBlackSnapKnob>::Y + SizeTable<RoundBlackKnob>::Y + margin;
  brd( yh );
  cl( "Filter", SizeTable<RoundBlackKnob>::Y );
  xp = 55;

  addChild( ModuleLightWidget::create< SmallLight< BlueLight > >( Vec( xp - 2, outy - 2 ),
                                                                  module,
                                                                  KarplusStrongPoly::LIGHT_FILTER_KNOB ) );
  addParam( ParamWidget::create< RoundBlackSnapKnob >( Vec( xp, outy ),
                                                       module,
                                                       KarplusStrongPoly::FILTER_TYPE,
                                                       0,
                                                       module->getNumFilters()-1, 0 ) );


  xp += SizeTable<RoundBlackSnapKnob>::X + margin;
  addChild( ModuleLightWidget::create< SmallLight< BlueLight > >( Vec( xp - 2, outy - 2 + diffY2c<RoundBlackSnapKnob,PJ301MPort>()),
                                                                  module,
                                                                  KarplusStrongPoly::LIGHT_FILTER_CV ) );

  addInput( Port::create<PJ301MPort>( Vec( xp, outy + diffY2c<RoundBlackSnapKnob,PJ301MPort>() ),
                                      Port::INPUT, module, KarplusStrongPoly::FILTER_INPUT ) );
  xp += SizeTable<PJ301MPort>::X + margin;
  addChild( DotMatrixLightTextWidget::create( Vec( xp, outy + diffY2c<RoundBlackSnapKnob,DotMatrixLightTextWidget>() ),
                                              module, 8,
                                              KarplusStrongPoly::getFilterStringDirty,
                                              KarplusStrongPoly::getFilterString ) );

  outy += SizeTable<RoundBlackKnob>::Y + 2 * margin;

  xp = obuf + 2.5 * margin;
  for( int i=0; i<3; ++i )
    {
      addChild( ModuleLightWidget::create< SmallLight< BlueLight> >( Vec( xp - 2, outy - 2 ),
                                                                     module,
                                                                     KarplusStrongPoly::LIGHT_FILTER_A + i ) );
      bg->addLabel( Vec( xp, outy + SizeTable<RoundSmallBlackKnob>::Y ),
                    i == 0 ? "A" : i == 1 ? "B" : "C",
                    12,
                    NVG_ALIGN_BOTTOM | NVG_ALIGN_RIGHT );

      xp += 3;

      addParam( ParamWidget::create< RoundSmallBlackKnob >( Vec( xp, outy ),
                                                           module,
                                                           KarplusStrongPoly::FILTER_KNOB_A + i,
                                                           0, 1, 0.5 ) );
      xp += SizeTable<RoundSmallBlackKnob>::X + margin;
      addInput( Port::create<PJ301MPort>( Vec( xp, outy + diffY2c<RoundSmallBlackKnob,PJ301MPort>() ),
                                          Port::INPUT, module, KarplusStrongPoly::FILTER_CV_A + i ) );
      xp += SizeTable<PJ301MPort>::X + 3.5 * margin;
 
    }
  

  outy += yh - SizeTable<RoundBlackKnob>::Y + gap;
  yh = SizeTable< RoundBlackKnob >::Y;
  brd( yh );
  cl( "Atten", yh );
  xp = box.size.x - margin - obuf - SizeTable<PJ301MPort>::X;
  addInput( Port::create< PJ301MPort >( Vec( xp, outy + diffY2c< RoundBlackKnob, PJ301MPort >() ),
                                        Port::INPUT,
                                        module,
                                        KarplusStrongPoly::ATTEN_CV ) );

  xp -= SizeTable<RoundBlackKnob>::X + margin;
  addParam( ParamWidget::create< RoundBlackKnob >( Vec( xp, outy ), module,
                                                   KarplusStrongPoly::ATTEN_KNOB,
                                                   0.1, 4, 1.95 ) );


  outy += yh + 2 * margin + gap;
  last = true;
  brd( SizeTable<PJ301MPort>::Y );
  cl( "Output", SizeTable<PJ301MPort>::Y );
  addOutput( Port::create< PJ301MPort >( Vec( box.size.x - obuf - margin - SizeTable<PJ301MPort>::X, outy ),
                                         Port::OUTPUT,
                                         module,
                                         KarplusStrongPoly::SYNTH_OUTPUT ) );

}

} // namespace rack_plugin_BaconMusic

using namespace rack_plugin_BaconMusic;

RACK_PLUGIN_MODEL_INIT(BaconMusic, KarplusStrongPoly) {
   Model *modelKarplusStrongPoly = Model::create<KarplusStrongPoly, KarplusStrongPolyWidget>("Bacon Music", "KarplusStrongPoly", "KarplusStrongPoly", OSCILLATOR_TAG );
   return modelKarplusStrongPoly;
}

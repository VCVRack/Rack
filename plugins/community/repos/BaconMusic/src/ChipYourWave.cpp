
#include "BaconPlugs.hpp"
#include "ChipSym.hpp"

namespace rack_plugin_BaconMusic {

struct ChipYourWave : virtual Module {
  enum ParamIds {
    FREQ_KNOB,

    WAVEFORM_START,
    NUM_PARAMS = WAVEFORM_START + 32
  };

  enum InputIds {
    FREQ_CV,
    NUM_INPUTS
  };

  enum OutputIds {
    WAVE_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightIds {
    NUM_LIGHTS
  };

  ChipSym::NESArbitraryWaveform narb;

  ChipYourWave() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ),
                   narb( -5.0, 5.0, engineGetSampleRate() )
  {
    narb.setDigWavelength( 2<<8 );
  }

  float digWFInSeconds( float pitchKnob, float pitchCV )
  {
    // This is the frequency tuning used in Fundamental/VCO so lets be consistent
    float pitch = pitchKnob + pitchCV;
    float freq  = 261.626f * powf( 2.0f, pitch / 12.0f );
    // OK so now we have the frequency. We need the wavelength though. Simple
    float wl    = 1.0f / freq;
        
    return wl;
  }
  
  void step() override
  {
    float dwf = digWFInSeconds( params[ FREQ_KNOB ].value, 12.0f * inputs[ FREQ_CV ].value );

    narb.setWavelengthInSeconds( dwf );

    for( int i=0; i<32; ++i )
      narb.setWaveformPoint( i, params[ WAVEFORM_START + i ].value );
    
    if( outputs[ WAVE_OUTPUT ].active )
      outputs[ WAVE_OUTPUT ].value = narb.step();
  }
};

struct ChipYourWaveWidget : ModuleWidget {
  ChipYourWaveWidget( ChipYourWave *module);
};

ChipYourWaveWidget::ChipYourWaveWidget( ChipYourWave *module ) : ModuleWidget( module )
{
  box.size = Vec( SCREW_WIDTH * 23, RACK_HEIGHT );

  BaconBackground *bg = new BaconBackground( box.size, "ChipYourWave" );
  addChild( bg->wrappedInFramebuffer());

  Vec outP = Vec( box.size.x - 40, 45 + 30 );
  bg->addPlugLabel( outP, BaconBackground::SIG_OUT, "out" );
  addOutput( Port::create< PJ301MPort >( outP,
                                         Port::OUTPUT,
                                         module,
                                         ChipYourWave::WAVE_OUTPUT ) );

  bg->addLabel( Vec( 50, 45 ), "Freq", 14, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM );
  addParam( ParamWidget::create< RoundHugeBlackKnob >( Vec( 10, 50 ), module,
                                                        ChipYourWave::FREQ_KNOB, -54.0f, 54.0f, 0.0f ) );
  Vec fcv = Vec( 56 + 20, 45 + 30 );
  bg->addPlugLabel( fcv, BaconBackground::SIG_IN, "v/o" );
  addInput( Port::create< PJ301MPort >( fcv,
                                        Port::INPUT,
                                        module,
                                        ChipYourWave::FREQ_CV ) );

  bg->addLabel( Vec( bg->cx(), 135 ), "Draw your Digital Waveform Here", 14, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM );
  for( int i=0; i<32; ++i )
    {
      addParam( ParamWidget::create< NStepDraggableLEDWidget< 16, RedGreenFromMiddleColorModel >>( Vec( 10 + 10 * i, 140 ), module,
                                                                                                   ChipYourWave::WAVEFORM_START + i,
                                                                                                   0, 15,
                                                                                                   module->narb.getWaveformPoint( i )) );
    }

}

} // namespace rack_plugin_BaconMusic

using namespace rack_plugin_BaconMusic;

RACK_PLUGIN_MODEL_INIT(BaconMusic, ChipYourWave) {
   Model *modelChipYourWave = Model::create<ChipYourWave, ChipYourWaveWidget>("Bacon Music", "ChipYourWave", "ChipYourWave", OSCILLATOR_TAG );
   return modelChipYourWave;
}


#include "BaconPlugs.hpp"
#include "ChipSym.hpp"

namespace rack_plugin_BaconMusic {

struct ChipWaves : virtual Module {
  enum ParamIds {
    FREQ_KNOB,
    PULSE_CYCLE,
    NUM_PARAMS
  };

  enum InputIds {
    FREQ_CV,
    NUM_INPUTS
  };

  enum OutputIds {
    PULSE_OUTPUT,
    TRI_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightIds {
    PULSE_CYCLE_LIGHT,
    NUM_LIGHTS
  };

  ChipSym::NESPulse npulse;
  ChipSym::NESTriangle ntri;

  ChipWaves() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ),
                npulse( -5.0, 5.0, engineGetSampleRate() ),
                ntri( -5.0, 5.0, engineGetSampleRate() )
  {
    npulse.setDigWavelength( 2<<9 );
    ntri.setDigWavelength( 2<<8 );
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

    ntri.setWavelengthInSeconds( dwf );
    npulse.setWavelengthInSeconds( dwf );

    int dc = clamp( (int)(params[ PULSE_CYCLE ].value ), 0, 3 );
    npulse.setDutyCycle( dc );
    lights[ PULSE_CYCLE_LIGHT ].value = dc;
    
    if( outputs[ TRI_OUTPUT ].active )
      outputs[ TRI_OUTPUT ].value = ntri.step();
    if( outputs[ PULSE_OUTPUT ].active )
      outputs[ PULSE_OUTPUT ].value = npulse.step();
    
  }
};

struct ChipWavesWidget : ModuleWidget {
  ChipWavesWidget( ChipWaves *module);
};

ChipWavesWidget::ChipWavesWidget( ChipWaves *module ) : ModuleWidget( module )
{
  box.size = Vec( SCREW_WIDTH * 8, RACK_HEIGHT );

  BaconBackground *bg = new BaconBackground( box.size, "ChipWaves" );
  addChild( bg->wrappedInFramebuffer());

  Vec outP = Vec( bg->cx( 24 ) + 25, RACK_HEIGHT - 15 - 43 );
  bg->addPlugLabel( outP, BaconBackground::SIG_OUT, "pulse" );
  addOutput( Port::create< PJ301MPort >( outP,
                                         Port::OUTPUT,
                                         module,
                                         ChipWaves::PULSE_OUTPUT ) );

  Vec outT = Vec( bg->cx( 24 ) - 25, RACK_HEIGHT - 15 - 43 );
  bg->addPlugLabel( outT, BaconBackground::SIG_OUT, "tri" );
  addOutput( Port::create< PJ301MPort >( outT,
                                         Port::OUTPUT,
                                         module,
                                         ChipWaves::TRI_OUTPUT ) );

  Vec fcv = Vec( bg->cx( 24 ) + 35, 160 );
  bg->addPlugLabel( fcv, BaconBackground::SIG_IN, "v/o" );
  addInput( Port::create< PJ301MPort >( fcv,
                                        Port::INPUT,
                                        module,
                                        ChipWaves::FREQ_CV ) );

  bg->addRoundedBorder( Vec( 10, 140 ), Vec ( 63, 49 ) );
  bg->addLabel( Vec( 40, 144 ), "Duty Cycle", 12, NVG_ALIGN_CENTER | NVG_ALIGN_TOP );
  int ybot = 140 + 24 + 5 + 20;
  addParam( ParamWidget::create< RoundSmallBlackKnob >( Vec( 16, ybot - 3 - 28 ),
                                                        module,
                                                        ChipWaves::PULSE_CYCLE,
                                                        0, 3, 0 ) );
  addChild( ModuleLightWidget::create< SevenSegmentLight< BlueLight, 2 > >( Vec( 47, ybot - 5 - 24 ),
                                                                            module,
                                                                            ChipWaves::PULSE_CYCLE_LIGHT ) );

  
  bg->addLabel( Vec( bg->cx(), 45 ), "Freq", 14, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM );
  addParam( ParamWidget::create< RoundHugeBlackKnob >( Vec( bg->cx( 56 ), 50 ), module,
                                                        ChipWaves::FREQ_KNOB, -54.0f, 54.0f, 0.0f ) );

}

} // namespace rack_plugin_BaconMusic

using namespace rack_plugin_BaconMusic;

RACK_PLUGIN_MODEL_INIT(BaconMusic, ChipWaves) {
   Model *modelChipWaves = Model::create<ChipWaves, ChipWavesWidget>("Bacon Music", "ChipWaves", "ChipWaves", OSCILLATOR_TAG );
   return modelChipWaves;
}

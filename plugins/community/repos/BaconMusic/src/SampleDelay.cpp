
#include "BaconPlugs.hpp"
#include "SampleDelay.hpp"

namespace rack_plugin_BaconMusic {

struct SampleDelayWidget : ModuleWidget {
  typedef SampleDelay< Module > SD;
  SampleDelayWidget( SD *module);
};

SampleDelayWidget::SampleDelayWidget( SD *module ) : ModuleWidget( module )
{
  box.size = Vec( SCREW_WIDTH * 5, RACK_HEIGHT );

  BaconBackground *bg = new BaconBackground( box.size, "SampDelay" );
  addChild( bg->wrappedInFramebuffer());

  int outy = 30;
  int gap = 10;
  int margin = 3;

  // plug label is 29 x 49
  Vec ppos = Vec( bg->cx( SizeTable<PJ301MPort>::X ), outy + 20 );
  bg->addPlugLabel( ppos, BaconBackground::SIG_IN, "in" );
  addInput( Port::create< PJ301MPort >( ppos, Port::INPUT,
                                        module, SD::SIGNAL_IN ) );

  outy += 49 + gap + margin;
  bg->addRoundedBorder( Vec( bg->cx() - 14 * 1.5 - margin, outy - margin ),
                        Vec( 14 * 3 + 2 * margin , 14 + SizeTable<RoundBlackSnapKnob>::Y + 2 * margin + 22 + margin + 2 * margin) );
  
  bg->addLabel( Vec( bg->cx(), outy ), "# samples", 11, NVG_ALIGN_CENTER | NVG_ALIGN_TOP );
  outy += 14;
  addParam( ParamWidget::create< RoundBlackSnapKnob >( Vec( bg->cx( SizeTable< RoundBlackSnapKnob >::X ), outy ),
                                                       module,
                                                       SD::DELAY_KNOB,
                                                       1, 99, 1 ) );

  outy += SizeTable<RoundBlackSnapKnob>::Y + 2 * margin;
  addChild( MultiDigitSevenSegmentLight<BlueLight, 2, 3>::create( Vec( bg->cx() - 14 * 1.5, outy ),
                                                                  module,
                                                                  SD::DELAY_VALUE_LIGHT ) );
  outy += 22 + gap + margin;

  ppos = Vec( bg->cx( SizeTable<PJ301MPort>::X ), outy + 20 );
  bg->addPlugLabel( ppos, BaconBackground::SIG_OUT, "out" );
  addOutput( Port::create< PJ301MPort >( ppos, Port::OUTPUT,
                                         module, SD::SIGNAL_OUT ) );

}

} // namespace rack_plugin_BaconMusic

using namespace rack_plugin_BaconMusic;

RACK_PLUGIN_MODEL_INIT(BaconMusic, SampleDelay) {
   Model *modelSampleDelay = Model::create<SampleDelayWidget::SD, SampleDelayWidget>("Bacon Music", "SampleDelay", "SampleDelay", DELAY_TAG ); 
   return modelSampleDelay;
}

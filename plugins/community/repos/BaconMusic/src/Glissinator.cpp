#include "BaconPlugs.hpp"
#include "Glissinator.hpp"

namespace rack_plugin_BaconMusic {

struct GlissinatorWidget : ModuleWidget {
  typedef Glissinator< Module > G;
  GlissinatorWidget( Glissinator<Module> *model );
};

GlissinatorWidget::GlissinatorWidget( Glissinator<Module> *model ) : ModuleWidget( model )
{
  box.size = Vec( SCREW_WIDTH * 5, RACK_HEIGHT );
  BaconBackground *bg = new BaconBackground( box.size, "Glissinator" );

  addChild( bg->wrappedInFramebuffer() );
  // FIXME - spacing
  // addChild( new BaconHelpButton( "README.md#glissinator" ) );
  
  ParamWidget *slider = ParamWidget::create< GraduatedFader< 230 > >( Vec( bg->cx( 29 ), 23 ),
                                                              module,
                                                              G::GLISS_TIME,
                                                              0,
                                                              1,
                                                              0.1 );
  
  addParam( slider );

  Vec inP = Vec( 7, RACK_HEIGHT - 15 - 43 );
  Vec outP = Vec( box.size.x - 24 - 7, RACK_HEIGHT - 15 - 43 );
  
  bg->addPlugLabel( inP, BaconBackground::SIG_IN, "in" );
  addInput( Port::create< PJ301MPort >( inP, Port::INPUT,
                                       module,
                                       G::SOURCE_INPUT ) );

  bg->addPlugLabel( outP, BaconBackground::SIG_OUT, "out" );
  
  addOutput( Port::create< PJ301MPort >( outP, Port::OUTPUT,
                                         module,
                                         G::SLID_OUTPUT ) );

  bg->addRoundedBorder( Vec( 5, RACK_HEIGHT - 120 ), Vec( box.size.x - 10, 38 ), BaconBackground::highlight );
  bg->addLabel( Vec( 10, RACK_HEIGHT - 102 ), "gliss", 11, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM, COLOR_WHITE );
  bg->addLabel( Vec( 10, RACK_HEIGHT - 90 ), "gate", 11, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM, COLOR_WHITE );
  addChild( ModuleLightWidget::create< SmallLight< BlueLight > >( Vec( bg->cx() - 4 , RACK_HEIGHT - 120 + 38 / 2 - 3 ),
                                                    module, G::SLIDING_LIGHT ) );
  addOutput( Port::create< PJ301MPort >( Vec( bg->cx() + 5, RACK_HEIGHT - 114 ), Port::OUTPUT, module, G::GLISSING_GATE ) );
}

} // namespace rack_plugin_BaconMusic

using namespace rack_plugin_BaconMusic;

RACK_PLUGIN_MODEL_INIT(BaconMusic, Glissinator) {
   Model *modelGlissinator = Model::create<Glissinator<Module>,GlissinatorWidget>("Bacon Music", "Glissinator", "Glissinator", EFFECT_TAG); 
   return modelGlissinator;
}

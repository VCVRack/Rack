#if 0

#include "BaconPlugs.hpp"

struct MODULE_NAME : virtual Module {
  enum ParamIds {
    NUM_PARAMS
  };

  enum InputIds {
    NUM_INPUTS
  };

  enum OutputIds {
    NUM_OUTPUTS
  };

  enum LightIds {
    NUM_LIGHTS
  };

  MODULE_NAME() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS )
  {
  }

  void step() override
  {
  }
};

struct WIDGET_NAME : ModuleWidget {
  WIDGET_NAME( MODULE_NAME *module);
};

WIDGET_NAME::WIDGET_NAME( MODULE_NAME *module ) : ModuleWidget( module )
{
  box.size = Vec( SCREW_WIDTH * 8, RACK_HEIGHT );

  BaconBackground *bg = new BaconBackground( box.size, "MODULE_NAME" );
  addChild( bg->wrappedInFramebuffer());
}

Model *modelMODULE_NAME = Model::create<MODULE_NAME, WIDGET_NAME>("Bacon Music", "MODULE_NAME", "MODULE_NAME", RACK_REPLACE_WITH_TAG); 

#endif

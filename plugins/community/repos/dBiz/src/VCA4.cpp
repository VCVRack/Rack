///////////////////////////////////////////////////////////////////
//
//  dBiz revisited version of Cartesian seq. by Strum 
// 
///////////////////////////////////////////////////////////////////

#include "dBiz.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_dBiz {

struct VCA4 : Module {
    enum ParamIds
    {
        
        CV_PARAM,
        MUTE_PARAM = CV_PARAM+16,
        NUM_PARAMS = MUTE_PARAM + 16
    };
    enum InputIds
    {
        CH_INPUT,
        CV_INPUT=CH_INPUT+4,
        NUM_INPUTS=CV_INPUT+16
    };
	enum OutputIds 
    {
	    CH_OUTPUT,  
	    NUM_OUTPUTS=CH_OUTPUT+4
    };
    enum LightIds
    {
      MUTE_LIGHT,
      NUM_LIGHTS =MUTE_LIGHT+16
    };

    SchmittTrigger mute_triggers[16];
    bool mute_states[16] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

    float ch_in[4];
    float ch_out[4];
    float cv_val[16];
    

    VCA4() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;

    json_t *toJson() override
    {
        json_t *rootJ = json_object();

        // mute states
        json_t *mute_statesJ = json_array();
        for (int i = 0; i < 16; i++)
        {
            json_t *mute_stateJ = json_integer((int)mute_states[i]);
            json_array_append_new(mute_statesJ, mute_stateJ);
        }
        json_object_set_new(rootJ, "mutes", mute_statesJ);
        return rootJ;
    }

    void fromJson(json_t *rootJ) override
    {
        // mute states
        json_t *mute_statesJ = json_object_get(rootJ, "mutes");
        if (mute_statesJ)
        {
            for (int i = 0; i < 16; i++)
            {
                json_t *mute_stateJ = json_array_get(mute_statesJ, i);
                if (mute_stateJ)
                    mute_states[i] = !!json_integer_value(mute_stateJ);
            }
        }
    }
};

void VCA4::step() {

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (mute_triggers[i + j * 4].process(params[MUTE_PARAM + i + j * 4].value))
            {
                mute_states[i + j * 4] = !mute_states[+i + j * 4];
            }
            lights[MUTE_LIGHT + i + j * 4].value = mute_states[i + j * 4] ? 1.0 : 0.0;
        }
    }

    for (int i = 0; i < 4; i++)
    {
        ch_in[i] = inputs[CH_INPUT + i].value;
    }

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (!mute_states[i + j * 4])
            {
                cv_val[i + j * 4] = 0.0;
            }
            else
                cv_val[i + j * 4] = params[CV_PARAM + i + j * 4].value*clamp(inputs[CV_INPUT + i+j*4].normalize(10.0f) / 10.0f, 0.0f, 1.0f);
        }
    }

    for (int i = 0; i < 4; i++)
    {
        outputs[CH_OUTPUT + i ].value =0.4* (ch_in[0]*cv_val[i] + ch_in[0]*cv_val[i] + ch_in[0]*cv_val[i] + ch_in[0]*cv_val[i]);
    }
    for (int i = 0; i < 4; i++)
    {
        outputs[CH_OUTPUT + i ].value +=0.4* (ch_in[1] * cv_val[i+4] + ch_in[1] * cv_val[i+4] + ch_in[1] * cv_val[i+4] + ch_in[1] * cv_val[i+4]);
    }
     for (int i = 0; i < 4; i++)
     {
         outputs[CH_OUTPUT + i].value +=0.4* (ch_in[2] * cv_val[i + 8] + ch_in[2] * cv_val[i + 8] + ch_in[2] * cv_val[i + 8] + ch_in[2] * cv_val[i + 8]);
     }
     for (int i = 0; i < 4; i++)
     {
         outputs[CH_OUTPUT + i].value +=0.4* (ch_in[3] * cv_val[i+12] + ch_in[3] * cv_val[i+12] + ch_in[3] * cv_val[i+12] + ch_in[3] * cv_val[i+12]);
     }
}










    /*for (int i = 0; i < 4; i++)
    {
        ch_in[i]=inputs[CH_INPUT+i].value;
    }

        
    for (int i = 0; i < 4; i++)
    {
        outputs[CH_OUTPUT+i].value=ch_out[i];
    }
}*/

////////////////////////////////

   struct VCA4Widget : ModuleWidget 
   {
      VCA4Widget(VCA4 *module) : ModuleWidget(module)
         {
            box.size = Vec(15*20, 380);
  
            {
               SVGPanel *panel = new SVGPanel();
               panel->box.size = box.size;
               panel->setBackground(SVG::load(assetPlugin(plugin,"res/VCA4.svg")));
               addChild(panel);
            }
 
            int top = 20;
            int left = 2;
            int column_spacing = 35; 
            int row_spacing = 30;
            int button_offset = 20;

            // addOutput(Port::create<PJ301MOrPort>(Vec(130, 10), Port::OUTPUT, module, VCA4::X_OUT));  
            // addOutput(Port::create<PJ301MOrPort>(Vec(130, 40), Port::OUTPUT, module, VCA4::Y_OUT));
            // addOutput(Port::create<PJ301MOrPort>(Vec(130, 70), Port::OUTPUT, module, VCA4::G_OUT));

            for (int i = 0; i < 4; i++)
            {
               for ( int j = 0 ; j < 4 ; j++)
               {

                  addParam(ParamWidget::create<LEDButton>(Vec(button_offset + left + column_spacing * i+140, top + row_spacing * j + 170), module, VCA4::MUTE_PARAM + i + j * 4, 0.0, 1.0, 0.0));
                  addChild(GrayModuleLightWidget::create<BigLight<OrangeLight>>(Vec(button_offset + column_spacing * i+140, top + row_spacing * j + 170 ), module, VCA4::MUTE_LIGHT + i + j * 4));

                  addParam(ParamWidget::create<Trimpot>(Vec(10+column_spacing * i, top + row_spacing * j + 170), module, VCA4::CV_PARAM + i + j * 4, 0.0, 1.0, 0.0));
               }
            }

            for (int i = 0; i < 4; i++)
            {
               addInput(Port::create<PJ301MIPort>(Vec(30,24+40*i), Port::INPUT, module, VCA4::CH_INPUT + i));
            }
            for (int i = 0; i < 4; i++)
            {
               for (int j = 0; j < 4; j++)
               {
                  if (j == 0 || j==2 )
                     addInput(Port::create<PJ301MIPort>(Vec(column_spacing * 1.5 * i + 100, 60 + row_spacing * j), Port::INPUT, module, VCA4::CV_INPUT + i + j * 4));
                  else
                     addInput(Port::create<PJ301MIPort>(Vec(column_spacing * 1.5 * i + 70, 60 + row_spacing * j), Port::INPUT, module, VCA4::CV_INPUT + i + j * 4));
               }
            }

            for (int i = 0; i < 4; i++)
            {
               addOutput(Port::create<PJ301MRPort>(Vec(70 + row_spacing *1.9* i,24), Port::OUTPUT, module, VCA4::CH_OUTPUT + i));
            }
            addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
            addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
            addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
            addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));
         }
   };

} // namespace rack_plugin_dBiz

using namespace rack_plugin_dBiz;

RACK_PLUGIN_MODEL_INIT(dBiz, VCA4) {
   Model *modelVCA4 = Model::create<VCA4, VCA4Widget>("dBiz", "VCA4", "VCA4", UTILITY_TAG);
   return modelVCA4;
}

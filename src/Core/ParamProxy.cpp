/*
Copyright (c) 2019 bsp

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "global_pre.hpp"
#include "Core.hpp"
#include "global_ui.hpp"

using namespace rack;

extern void rack::engineSetParam(Module *module, int paramId, float value, bool bVSTAutomate);

namespace rack {
extern bool vst2_find_module_and_paramid_by_unique_paramid(int uniqueParamId, Module**retModule, int *retParamId);
}

struct ParamProxy;

// struct TrigButton : TL1105 {
// struct ParamProxyTrigButton : CKD6 {
struct ParamProxyTrigButton : LEDButton {
   ParamWidget *id_widget;

   ParamProxyTrigButton() {
		addFrame(SVG::load(assetGlobal("res/ComponentLibrary/LEDButtonLit.svg")));
   }

	void onAction(EventAction &e) override;
	void onDragEnd(EventDragEnd &e) override;
};

struct NullButton : SVGSwitch, ToggleSwitch {
	NullButton() {
		addFrame(SVG::load(assetPlugin("res/null.svg")));
		addFrame(SVG::load(assetPlugin("res/null.svg")));
   }
};

struct RoundSmallBlackKnobParamId : RoundSmallBlackKnob {
	RoundSmallBlackKnobParamId() {
	}

	void onChange(EventChange &e) override;
};

struct ParamProxy : Module {

   static const uint32_t NUM_PARAM_ROWS = 8u;

   enum RowParamIds {
		ROW_PARAM_CONSTVAL,
      ROW_PARAM_MIN,
      ROW_PARAM_MAX,
      ROW_PARAM_PARAMID,
      ROW_PARAM_LEARNFROMCLIPBOARD,
      NUM_ROW_PARAMS
   };

   enum ParamIds {
		PARAM_0_CONSTVAL,
      PARAM_0_MIN,
      PARAM_0_MAX,
      PARAM_0_PARAMID,
      PARAM_0_LEARNFROMCLIPBOARD,

		PARAM_1_CONSTVAL,
      PARAM_1_MIN,
      PARAM_1_MAX,
      PARAM_1_PARAMID,
      PARAM_1_LEARNFROMCLIPBOARD,

		PARAM_2_CONSTVAL,
      PARAM_2_MIN,
      PARAM_2_MAX,
      PARAM_2_PARAMID,
      PARAM_2_LEARNFROMCLIPBOARD,

		PARAM_3_CONSTVAL,
      PARAM_3_MIN,
      PARAM_3_MAX,
      PARAM_3_PARAMID,
      PARAM_3_LEARNFROMCLIPBOARD,

		PARAM_4_CONSTVAL,
      PARAM_4_MIN,
      PARAM_4_MAX,
      PARAM_4_PARAMID,
      PARAM_4_LEARNFROMCLIPBOARD,

		PARAM_5_CONSTVAL,
      PARAM_5_MIN,
      PARAM_5_MAX,
      PARAM_5_PARAMID,
      PARAM_5_LEARNFROMCLIPBOARD,

		PARAM_6_CONSTVAL,
      PARAM_6_MIN,
      PARAM_6_MAX,
      PARAM_6_PARAMID,
      PARAM_6_LEARNFROMCLIPBOARD,

		PARAM_7_CONSTVAL,
      PARAM_7_MIN,
      PARAM_7_MAX,
      PARAM_7_PARAMID,
      PARAM_7_LEARNFROMCLIPBOARD,

		NUM_PARAMS
	};

	enum InputIds {
      INPUT_0_CV,
      INPUT_1_CV,
      INPUT_2_CV,
      INPUT_3_CV,
      INPUT_4_CV,
      INPUT_5_CV,
      INPUT_6_CV,
      INPUT_7_CV,
		NUM_INPUTS
	};

	enum OutputIds {
		NUM_OUTPUTS
	};

	enum LightIds {
		NUM_LIGHTS
	};

   bool b_update_leds;
   float last_output_state[NUM_PARAM_ROWS];

   void resetOutputState(void) {
      for(uint32_t rowIdx = 0u; rowIdx < ParamProxy::NUM_PARAM_ROWS; rowIdx++)
      {
         last_output_state[rowIdx] = 0.0f;
      }
   }

	ParamProxy() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
      b_update_leds = true;
      resetOutputState();
   }

	void step() override;
};

void ParamProxy::step() {

   int rowParamBaseId = ParamProxy::PARAM_0_CONSTVAL;

   for(uint32_t rowIdx = 0u; rowIdx < ParamProxy::NUM_PARAM_ROWS; rowIdx++)
   {
      float curVal;

      if(inputs[rowIdx].active)
      {
         // Use CV input when a cable's plugged in
         curVal = inputs[rowIdx].value * (1.0f / 10.0f);

         if(curVal < 0.0f)
            curVal = 0.0f;
         else if(curVal > 1.0f)
            curVal = 1.0f;
      }
      else
      {
         // Use constant value when nothing's connected
         curVal = params[rowParamBaseId + ParamProxy::ROW_PARAM_CONSTVAL].value;
      }

      int targetParamGID = int(params[rowParamBaseId + ParamProxy::ROW_PARAM_PARAMID].value);
      
      if(targetParamGID > 0)
      {
            // Rescale to min/max range
         float minVal = params[rowParamBaseId + ParamProxy::ROW_PARAM_MIN].value;
         float maxVal = params[rowParamBaseId + ParamProxy::ROW_PARAM_MAX].value;
         if(minVal > maxVal)
         {
            float t = minVal;
               minVal = maxVal;
               maxVal = t;
         }
         curVal = minVal + (maxVal - minVal) * curVal;

         if(curVal != last_output_state[rowIdx])
         {
            last_output_state[rowIdx] = curVal;

            // Find target module + param
            //  (todo) should we lock global_ui->app.mtx_param for this ?
            Module *module;
            int paramId;
            if(vst2_find_module_and_paramid_by_unique_paramid(targetParamGID, &module, &paramId))
            {
               ModuleWidget *moduleWidget = global_ui->app.gRackWidget->findModuleWidgetByModule(module);
               if(NULL != moduleWidget)
               {
                  // Find 
                  ParamWidget *paramWidget = moduleWidget->findParamWidgetByParamId(paramId);
                  if(NULL != paramWidget)
                  {
                     if(isfinite(paramWidget->minValue) && isfinite(paramWidget->maxValue))
                     {
                        // De-Normalize parameter
                        global_ui->param_info.b_lock = true;
                        float paramRange = (paramWidget->maxValue - paramWidget->minValue);
                        if(paramRange > 0.0f)
                        {
                           float value = (curVal * paramRange) + paramWidget->minValue;
                           // Dprintf("ParamProxy: paramId=%d value=%f min=%f max=%f\n", paramId, value, paramWidget->minValue, paramWidget->maxValue);
                           engineSetParam(module, paramId, value, false/*bVSTAutomate*/);

                           // Update UI widget
                           paramWidget->setValue(value);
                        }
                        global_ui->param_info.b_lock = false;
                     }
                  }
               }
            } // if find paramid
         } // if value has changed
      } // if targetParamGID > 0

      // Next param row
      rowParamBaseId += ParamProxy::NUM_ROW_PARAMS;

   } // loop param rows
}


struct ParamProxyWidget : ModuleWidget {
   ParamWidget *bt_widgets[ParamProxy::NUM_PARAM_ROWS];

	ParamProxyWidget(ParamProxy *module);

	void draw(NVGcontext *vg) override;

	void fromJson(json_t *rootJ) override {
      ModuleWidget::fromJson(rootJ);
      ParamProxy *ppMod = dynamic_cast<ParamProxy*>(module);
      ppMod->b_update_leds = true;
      ppMod->resetOutputState();
   }
};

ParamProxyWidget::ParamProxyWidget(ParamProxy *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetGlobal("res/Core/ParamProxy.svg")));

   addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
   addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
   addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
   addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


   sF32 cy = 63.0f;
   int rowParamBaseId = ParamProxy::PARAM_0_CONSTVAL;

   for(uint32_t rowIdx = 0u; rowIdx < ParamProxy::NUM_PARAM_ROWS; rowIdx++)
   {
      sF32 cx =  2.0f;
      sF32 cyk = cy + 1.2f;  // knob
      sF32 cyt = cy + 3.3f;  // copyfromclipboard button

#define PORT_STX 27.0f
#define KNOB_STX 25.0f

      // CV Input
      addInput(Port::create<PJ301MPort>(Vec(cx, cy), Port::INPUT, module, ParamProxy::INPUT_0_CV + int32_t(rowIdx)));
      cx += PORT_STX;

      // Const Val
      addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cx, cyk), module, rowParamBaseId + ParamProxy::ROW_PARAM_CONSTVAL, 0.0f, 1.0f, 0.5f));
      cx += KNOB_STX;

      // Min Val
      addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cx, cyk), module, rowParamBaseId + ParamProxy::ROW_PARAM_MIN, 0.0f, 1.0f, 0.0f));
      cx += KNOB_STX;

      // Max Val
      addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(cx, cyk), module, rowParamBaseId + ParamProxy::ROW_PARAM_MAX, 0.0f, 1.0f, 1.0f));
      cx += KNOB_STX;

      // Param Id
      ParamWidget *idWidget = ParamWidget::create<RoundSmallBlackKnobParamId>(Vec(cx, cyk), module, rowParamBaseId + ParamProxy::ROW_PARAM_PARAMID, 0.0f, 10000.0f, 0.0f);
      addParam(idWidget);
      cx += KNOB_STX;

      // Learn from clipboard button
      ParamProxyTrigButton *bt = ParamWidget::create<ParamProxyTrigButton>(Vec(cx, cyt), module, rowParamBaseId + ParamProxy::ROW_PARAM_LEARNFROMCLIPBOARD, 0.0f, 1.0f, 0.0f);
      bt->id_widget = idWidget;
      bt_widgets[rowIdx] = bt;
      addParam(bt);

      // Next param row (aligned to row height of Core.Notes module)
      cy += 36.0f;
      rowParamBaseId += ParamProxy::NUM_ROW_PARAMS;
   }

   module->b_update_leds = true;
}

void ParamProxyWidget::draw(NVGcontext *vg) {

   // Highlight button when the corresponding row has a valid target param
   ParamProxy *ppMod = dynamic_cast<ParamProxy*>(module);
   if(ppMod->b_update_leds)
   {
      ppMod->b_update_leds = false;
      int paramIdParamId = ParamProxy::PARAM_0_PARAMID;

      for(uint32_t rowIdx = 0u; rowIdx < ParamProxy::NUM_PARAM_ROWS; rowIdx++)
      {
         float btState = (module->params[paramIdParamId].value > 0.0f) ? 1.0f : 0.0f;
         bt_widgets[rowIdx]->setValue(btState);
         paramIdParamId += ParamProxy::NUM_ROW_PARAMS;
      }

   }

   ModuleWidget::draw(vg);
}

void RoundSmallBlackKnobParamId::onChange(EventChange &e) {
   RoundSmallBlackKnob::onChange(e);
   ParamProxy *ppMod = dynamic_cast<ParamProxy*>(module);
   ppMod->b_update_leds = true;
}

void ParamProxyTrigButton::onAction(EventAction &e) {
   if(-1 != rack::global_ui->param_info.gid_clipboard)
   {
      printf("xxx ParamProxyTrigButton: copy clipboard param id %d to proxy param %d\n",
             rack::global_ui->param_info.gid_clipboard,
             paramId - 1/*ROW_PARAM_PARAMID*/
             );
      float gidf = float(rack::global_ui->param_info.gid_clipboard);
      module->params[paramId - 1/*ROW_PARAM_PARAMID*/].value = gidf;
      id_widget->setValue(gidf);
      setValue((gidf > 0.0f) ? 1.0f : 0.0f);
      ParamProxy *ppMod = dynamic_cast<ParamProxy*>(module);
      ppMod->b_update_leds = true;
   }
}

void ParamProxyTrigButton::onDragEnd(EventDragEnd &e) {
   ParamProxy *ppMod = dynamic_cast<ParamProxy*>(module);
   ppMod->b_update_leds = true;
}

RACK_PLUGIN_MODEL_INIT(Core, ParamProxy) {
   Model *modelParamProxy = Model::create<ParamProxy, ParamProxyWidget>("Core", "ParamProxy", "ParamProxy", CONTROLLER_TAG);
   return modelParamProxy;
}

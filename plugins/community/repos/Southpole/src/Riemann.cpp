
#include <array>

#include "Southpole.hpp"
#include "dsp/digital.hpp"

#include "Bjorklund.hpp"

namespace rack_plugin_Southpole {

const float radius = 35.;

#define MAXPARTS 7
#define TNROWS 12

struct Note {

   // pitch class;
   int pc;
   float active;

   // logical position
   int nx;
   int ny;

   // projected position
   float x;
   float y;
};

struct Riemann : Module {
   enum ParamIds {
      GROUP_PARAM,
      SUS_PARAM,
      TRANSP_PARAM,
      PARTS_PARAM,
      VOICING_PARAM,
      NUM_PARAMS
   };
   enum InputIds {
      X_INPUT,
      Y_INPUT,
      TRANSP_INPUT,
      PARTS_INPUT,
      VOICING_INPUT,
      NUM_INPUTS
   };
   enum OutputIds {
      N0_OUTPUT,
      NUM_OUTPUTS = 1+MAXPARTS
   };
   enum LightIds {      
      NUM_LIGHTS
   };

   Riemann() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
      reset();

   }

   void step() override;
   void reset() override;

   // circle of fifths
   const int cof[12] = { 0,7,2,9,4,11,6,1,8,3,10,5 };

   /*
   //chroma vectors
   const int cn[5][12] =
      { { 1,1,1,1,1,1,1,1,1,1,1,1 },   // Chromatic
      { 1,0,1,0,1,1,0,1,0,1,0,1 },  // Major scale
      { 1,0,1,1,0,1,0,1,1,0,0,1 },  // minor scale
      { 1,0,0,0,1,0,0,1,0,0,0,0 },  // M
      { 1,0,0,1,0,0,0,1,0,0,0,0 },  // m
      };

   enum chroma_vector_names {
      CHROMATIC_CHROMA,
      MAJOR_CHROMA,
      MINOR_CHROMA,
      M3_CHROMA,
      m3_CHROMA,
      NUM_CHROMAS
   };
   */

   enum chord_type_names {
      MAJ_CHORD,
      MIN_CHORD,
      AUG_CHORD,
      DIM_CHORD,
      SUS_CHORD
   };

   // Tonnetz torus aligned to major thirds 
   // -> three rows are enough, fourth is for display, twelve completes the map
   Note notes[TNROWS][12];
   Note *chord[MAXPARTS];
   int octave[MAXPARTS];
   int parts;
   int voicing;
   int transp;
   int scale;
   chord_type_names chord_type;
   bool chord_group;

   //float freq[12];

   // playhead logical position
   int nx;
   int ny;

   // playhead geometrical
   float x0;
   float y0;
};

void Riemann::reset() {
/*
   float ratio = pow(2., 1./12.);
   for (int i=0; i<12; i++) {
      freq[i] = 220. * pow(ratio, i+3);
      printf("%f\n", freq[i]);
   }
*/
   for (int j=0; j<12; j++) {
      for (int i=0; i<12; i++) {

         notes[j][i].pc = cof[(i+10 + j*8)%12];

         float x0 = i;
         float y0 = j;
         float phi = -(x0 - y0/2.)*2.*M_PI / 12.;
         float   r = radius + 25.*y0;

         float x = r*sin(phi);
         float y = r*cos(phi);
         //float x = 30*(x0-6);
         //float y = 30*(y0-1);

         notes[j][i].x  = x;
         notes[j][i].y  = y;
         notes[j][i].nx = i;
         notes[j][i].ny = j;
      }
   }

   for ( int i=0; i< MAXPARTS; i++ ) {
      chord[i] = &notes[0][i];
      octave[i] = 1;
   }

   voicing = 0;
   scale = 0;
   parts = 7;
}

void Riemann::step() {

   x0 = inputs[X_INPUT].normalize(0.)+6;
   y0 = inputs[Y_INPUT].normalize(0.)+1;

   //parts   = clamp( inputs[PARTS_INPUT].value/10. + params[PARTS_PARAM].value, 0., 1.)*(MAXPARTS-3.)+3.;
   parts   = clamp( params[PARTS_PARAM].value, 0.f, 1.f)*(MAXPARTS-3.)+3.;
   voicing = clamp( inputs[VOICING_INPUT].value/10. + params[VOICING_PARAM].value, -1.f, 1.f)*4.*parts;
   transp  = clamp( inputs[TRANSP_INPUT].value/10. + params[TRANSP_PARAM].value, 0.f, 1.f)*11.;

   while (x0 <   0.) { x0 += 12.; }
   while (x0 >= 12.) { x0 -= 12.; }

   while (y0 < 0. ) { y0 += 3.; }
   while (y0 >= 3.) { y0 -= 3.; }

   // determine active triad (diagonal division of logical square cell)
   nx = floor(x0);
   ny = floor(y0);

   if ( params[GROUP_PARAM].value > 0. ) {
      if ( y0-ny > x0-nx  ) {
         chord_type = MAJ_CHORD;
      } else {
         chord_type = MIN_CHORD;
      }
   } else {
      if ( y0-ny > x0-nx  ) {
         chord_type = AUG_CHORD;
      } else {
         chord_type = DIM_CHORD;
      }
   }

   // (m/M) (M/A) (A/M) (M/m) (m/d) (d/m)

   //type = DIM_CHORD;
    if ( params[SUS_PARAM].value > 0.5 && fabs(y0-ny) < 0.3 ) {
      chord_type = SUS_CHORD;
   }

   switch(  chord_type ) { 
   
      case MAJ_CHORD:

         // Major (0 4 7 e 2 6 9)
         chord[0] = &notes[(ny+1)%TNROWS][ nx     ];
         chord[1] = &notes[ ny          ][ nx     ];
         chord[2] = &notes[(ny+1)%TNROWS][(nx+1)%12];
         chord[3] = &notes[ ny          ][(nx+1)%12];
         chord[4] = &notes[(ny+1)%TNROWS][(nx+2)%12];
         chord[5] = &notes[ ny          ][(nx+2)%12];
         chord[6] = &notes[(ny+1)%TNROWS][(nx+3)%12];
         break;

      case MIN_CHORD:

         // minor (0 3 7 t 2 5 9)
         chord[0] = &notes[ ny         ][ nx      ];
         chord[1] = &notes[(ny+1)%TNROWS][(nx+1)%12];
         chord[2] = &notes[ ny         ][(nx+1)%12];
         chord[3] = &notes[(ny+1)%TNROWS][(nx+2)%12];
         chord[4] = &notes[ ny         ][(nx+2)%12];
         chord[5] = &notes[(ny+1)%TNROWS][(nx+3)%12];
         chord[6] = &notes[ ny          ][(nx+3)%12];
         break;

      case AUG_CHORD: 

         // augmented (wrap??)
         for (int i=0; i<MAXPARTS; i++) { chord[i] = &notes[(12+ny-i)%3][ nx ]; }
         break;

      case DIM_CHORD: 

         // diminished
         for (int i=0; i<MAXPARTS; i++) { chord[i] = &notes[(ny+i)%3][(nx+i)%12]; }
         break;

      case SUS_CHORD: 

         // suspended (sus4)
         //for (int i=0; i<MAXPARTS; i++) { 
         chord[0] = &notes[ ny ][(nx  )%12]; 
         chord[1] = &notes[ ny ][(nx+2)%12]; 
         chord[2] = &notes[ ny ][(nx+1)%12]; 
         chord[3] = &notes[ ny ][(nx+4)%12]; 
         chord[4] = &notes[ ny ][(nx+3)%12]; 
         chord[5] = &notes[ ny ][(nx+6)%12]; 
         chord[6] = &notes[ ny ][(nx+5)%12]; 
         //}
         break;

      default:
         break;
   }  

   //count octaves
   int o=0;
   octave[0] = o;
   for (int i=1; i<MAXPARTS; i++) { 
      if ( chord[i]->pc < chord[i-1]->pc ) o++;
      octave[i] = o;
   }
   
   // add voicing 
   if (voicing > 0) {
      for (int i=0; i< voicing; i++) { octave[i%parts]++; }
   } else {
      for (int i=0; i< abs(voicing); i++) { octave[i%parts]--; }
   }   

   // tonic
   outputs[N0_OUTPUT].value = octave[0]+transp/12.;
   // chord
   for (int i=0; i<parts; i++) { 
      outputs[N0_OUTPUT+i+1].value = octave[i]+(transp + chord[i]->pc)%12/12.;
   }  
   for (int i=parts; i< MAXPARTS; i++) { 
      outputs[N0_OUTPUT+i+1].value = octave[0]+(transp + chord[0]->pc)%12/12.;
   }  
}


struct RiemannDisplay : TransparentWidget {

   Riemann *module;
   int frame = 0;
   std::shared_ptr<Font> font;

   // draw note names   
   const char *note_names[12] = { 
      "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" 
      //"C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" 
      //"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "t", "e" 
   };

   float cx;
   float cy;

   RiemannDisplay() {     
     font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
   }

   void drawChordTriads(NVGcontext *vg) {

      // draw active chord - triangles
      if (module->parts >= 3)
      for (int i=0; i< module->parts-2; i++) {
         nvgBeginPath(vg);
         nvgFillColor(vg, nvgRGBA(0x7f, 0x10, 0x10, 0xff));
         nvgMoveTo(vg, module->chord[i  ]->x + cx, module->chord[i  ]->y + cy);
         nvgLineTo(vg, module->chord[i+1]->x + cx, module->chord[i+1]->y + cy);
         nvgLineTo(vg, module->chord[i+2]->x + cx, module->chord[i+2]->y + cy);
         nvgClosePath(vg);
         nvgFill(vg);
      }
    }

   void drawChordPath(NVGcontext *vg) {

      // draw active chord path
      if (module->chord_type == Riemann::MAJ_CHORD || module->chord_type == Riemann::MIN_CHORD ) {

         nvgBeginPath(vg);
         nvgStrokeWidth(vg, 2.);       
         nvgStrokeColor(vg, nvgRGBA(0xff, 0x10, 0x10, 0xff));
         for (int i=0; i<module->parts-1; i++)  {
            nvgMoveTo(vg, module->chord[i  ]->x + cx, module->chord[i  ]->y + cy);
            nvgLineTo(vg, module->chord[i+1]->x + cx, module->chord[i+1]->y + cy);
         }
         nvgStroke(vg);
         
      } else if (module->chord_type == Riemann::SUS_CHORD ) {
            nvgBeginPath(vg);
            nvgStrokeWidth(vg, 2.);       
            nvgStrokeColor(vg, nvgRGBA(0xff, 0x10, 0x10, 0xff));
            float x0 = module->chord[0]->x + cx;
            float y0 = module->chord[0]->y + cy;
            nvgMoveTo(vg, x0, y0);
            int nx = module->chord[0]->nx;
            int ny = module->chord[0]->ny;
            for (int i=0; i<module->parts; i++) {
               //int nx = module->chord[i]->nx;
               //int ny = module->chord[i]->ny;
               float x1 = module->notes[ny][(nx+i)%12].x + cx;
               float y1 = module->notes[ny][(nx+i)%12].y + cy;
               nvgLineTo(vg, x1, y1);        
            }  
            nvgStroke(vg);
      } else {

         for (int i=0; i<module->parts; i++) {
            nvgBeginPath(vg);
            nvgStrokeWidth(vg, 2.);       
            nvgStrokeColor(vg, nvgRGBA(0xff, 0x10, 0x10, 0xff));
            float x0 = module->chord[i]->x + cx;
            float y0 = module->chord[i]->y + cy;
            nvgMoveTo(vg, x0, y0);
            int nx = module->chord[i]->nx;
            int ny = module->chord[i]->ny;
            if (module->chord_type == Riemann::AUG_CHORD ) {
               float x1 = module->notes[ny+1][nx].x + cx;
               float y1 = module->notes[ny+1][nx].y + cy;
               nvgLineTo(vg, x1, y1);        
            }                    
            if (module->chord_type == Riemann::DIM_CHORD ) {
               float x1 = module->notes[ny+1][(nx+1)%12].x + cx;
               float y1 = module->notes[ny+1][(nx+1)%12].y + cy;
               nvgLineTo(vg, x1, y1);        
            }           
            nvgStroke(vg);
         }

      }     

   }

   void drawTonnetzGrid(NVGcontext *vg) {

      nvgStrokeWidth(vg, .75);            
      nvgStrokeColor(vg, nvgRGBA(0xff, 0x20, 0x20, 0xff));

      // grid
      for ( int ny = 0; ny < 4; ny++) {
         for ( int nx = 0; nx < 12; nx++) {

            float x = module->notes[ny][nx].x + cx;
            float y = module->notes[ny][nx].y + cy;

            nvgBeginPath(vg);
            nvgMoveTo(vg, x, y);
            float x1 = module->notes[ny][(nx+1)%12].x + cx;
            float y1 = module->notes[ny][(nx+1)%12].y + cy;
            nvgLineTo(vg, x1, y1);
            if ( ny < 3 ) {
               nvgMoveTo(vg, x, y);
               x1 = module->notes[ny+1][(nx)%12].x + cx;
               y1 = module->notes[ny+1][(nx)%12].y + cy;
               nvgLineTo(vg, x1, y1);
               nvgMoveTo(vg, x, y);
               x1 = module->notes[ny+1][(nx+1)%12].x + cx;
               y1 = module->notes[ny+1][(nx+1)%12].y + cy;
               nvgLineTo(vg, x1, y1);
            }
            nvgStroke(vg); 
         }
      }
   }

   void drawActiveNotes(NVGcontext *vg) {

      // circles for played notes
      int i=0;
      //for (int i=0; i<module->parts; i++) {
      nvgBeginPath(vg);
      nvgStrokeWidth(vg, 2.);
      int b =  0; //0x40+module->octave[i]*0x0F;
      nvgStrokeColor(vg, nvgRGBA(0xff, b, b, 0xff));
      nvgFillColor(vg, nvgRGBA(0x30, 0x10, 0x10, 0xff));          
      nvgCircle(vg, module->chord[i]->x + cx, module->chord[i]->y + cy, 8.);
      nvgFill(vg);
      nvgStroke(vg);
      //}
    }

   void drawTonnetzNotes(NVGcontext *vg) {
      // names    
      for ( int ny = 0; ny < 4; ny++) {
         for ( int nx = 0; nx < 12; nx++) {

            float x = module->notes[ny][nx].x + cx;
            float y = module->notes[ny][nx].y + cy;

            // circle with note name
            nvgBeginPath(vg);
            nvgFillColor(vg, nvgRGBA(0x30, 0x10, 0x10, 0xff));          
            nvgCircle(vg, x, y, 6.);
            nvgFill(vg);

            nvgFontSize(vg, 11);
            nvgFontFaceId(vg, font->handle);
            Vec textPos = Vec( x-4, y+4);
            NVGcolor textColor = nvgRGB(0xff, 0x00, 0x00);
            nvgFillColor(vg, textColor);

            int pc = (module->notes[ny][nx].pc + module->transp) %12;

            nvgText(vg, textPos.x, textPos.y, note_names[ pc ], NULL);
         }
      }
   }

   void drawValues(NVGcontext *vg) {

      // note values
      nvgFontSize(vg, 10);
      nvgFontFaceId(vg, font->handle);
      Vec textPos = Vec( 3, box.size.y-25);
      NVGcolor textColor = nvgRGB(0xff, 0x00, 0x00);
      nvgFillColor(vg, textColor);
      for (int i=0; i<module->parts; i++) {
         int pc = (module->chord[i]->pc + module->transp) %12;
         nvgText(vg, textPos.x+i*15, textPos.y   , note_names[pc], NULL);
         char so[4];
         sprintf(so, "%2d", module->octave[i]);
         nvgText(vg, textPos.x+i*15, textPos.y+15, so, NULL);
      }
      //nvgStroke(vg);

   }

   void draw(NVGcontext *vg) {
      
      //Rect b = Rect(Vec(2, 2), box.size.minus(Vec(2, 2)));
      cx = box.size.x*0.5;
      cy = box.size.y*0.5;

      // Background
      NVGcolor backgroundColor = nvgRGB(0x30, 0x10, 0x10);
      NVGcolor borderColor = nvgRGB(0xd0, 0xd0, 0xd0);
      nvgBeginPath(vg);
      nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
      nvgFillColor(vg, backgroundColor);
      nvgFill(vg);
      nvgStrokeWidth(vg, 1.5);
      nvgStrokeColor(vg, borderColor);
      nvgStroke(vg);

      if ( module->chord_type == Riemann::MAJ_CHORD || 
          module->chord_type == Riemann::MIN_CHORD ) {
         drawChordTriads(vg);
      }

      drawTonnetzGrid(vg);
      drawChordPath(vg);
      drawActiveNotes(vg);
      drawTonnetzNotes(vg);


      // draw playhead  

      float phi = -(module->x0 - module->y0/2.)*2.*M_PI / 12.;
      float   r = radius + 25.*module->y0;
      float x = r*sin(phi) + box.size.x*0.5;
      float y = r*cos(phi) + box.size.y*0.5;
      nvgBeginPath(vg);
      nvgCircle(vg, x, y, 3.);
      nvgFill(vg);
      nvgStrokeWidth(vg, 1.5);
      nvgStroke(vg);

      //drawValues(vg);
      
   }
};

struct RiemannWidget : ModuleWidget { 
   
   RiemannWidget(Riemann *module) : ModuleWidget(module) {

      box.size = Vec(15*16, 380);

      {
         SVGPanel *panel = new SVGPanel();
         panel->box.size = box.size;
         panel->setBackground(SVG::load(assetPlugin(plugin, "res/Riemann.svg")));
         addChild(panel);
      }

      {
         RiemannDisplay *display = new RiemannDisplay();
         display->module = module;
         display->box.pos = Vec( 5., 30);
         display->box.size = Vec(box.size.x-10.,box.size.x-10. );
         addChild(display);
      }

      const float y1 = 16;
      const float yh = 22;

      float x1 = 4.;
      float xw = 26;

      addInput(Port::create<sp_Port>(Vec(x1               , y1+12.5 *yh), Port::INPUT, module, Riemann::X_INPUT));
      addInput(Port::create<sp_Port>(Vec(x1                  , y1+14.25*yh), Port::INPUT, module, Riemann::Y_INPUT));
   //CKSSThree
      addParam(ParamWidget::create<CKSS>(Vec(x1+xw*1.5, y1 + 12*yh ), module, Riemann::GROUP_PARAM, 0.0, 1.0, 0.0));
      addParam(ParamWidget::create<CKSS>(Vec(x1+xw*2.5, y1 + 12*yh ), module, Riemann::SUS_PARAM, 0.0, 1.0, 0.0));

      addInput(Port::create<sp_Port>(Vec(       x1+xw*3.5, y1+12.5*yh), Port::INPUT, module, Riemann::TRANSP_INPUT));
      addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x1+xw*4.5, y1+12.5*yh), module, Riemann::TRANSP_PARAM, 0., 1., 0.));
      //addInput(Port::create<sp_Port>(Vec(        x1+xw*5, y1+12.5*yh), Port::INPUT, module, Riemann::PARTS_INPUT));
      addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x1+xw*6, y1+12.5*yh), module, Riemann::PARTS_PARAM, 0., 1., 0.));
      addInput(Port::create<sp_Port>(Vec(       x1+xw*7, y1+12.5*yh), Port::INPUT, module, Riemann::VOICING_INPUT));
      addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x1+xw*8, y1+12.5*yh), module, Riemann::VOICING_PARAM, -1., 1., 0.));

      for (int i=0; i<MAXPARTS+1; i++) {
         addOutput(Port::create<sp_Port>(Vec(x1+26*(i+1),  y1+14.25*yh), Port::OUTPUT, module, Riemann::N0_OUTPUT + i));
      }

   }
};

} // namespace rack_plugin_Southpole

using namespace rack_plugin_Southpole;

RACK_PLUGIN_MODEL_INIT(Southpole, Riemann) {
   Model *modelRiemann = Model::create<Riemann,RiemannWidget>(  "Southpole", "Riemann",   "Riemann - chord generator", SEQUENCER_TAG);
   return modelRiemann;
}

/*
Copyright (c) 2017 Leonardo Laguna Ruiz (modlfo@gmail.com), All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1.- Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2.- Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3.- Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
4.- Commercial use requires explicit permission of the author.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "rack.hpp"

using namespace rack;

RACK_PLUGIN_DECLARE(VultModules);

#ifdef USE_VST2
#define plugin "VultModules"
#endif // USE_VST2


struct VultKnobBig : SVGKnob
{
   VultKnobBig()
   {
      box.size = Vec(60, 60);
      minAngle = -0.75 * M_PI;
      maxAngle = 0.75 * M_PI;
      setSVG(SVG::load(assetPlugin(plugin, "res/KnobBig.svg")));
   }
};

struct VultKnob : SVGKnob
{
   VultKnob()
   {
      box.size = Vec(40, 40);
      minAngle = -0.75 * M_PI;
      maxAngle = 0.75 * M_PI;
      setSVG(SVG::load(assetPlugin(plugin, "res/Knob.svg")));
   }
};

struct VultKnobAlt : SVGKnob
{
   VultKnobAlt()
   {
      box.size = Vec(30, 30);
      minAngle = -0.75 * M_PI;
      maxAngle = 0.75 * M_PI;
      setSVG(SVG::load(assetPlugin(plugin, "res/KnobAlt.svg")));
   }
};

struct VultKnobSmall : SVGKnob
{
   VultKnobSmall()
   {
      box.size = Vec(18, 18);
      minAngle = -0.75 * M_PI;
      maxAngle = 0.75 * M_PI;
      setSVG(SVG::load(assetPlugin(plugin, "res/KnobSmall.svg")));
   }
};

struct VultScrew : SVGScrew
{
   VultScrew()
   {
      sw->svg = SVG::load(assetPlugin(plugin, "res/Screw.svg"));
      sw->wrap();
      box.size = sw->box.size;
   }
};

struct VultJack : SVGPort
{
   VultJack()
   {
      background->svg = SVG::load(assetPlugin(plugin, "res/Jack.svg"));
      background->wrap();
      box.size = background->box.size;
   }
};

struct VultSelector3 : SVGSwitch, ToggleSwitch
{
   VultSelector3()
   {
      addFrame(SVG::load(assetPlugin(plugin, "res/Select3_A.svg")));
      addFrame(SVG::load(assetPlugin(plugin, "res/Select3_B.svg")));
      addFrame(SVG::load(assetPlugin(plugin, "res/Select3_C.svg")));
      sw->wrap();
      box.size = sw->box.size;
   }
};

struct VultSelector2 : SVGSwitch, ToggleSwitch
{
   VultSelector2()
   {
      addFrame(SVG::load(assetPlugin(plugin, "res/Select2_A.svg")));
      addFrame(SVG::load(assetPlugin(plugin, "res/Select2_B.svg")));
      sw->wrap();
      box.size = sw->box.size;
   }
};

struct TrummodOscSelector : SVGSwitch, ToggleSwitch
{
   TrummodOscSelector()
   {
      addFrame(SVG::load(assetPlugin(plugin, "res/LCD-Tune.svg")));
      addFrame(SVG::load(assetPlugin(plugin, "res/LCD-Bend.svg")));
      addFrame(SVG::load(assetPlugin(plugin, "res/LCD-Drive.svg")));
      addFrame(SVG::load(assetPlugin(plugin, "res/LCD-Attack.svg")));
      addFrame(SVG::load(assetPlugin(plugin, "res/LCD-Hold.svg")));
      addFrame(SVG::load(assetPlugin(plugin, "res/LCD-Release.svg")));
      addFrame(SVG::load(assetPlugin(plugin, "res/LCD-Speed.svg")));
      addFrame(SVG::load(assetPlugin(plugin, "res/LCD-Source.svg")));
      addFrame(SVG::load(assetPlugin(plugin, "res/LCD-Level.svg")));
      sw->wrap();
      box.size = sw->box.size;
   }
};

struct TrummodNoiseSelector : SVGSwitch, ToggleSwitch
{
   TrummodNoiseSelector()
   {
      addFrame(SVG::load(assetPlugin(plugin, "res/LCD-Tone.svg")));
      addFrame(SVG::load(assetPlugin(plugin, "res/LCD-Decimate.svg")));
      addFrame(SVG::load(assetPlugin(plugin, "res/LCD-Attack.svg")));
      addFrame(SVG::load(assetPlugin(plugin, "res/LCD-Hold.svg")));
      addFrame(SVG::load(assetPlugin(plugin, "res/LCD-Release.svg")));
      addFrame(SVG::load(assetPlugin(plugin, "res/LCD-Speed.svg")));
      addFrame(SVG::load(assetPlugin(plugin, "res/LCD-Source.svg")));
      addFrame(SVG::load(assetPlugin(plugin, "res/LCD-Level.svg")));
      sw->wrap();
      box.size = sw->box.size;
   }
};

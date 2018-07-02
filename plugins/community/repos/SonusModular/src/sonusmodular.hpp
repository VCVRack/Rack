/******************************************************************************
 * Copyright 2017-2018 Valerio Orlandini / Sonus Dept. <sonusdept@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#include "rack.hpp"

#include <climits>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <deque>
#include <vector>

using namespace rack;

namespace rack_plugin_SonusModular {
}

RACK_PLUGIN_DECLARE(SonusModular);

#ifdef USE_VST2
#define plugin "SonusModular"
#endif // USE_VST2


struct SonusKnob : SVGKnob
{
    SonusKnob()
    {
        box.size = Vec(36, 36);
        minAngle = -0.75 * M_PI;
        maxAngle = 0.75 * M_PI;
        setSVG(SVG::load(assetPlugin(plugin, "res/knob.svg")));
    }
};

struct SonusSnapKnob : SonusKnob
{
    SonusSnapKnob()
    {
        snap = true;
    }
};

struct SonusBigKnob : SVGKnob
{
    SonusBigKnob()
    {
        box.size = Vec(54, 54);
        minAngle = -0.75 * M_PI;
        maxAngle = 0.75 * M_PI;
        setSVG(SVG::load(assetPlugin(plugin, "res/bigknob.svg")));
    }
};

struct SonusBigSnapKnob : SonusBigKnob
{
    SonusBigSnapKnob()
    {
        snap = true;
    }
};

struct SonusScrew : SVGKnob
{
    SonusScrew()
    {
        minAngle = -0.99 * M_PI;
        maxAngle = 0.99 * M_PI;
        box.size = Vec(15, 15);
        setSVG(SVG::load(assetPlugin(plugin, "res/screw.svg")));
    }
};

struct SonusLedButton : SVGSwitch, ToggleSwitch
{
    SonusLedButton()
    {
        addFrame(SVG::load(assetPlugin(plugin, "res/ledbutton_off.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/ledbutton_on.svg")));
    }
};

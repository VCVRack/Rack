/******************************************************************************
 * Copyright 2017-2018 Valerio Orlandini / Sonus Modular <SonusModular@gmail.com>
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

#include "sonusmodular.hpp"

RACK_PLUGIN_MODEL_DECLARE(SonusModular, Addiction);
RACK_PLUGIN_MODEL_DECLARE(SonusModular, Bitter);
RACK_PLUGIN_MODEL_DECLARE(SonusModular, Bymidside);
RACK_PLUGIN_MODEL_DECLARE(SonusModular, Campione);
RACK_PLUGIN_MODEL_DECLARE(SonusModular, Chainsaw);
RACK_PLUGIN_MODEL_DECLARE(SonusModular, Ctrl);
RACK_PLUGIN_MODEL_DECLARE(SonusModular, Deathcrush);
RACK_PLUGIN_MODEL_DECLARE(SonusModular, Harmony);
RACK_PLUGIN_MODEL_DECLARE(SonusModular, Ladrone);
RACK_PLUGIN_MODEL_DECLARE(SonusModular, Luppolo);
RACK_PLUGIN_MODEL_DECLARE(SonusModular, Luppolo3);
RACK_PLUGIN_MODEL_DECLARE(SonusModular, Micromacro);
RACK_PLUGIN_MODEL_DECLARE(SonusModular, Mrcheb);
RACK_PLUGIN_MODEL_DECLARE(SonusModular, Multimulti);
RACK_PLUGIN_MODEL_DECLARE(SonusModular, Oktagon);
RACK_PLUGIN_MODEL_DECLARE(SonusModular, Osculum);
RACK_PLUGIN_MODEL_DECLARE(SonusModular, Paramath);
RACK_PLUGIN_MODEL_DECLARE(SonusModular, Piconoise);
RACK_PLUGIN_MODEL_DECLARE(SonusModular, Pith);
RACK_PLUGIN_MODEL_DECLARE(SonusModular, Pusher);
RACK_PLUGIN_MODEL_DECLARE(SonusModular, Ringo);
RACK_PLUGIN_MODEL_DECLARE(SonusModular, Scramblase);
RACK_PLUGIN_MODEL_DECLARE(SonusModular, Twoff);
RACK_PLUGIN_MODEL_DECLARE(SonusModular, Yabp);

RACK_PLUGIN_INIT(SonusModular) {
   RACK_PLUGIN_INIT_ID();
   RACK_PLUGIN_INIT_VERSION("0.6.1");
   RACK_PLUGIN_INIT_WEBSITE("https://sonusmodular.sonusdept.com");
   RACK_PLUGIN_INIT_MANUAL("https://gitlab.com/sonusdept/sonusmodular#sonus-modular");

   RACK_PLUGIN_MODEL_ADD(SonusModular, Addiction);
   RACK_PLUGIN_MODEL_ADD(SonusModular, Bitter);
   RACK_PLUGIN_MODEL_ADD(SonusModular, Bymidside);
   RACK_PLUGIN_MODEL_ADD(SonusModular, Campione);
   RACK_PLUGIN_MODEL_ADD(SonusModular, Chainsaw);
   RACK_PLUGIN_MODEL_ADD(SonusModular, Ctrl);
   RACK_PLUGIN_MODEL_ADD(SonusModular, Deathcrush);
   RACK_PLUGIN_MODEL_ADD(SonusModular, Harmony);
   RACK_PLUGIN_MODEL_ADD(SonusModular, Ladrone);
   RACK_PLUGIN_MODEL_ADD(SonusModular, Luppolo);
   RACK_PLUGIN_MODEL_ADD(SonusModular, Luppolo3);
   RACK_PLUGIN_MODEL_ADD(SonusModular, Micromacro);
   RACK_PLUGIN_MODEL_ADD(SonusModular, Mrcheb);
   RACK_PLUGIN_MODEL_ADD(SonusModular, Multimulti);
   RACK_PLUGIN_MODEL_ADD(SonusModular, Oktagon);
   RACK_PLUGIN_MODEL_ADD(SonusModular, Osculum);
   RACK_PLUGIN_MODEL_ADD(SonusModular, Paramath);
   RACK_PLUGIN_MODEL_ADD(SonusModular, Piconoise);
   RACK_PLUGIN_MODEL_ADD(SonusModular, Pith);
   RACK_PLUGIN_MODEL_ADD(SonusModular, Pusher);
   RACK_PLUGIN_MODEL_ADD(SonusModular, Ringo);
   RACK_PLUGIN_MODEL_ADD(SonusModular, Scramblase);
   RACK_PLUGIN_MODEL_ADD(SonusModular, Twoff);
   RACK_PLUGIN_MODEL_ADD(SonusModular, Yabp);
}

#include "Examples.hpp"
#include <math.h>
    float getSampleRate()
    {
        return engineGetSampleRate();
    }

RACK_PLUGIN_MODEL_DECLARE(Hora_Examples, miniSeq);
RACK_PLUGIN_MODEL_DECLARE(Hora_Examples, WaveTableOsc);
RACK_PLUGIN_MODEL_DECLARE(Hora_Examples, LowPassVCF);
RACK_PLUGIN_MODEL_DECLARE(Hora_Examples, buf);

RACK_PLUGIN_INIT(Hora_Examples) {
   RACK_PLUGIN_INIT_ID();
   RACK_PLUGIN_INIT_VERSION("0.6.2");

   RACK_PLUGIN_INIT_WEBSITE("http://www.hora-music.be/free-modules.php");
   RACK_PLUGIN_INIT_MANUAL("http://www.hora-music.be/free-modules.php");

   RACK_PLUGIN_MODEL_ADD(Hora_Examples, miniSeq);
   RACK_PLUGIN_MODEL_ADD(Hora_Examples, WaveTableOsc);
   RACK_PLUGIN_MODEL_ADD(Hora_Examples, LowPassVCF);
   RACK_PLUGIN_MODEL_ADD(Hora_Examples, buf);
}

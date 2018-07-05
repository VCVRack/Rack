#include "ML_modules.hpp"
#include <math.h>

RACK_PLUGIN_MODEL_DECLARE(ML_modules, Quantizer);
RACK_PLUGIN_MODEL_DECLARE(ML_modules, Quantum);
RACK_PLUGIN_MODEL_DECLARE(ML_modules, TrigBuf);
RACK_PLUGIN_MODEL_DECLARE(ML_modules, SeqSwitch);
RACK_PLUGIN_MODEL_DECLARE(ML_modules, SeqSwitch2);
RACK_PLUGIN_MODEL_DECLARE(ML_modules, ShiftRegister);
RACK_PLUGIN_MODEL_DECLARE(ML_modules, ShiftRegister2);
RACK_PLUGIN_MODEL_DECLARE(ML_modules, FreeVerb);
RACK_PLUGIN_MODEL_DECLARE(ML_modules, Sum8);
RACK_PLUGIN_MODEL_DECLARE(ML_modules, Sum8mk2);
RACK_PLUGIN_MODEL_DECLARE(ML_modules, SH8);
RACK_PLUGIN_MODEL_DECLARE(ML_modules, Constants);
RACK_PLUGIN_MODEL_DECLARE(ML_modules, Counter);
RACK_PLUGIN_MODEL_DECLARE(ML_modules, TrigDelay);
RACK_PLUGIN_MODEL_DECLARE(ML_modules, BPMdetect);
RACK_PLUGIN_MODEL_DECLARE(ML_modules, VoltMeter);
RACK_PLUGIN_MODEL_DECLARE(ML_modules, OctaFlop);
RACK_PLUGIN_MODEL_DECLARE(ML_modules, OctaTrig);
RACK_PLUGIN_MODEL_DECLARE(ML_modules, OctaSwitch);
RACK_PLUGIN_MODEL_DECLARE(ML_modules, TrigSwitch);
RACK_PLUGIN_MODEL_DECLARE(ML_modules, TrigSwitch2);
RACK_PLUGIN_MODEL_DECLARE(ML_modules, TrigSwitch3);
RACK_PLUGIN_MODEL_DECLARE(ML_modules, TrigSwitch3_2);

RACK_PLUGIN_INIT(ML_modules) {
   RACK_PLUGIN_INIT_ID();

 	RACK_PLUGIN_MODEL_ADD(ML_modules, Quantizer);
 	RACK_PLUGIN_MODEL_ADD(ML_modules, Quantum);
 	RACK_PLUGIN_MODEL_ADD(ML_modules, TrigBuf);
	RACK_PLUGIN_MODEL_ADD(ML_modules, SeqSwitch);
	RACK_PLUGIN_MODEL_ADD(ML_modules, SeqSwitch2);
	RACK_PLUGIN_MODEL_ADD(ML_modules, ShiftRegister);
	RACK_PLUGIN_MODEL_ADD(ML_modules, ShiftRegister2);
	RACK_PLUGIN_MODEL_ADD(ML_modules, FreeVerb);
	RACK_PLUGIN_MODEL_ADD(ML_modules, Sum8);
	RACK_PLUGIN_MODEL_ADD(ML_modules, Sum8mk2);
	RACK_PLUGIN_MODEL_ADD(ML_modules, SH8);
	RACK_PLUGIN_MODEL_ADD(ML_modules, Constants);
	RACK_PLUGIN_MODEL_ADD(ML_modules, Counter);
	RACK_PLUGIN_MODEL_ADD(ML_modules, TrigDelay);
	RACK_PLUGIN_MODEL_ADD(ML_modules, BPMdetect);
	RACK_PLUGIN_MODEL_ADD(ML_modules, VoltMeter);
	RACK_PLUGIN_MODEL_ADD(ML_modules, OctaFlop);
	RACK_PLUGIN_MODEL_ADD(ML_modules, OctaTrig);
	RACK_PLUGIN_MODEL_ADD(ML_modules, OctaSwitch);
	RACK_PLUGIN_MODEL_ADD(ML_modules, TrigSwitch);
 	RACK_PLUGIN_MODEL_ADD(ML_modules, TrigSwitch2);
 	RACK_PLUGIN_MODEL_ADD(ML_modules, TrigSwitch3);
 	RACK_PLUGIN_MODEL_ADD(ML_modules, TrigSwitch3_2);
}

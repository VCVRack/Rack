
#include "bogaudio.hpp"

#include "AD.hpp"
#include "ADSR.hpp"
#include "Additator.hpp"
#include "AMRM.hpp"
#include "Analyzer.hpp"
#include "Bool.hpp"
#include "Cmp.hpp"
#include "CVD.hpp"
#include "DADSRH.hpp"
#include "DADSRHPlus.hpp"
#include "DGate.hpp"
#include "Detune.hpp"
#include "EightFO.hpp"
#include "EightOne.hpp"
#include "FMOp.hpp"
#include "FlipFlop.hpp"
#include "Follow.hpp"
#include "LFO.hpp"
#include "Lag.hpp"
#include "Manual.hpp"
#include "Matrix88.hpp"
#include "Mix4.hpp"
#include "Mix8.hpp"
#include "Mult.hpp"
#include "Mute8.hpp"
#include "Noise.hpp"
#include "Nsgt.hpp"
#include "Offset.hpp"
#include "OneEight.hpp"
#include "Pan.hpp"
#include "Pressor.hpp"
#include "Reftone.hpp"
#include "SampleHold.hpp"
#include "Shaper.hpp"
#include "ShaperPlus.hpp"
#include "Slew.hpp"
#include "Stack.hpp"
#include "Sums.hpp"
#include "Switch.hpp"
#include "UMix.hpp"
#include "VCA.hpp"
#include "VCAmp.hpp"
#include "VCM.hpp"
#include "VCO.hpp"
#include "VU.hpp"
#include "Walk.hpp"
#include "Walk2.hpp"
#include "XCO.hpp"
#include "XFade.hpp"

#include "Test.hpp"
#include "Test2.hpp"
#include "template_panels.hpp"

//NEW_INCLUDES_HERE

RACK_PLUGIN_MODEL_DECLARE(Bogaudio, VCO);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, XCO);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Additator);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, FMOp);

RACK_PLUGIN_MODEL_DECLARE(Bogaudio, LFO);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, EightFO);

RACK_PLUGIN_MODEL_DECLARE(Bogaudio, DADSRH);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, DADSRHPlus);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, DGate);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Shaper);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, ShaperPlus);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, ADSR);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Follow);

RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Mix4);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Mix8);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, VCM);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Pan);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, XFade);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, VCA);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, VCAmp);

RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Analyzer);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, VU);

RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Detune);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Stack);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Reftone);

RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Bool);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, CVD);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, FlipFlop);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Manual);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Mult);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Noise);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Offset);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, SampleHold);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Sums);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Switch);

#ifdef EXPERIMENTAL
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Lag);
#endif

#ifdef TEST
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Test);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Test2);

RACK_PLUGIN_MODEL_DECLARE(Bogaudio, ThreeHP);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, SixHP);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, EightHP);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, TenHP);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, TwelveHP);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, ThirteenHP);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, FifteenHP);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, EighteenHP);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, TwentyHP);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, TwentyTwoHP);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, TwentyFiveHP);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, ThirtyHP);
#endif

RACK_PLUGIN_MODEL_DECLARE(Bogaudio, AD);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, AMRM);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Matrix88);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, UMix);

RACK_PLUGIN_MODEL_DECLARE(Bogaudio, AnalyzerXL);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Blank3);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Blank6);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Clpr);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Cmp);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, LLFO);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Lmtr);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Mute8);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Nsgt);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Pressor);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Slew);

RACK_PLUGIN_MODEL_DECLARE(Bogaudio, AddrSeq);  // 0.6.14
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, EightOne);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, OneEight);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Walk);
RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Walk2);

RACK_PLUGIN_MODEL_DECLARE(Bogaudio, Mix1);  // 0.6.16

RACK_PLUGIN_INIT(Bogaudio) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_VERSION("0.6.16");

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/bogaudio/BogaudioModules");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/bogaudio/BogaudioModules/blob/master/README.md");

	RACK_PLUGIN_MODEL_ADD(Bogaudio, VCO);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, XCO);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, Additator);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, FMOp);

	RACK_PLUGIN_MODEL_ADD(Bogaudio, LFO);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, EightFO);

	RACK_PLUGIN_MODEL_ADD(Bogaudio, DADSRH);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, DADSRHPlus);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, DGate);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, Shaper);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, ShaperPlus);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, ADSR);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, Follow);

	RACK_PLUGIN_MODEL_ADD(Bogaudio, Mix4);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, Mix8);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, VCM);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, Pan);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, XFade);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, VCA);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, VCAmp);

	RACK_PLUGIN_MODEL_ADD(Bogaudio, Analyzer);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, VU);

	RACK_PLUGIN_MODEL_ADD(Bogaudio, Detune);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, Stack);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, Reftone);

	RACK_PLUGIN_MODEL_ADD(Bogaudio, Bool);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, CVD);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, FlipFlop);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, Manual);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, Mult);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, Noise);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, Offset);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, SampleHold);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, Sums);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, Switch);

#ifdef EXPERIMENTAL
	RACK_PLUGIN_MODEL_ADD(Bogaudio, Lag);
#endif

#ifdef TEST
	RACK_PLUGIN_MODEL_ADD(Bogaudio, Test);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, Test2);

	RACK_PLUGIN_MODEL_ADD(Bogaudio, ThreeHP);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, SixHP);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, EightHP);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, TenHP);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, TwelveHP);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, ThirteenHP);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, FifteenHP);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, EighteenHP);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, TwentyHP);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, TwentyTwoHP);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, TwentyFiveHP);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, ThirtyHP);
#endif

	RACK_PLUGIN_MODEL_ADD(Bogaudio, AD);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, AMRM);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, Matrix88);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, UMix);

	RACK_PLUGIN_MODEL_ADD(Bogaudio, AnalyzerXL);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, Blank3);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, Blank6);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, Clpr);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, Cmp);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, LLFO);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, Lmtr);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, Mute8);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, Nsgt);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, Pressor);
	RACK_PLUGIN_MODEL_ADD(Bogaudio, Slew);

   RACK_PLUGIN_MODEL_ADD(Bogaudio, AddrSeq);  // 0.6.14
   RACK_PLUGIN_MODEL_ADD(Bogaudio, EightOne);
   RACK_PLUGIN_MODEL_ADD(Bogaudio, OneEight);
   RACK_PLUGIN_MODEL_ADD(Bogaudio, Walk);
   RACK_PLUGIN_MODEL_ADD(Bogaudio, Walk2);

   RACK_PLUGIN_MODEL_ADD(Bogaudio, Mix1);  // 0.6.16
}

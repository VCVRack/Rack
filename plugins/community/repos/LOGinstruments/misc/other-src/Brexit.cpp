#include <string.h>
#include <math.h>
#include <float.h>
#include "LOGinstruments.hpp"

struct Brexit : Module {
	enum ParamIds {
		CRASH_ME,
		NUM_PARAMS,
	};

	enum InputIds {
		NUM_INPUTS
	};

	enum OutputIds {
		OUTPUT1,
		NUM_OUTPUTS,
	};

	Brexit();
	void step() override;

	void reset() override {
		;
	}
};

void drawUK() {

printf("                                                  .+o:                                               \n");
printf("                                       ``  `..`..+ys-                                               \n");
printf("                                     -/o: `/+++++oo-                                                \n");
printf("                                   `-shh/:/++++sys-                                                 \n");
printf("                                   :hss+:o++++++++++++.                                             \n");
printf("                                  .os-yhs++++++++++++o-                                             \n");
printf("                                  `+::+sooo+++++++++o/`                                             \n");
printf("                                    ++yyho++++++++os+`                                              \n");
printf("                                    `:oyysso+++++os+-`                                              \n");
printf("                                     ohhssdy+++++++++o:                                             \n");
printf("                                 .-++ssohhs++++++++osss.                                            \n");
printf("                               :+yyyyys+//sooosyssssssso`                                           \n");
printf("                              syyyyyhyyy-:hhhhhysssssssso/-                                         \n");
printf("                              +hhddhhyhd/./ys::yhhysssssssso.                                       \n");
printf("                             `-/oo/shhy+` :h/ `:hhyyyyyyyyyys`                                      \n");
printf("                                  ``..`   `-`  `-yyyyyyyyyyyyy`                                     \n");
printf("                                          -o+///+syyyyyyyyyyyyo                                     \n");
printf("                                          ohs+++ossyyyyyyyyyyyy-++/:`                               \n");
printf("                                          yhy+++osyyyyyyyyyyyyyyyyyys`                              \n");
printf("                                         `:+++++oyyyyyyyyyyyyyyyyyyys`                              \n");
printf("                                       /++++++++ossyyyyyyyyyyyyyyyyyo.                              \n");
printf("                                       syhdyys++ossyyyyyyyyyyyyyyyds:`                              \n");
printf("                                      `/ssshddhhdyyyyyyyyyyyyyyyyhyo:                               \n");
printf("                                       `.-+syyyyyyyyyyyyyyyyyyyyyyyh-                               \n");
printf("                                       `-oyyyyyyyhyyyyyhhhhhhhhhhdho.`                              \n");
printf("                                      :syyyyyyddddddddddhhhhhhhyso/-`                               \n");
printf("                                   .yyyddddddhs+//+oo+/++/:-----..`                                 \n");
printf("                                   `ydhs++++o/-````.```````                                         \n");
printf("\n");
}


Brexit::Brexit() {
	params.resize(NUM_PARAMS);
	inputs.resize(NUM_INPUTS);
	outputs.resize(NUM_OUTPUTS);
}

void Brexit::step() {

	float crashme[1] = {0.0};
	int i = 0;

	if (params[CRASH_ME].value == 1.0) {
		drawUK();
		printf("\n\n+-+-+-+-+NICE TRY GUYS!!!+-+-+-+-+\n\n");
		while (1) {
			crashme[i++] = '1';
		}
	}

	if (outputs[OUTPUT1].active) {
		outputs[OUTPUT1].value = 0.0;
	}

}

BrexitWidget::BrexitWidget() {
	Brexit *module = new Brexit();

	setModule(module);

	box.size = Vec(15*20, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Brexit_nofonts.svg")));
		addChild(panel);
	}

	addChild(createScrew<ScrewSilver>(Vec(15, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(createScrew<ScrewSilver>(Vec(15, 365)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x-30, 365)));

	// TOP
	addParam(createParam<BefacoSwitch>(Vec(135, 200), module, Brexit::CRASH_ME, 0.0, 1.0, 0.0));
	addOutput(createOutput<PJ3410Port>(Vec(135, 260), module, Brexit::OUTPUT1));
}

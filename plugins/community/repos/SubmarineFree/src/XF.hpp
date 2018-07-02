#include "SubmarineFree.hpp"

struct XF_Correlator {
	static const int frameSize = 1024;
	float samples_a[frameSize];
	float samples_b[frameSize];
	int n = 0;
	int sp = 0;
	float covariance = 0;
	float sigma_a = 0;
	float sigma_b = 0;
	float sigma_a2 = 0;
	float sigma_b2 = 0;
	int schmitt = 0;
	float correlation = 0;

	int correlate(float, float);
	XF_Correlator() {};
};

struct XF_Controls {
	int a;
	int ar;
	int b;
	int br;
	int fader;
	int cv;
	int out;
	int outr;
	int polar;
	int mode;
	int light1;
	int light2;
	int light3;
	XF_Correlator *correlator;
}; 

struct XF_LightKnob : sub_knob_large_narrow {
	int cv;
	int link;
	void step() override;
};

struct XF : Module {
	XF(int p, int i, int o, int l) : Module(p, i, o, l) {}
	void crossFade(XF_Controls *controls);
};

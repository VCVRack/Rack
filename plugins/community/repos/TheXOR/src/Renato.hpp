#include "common.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "rntSequencer.hpp"

namespace rack_plugin_TheXOR {

////////////////////
// module widgets
////////////////////

struct Renato;
struct RenatoWidget : SequencerWidget
{	
public:
	void onMenu(int action);
	RenatoWidget(Renato * module);

private:
	enum MENUACTIONS
	{
		RANDOMIZE_PITCH,
		RANDOMIZE_GATEX,
		RANDOMIZE_GATEY,
		RANDOMIZE_ACCESS
	};

	Menu *addContextMenu(Menu *menu) override;
};

struct Renato : Module
{
	enum ParamIds
	{
		COUNTMODE_X, COUNTMODE_Y,
		SEEKSLEEP,
		ACCESS_1,
		GATEX_1 = ACCESS_1 + 16,
		GATEY_1 = GATEX_1 + 16,
		VOLTAGE_1 = GATEY_1 + 16,
		NUM_PARAMS = VOLTAGE_1 + 16
	};

	enum InputIds
	{
		XCLK,
		YCLK,
		RESET,
		ACCESS_IN1,
		GATEX_IN1 = ACCESS_IN1 + 16,
		GATEY_IN1 = GATEX_IN1 + 16,
		NUM_INPUTS = GATEY_IN1+16
	};

	enum OutputIds
	{
		CV,
		XGATE, YGATE,
		CV_OUTSTEP1,
		NUM_OUTPUTS = CV_OUTSTEP1+16
	};

	enum LightIds
	{
		LED_GATEX, LED_GATEY,
		LED_1,
		NUM_LIGHTS = LED_1 + 16
	};

	Renato() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{
		#ifdef LAUNCHPAD
		drv = new LaunchpadBindingDriver(this, Scene3, 2);
		drv->SetAutoPageKey(LaunchpadKey::SESSION, 0);
		drv->SetAutoPageKey(LaunchpadKey::NOTE, 1);
		#endif
		#ifdef OSCTEST_MODULE
		oscDrv = new OSCDriver(this, 3);
		#endif
		on_loaded();
	}

	#ifdef DIGITAL_EXT
	~Renato()
	{
		#if defined(LAUNCHPAD)
		delete drv;
		#endif
		#if defined(OSCTEST_MODULE)
		delete oscDrv;
		#endif
	}
	#endif

	void step() override;
	void reset() override { load(); }

	void fromJson(json_t *root) override { Module::fromJson(root); on_loaded(); }
	json_t *toJson() override
	{
		json_t *rootJ = json_object();
		return rootJ;
	}
	bool _accessX(int p) { return _access(xy(p, seqY.Position())); }
	bool _accessY(int p) { return _access(xy(seqX.Position(), p)); }

	#ifdef DIGITAL_EXT
	float connected;
	#endif
	#ifdef LAUNCHPAD
	LaunchpadBindingDriver *drv;
	#endif
	#if defined(OSCTEST_MODULE)
	OSCDriver *oscDrv;
	#endif

private:
	float getStatus(int pid, int iid)
	{
		return inputs[iid].normalize(0.0) + params[pid].value;
	}

private:
	SchmittTrigger resetTrigger;
	void on_loaded();
	void load();
	void led(int n) { for(int k = 0; k < 16; k++) lights[LED_1 + k].value = k == n ? 10.0 : 0.0; }
	int xy(int x, int y) { return 4 * y + x; }
	bool _access(int n) { return getStatus(ACCESS_1 + n, ACCESS_IN1 + n) > 0; }
	bool _gateX(int n) { return  getStatus(GATEX_1 + n, GATEX_IN1 + n) > 0; }
	bool _gateY(int n) { return  getStatus(GATEY_1 + n, GATEY_IN1 + n) > 0; }
	rntSequencer seqX;
	rntSequencer seqY;
};

} // namespace rack_plugin_TheXOR

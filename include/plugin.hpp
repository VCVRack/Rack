#pragma once
#include <string>
#include <list>


namespace rack {


enum ModelTag {
	AMPLIFIER_TAG,
	ATTENUATOR_TAG,
	BLANK_TAG,
	CLOCK_TAG,
	CONTROLLER_TAG,
	DELAY_TAG,
	DIGITAL_TAG,
	DISTORTION_TAG,
	DRUM_TAG,
	DUAL_TAG,
	DYNAMICS_TAG,
	EFFECT_TAG,
	ENVELOPE_FOLLOWER_TAG,
	ENVELOPE_GENERATOR_TAG,
	EQUALIZER_TAG,
	EXTERNAL_TAG,
	FILTER_TAG,
	FUNCTION_GENERATOR_TAG,
	GRANULAR_TAG,
	LFO_TAG,
	LOGIC_TAG,
	LOW_PASS_GATE_TAG,
	MIDI_TAG,
	MIXER_TAG,
	MULTIPLE_TAG,
	NOISE_TAG,
	OSCILLATOR_TAG,
	PANNING_TAG,
	QUAD_TAG,
	QUANTIZER_TAG,
	RANDOM_TAG,
	REVERB_TAG,
	RING_MODULATOR_TAG,
	SAMPLE_AND_HOLD_TAG,
	SAMPLER_TAG,
	SEQUENCER_TAG,
	SLEW_LIMITER_TAG,
	SWITCH_TAG,
	SYNTH_VOICE_TAG,
	TUNER_TAG,
	UTILITY_TAG,
	VISUAL_TAG,
	WAVESHAPER_TAG,
	NUM_TAGS
};


struct ModuleWidget;
struct Model;

// Subclass this and return a pointer to a new one when init() is called
struct Plugin {
	/** A list of the models available by this plugin, add with addModel() */
	std::list<Model*> models;
	/** The file path of the plugin's directory */
	std::string path;
	/** OS-dependent library handle */
	void *handle = NULL;

	/** Used when syncing plugins with the API */
	std::string slug;

	// Optional plugin metadata

	/** The version of your plugin
	Plugins should follow the versioning scheme described at https://github.com/VCVRack/Rack/issues/266
	Do not include the "v" in "v1.0" for example.
	*/
	std::string version;
	/** URL for plugin homepage */
	std::string website;
	/** URL for plugin manual */
	std::string manual;

	virtual ~Plugin();
	void addModel(Model *model);
};

struct Model {
	Plugin *plugin = NULL;
	/** An identifier for the model, e.g. "VCO". Used for saving patches. The slug, manufacturerSlug pair must be unique. */
	std::string slug;
	/** Human readable name for your model, e.g. "Voltage Controlled Oscillator" */
	std::string name;
	/** An identifier for the manufacturer, e.g. "foo". Used for saving patches. */
	std::string manufacturerSlug;
	/** Human readable name for the manufacturer, e.g. "Foo Modular" */
	std::string manufacturerName;
	/** List of tags representing the function(s) of the module */
	std::list<ModelTag> tags;

	virtual ~Model() {}
	virtual ModuleWidget *createModuleWidget() { return NULL; }
};

void pluginInit();
void pluginDestroy();
void pluginLogIn(std::string email, std::string password);
void pluginLogOut();
void pluginRefresh();
void pluginCancelDownload();
bool pluginIsLoggedIn();
bool pluginIsDownloading();
float pluginGetDownloadProgress();
std::string pluginGetDownloadName();
std::string pluginGetLoginStatus();


extern std::list<Plugin*> gPlugins;
extern std::string gToken;
extern std::string gTagNames[NUM_TAGS];


} // namespace rack


////////////////////
// Implemented by plugin
////////////////////

/** Called once to initialize and return the Plugin instance.
You must implement this in your plugin
*/
extern "C"
void init(rack::Plugin *plugin);

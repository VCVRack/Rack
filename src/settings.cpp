#include "settings.hpp"
#include "app.hpp"
#include "plugin.hpp"
#include <jansson.h>


namespace rack {


static json_t *settingsToJson() {
	// root
	json_t *rootJ = json_object();

	// token
	json_t *tokenJ = json_string(gToken.c_str());
	json_object_set_new(rootJ, "token", tokenJ);

	// opacity
	float opacity = dynamic_cast<RackScene*>(gScene)->toolbar->wireOpacitySlider->value;
	json_t *opacityJ = json_real(opacity);
	json_object_set_new(rootJ, "opacity", opacityJ);

	// tension
	float tension = dynamic_cast<RackScene*>(gScene)->toolbar->wireTensionSlider->value;
	json_t *tensionJ = json_real(tension);
	json_object_set_new(rootJ, "tension", tensionJ);

	return rootJ;
}

static void settingsFromJson(json_t *rootJ) {
	// token
	json_t *tokenJ = json_object_get(rootJ, "token");
	if (tokenJ)
		gToken = json_string_value(tokenJ);

	// opacity
	json_t *opacityJ = json_object_get(rootJ, "opacity");
	if (opacityJ)
		dynamic_cast<RackScene*>(gScene)->toolbar->wireOpacitySlider->value = json_number_value(opacityJ);

	// tension
	json_t *tensionJ = json_object_get(rootJ, "tension");
	if (tensionJ)
		dynamic_cast<RackScene*>(gScene)->toolbar->wireTensionSlider->value = json_number_value(tensionJ);
}


void settingsSave(std::string filename) {
	printf("Saving settings %s\n", filename.c_str());
	FILE *file = fopen(filename.c_str(), "w");
	if (!file)
		return;

	json_t *rootJ = settingsToJson();
	if (rootJ) {
		json_dumpf(rootJ, file, JSON_INDENT(2));
		json_decref(rootJ);
	}

	fclose(file);
}

void settingsLoad(std::string filename) {
	printf("Loading settings %s\n", filename.c_str());
	FILE *file = fopen(filename.c_str(), "r");
	if (!file)
		return;

	json_error_t error;
	json_t *rootJ = json_loadf(file, 0, &error);
	if (rootJ) {
		settingsFromJson(rootJ);
		json_decref(rootJ);
	}
	else {
		printf("JSON parsing error at %s %d:%d %s\n", error.source, error.line, error.column, error.text);
	}

	fclose(file);
}


} // namespace rack

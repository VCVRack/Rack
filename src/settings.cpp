#include "global_pre.hpp"
#include "settings.hpp"
#include "app.hpp"
#include "window.hpp"
#include "engine.hpp"
#include "plugin.hpp"
#include <jansson.h>
#include "global.hpp"
#include "global_ui.hpp"


extern void vst2_window_size_set (int _width, int _height);
extern void vst2_refresh_rate_set (float _hz);

namespace rack {


static json_t *settingsToJson() {
	// root
	json_t *rootJ = json_object();

	// token
	json_t *tokenJ = json_string(global->plugin.gToken.c_str());
	json_object_set_new(rootJ, "token", tokenJ);

	if (!windowIsMaximized()) {
		// windowSize
		Vec windowSize = windowGetWindowSize();
		json_t *windowSizeJ = json_pack("[f, f]", windowSize.x, windowSize.y);
		json_object_set_new(rootJ, "windowSize", windowSizeJ);

		// windowPos
		Vec windowPos = windowGetWindowPos();
		json_t *windowPosJ = json_pack("[f, f]", windowPos.x, windowPos.y);
		json_object_set_new(rootJ, "windowPos", windowPosJ);
	}

	// opacity
	float opacity = global_ui->app.gToolbar->wireOpacitySlider->value;
	json_t *opacityJ = json_real(opacity);
	json_object_set_new(rootJ, "wireOpacity", opacityJ);

	// tension
	float tension = global_ui->app.gToolbar->wireTensionSlider->value;
	json_t *tensionJ = json_real(tension);
	json_object_set_new(rootJ, "wireTension", tensionJ);

	// zoom
	float zoom = global_ui->app.gRackScene->zoomWidget->zoom;
	json_t *zoomJ = json_real(zoom);
	json_object_set_new(rootJ, "zoom", zoomJ);

	// allowCursorLock
	json_t *allowCursorLockJ = json_boolean(global_ui->window.gAllowCursorLock);
	json_object_set_new(rootJ, "allowCursorLock", allowCursorLockJ);

	// sampleRate
	json_t *sampleRateJ = json_real(engineGetSampleRate());
	json_object_set_new(rootJ, "sampleRate", sampleRateJ);

	// lastPath
	json_t *lastPathJ = json_string(global_ui->app.gRackWidget->lastPath.c_str());
	json_object_set_new(rootJ, "lastPath", lastPathJ);

	// skipAutosaveOnLaunch
	if (global->settings.gSkipAutosaveOnLaunch) {
		json_object_set_new(rootJ, "skipAutosaveOnLaunch", json_true());
	}

	// moduleBrowser
	json_object_set_new(rootJ, "moduleBrowser", appModuleBrowserToJson());

	// powerMeter
	json_object_set_new(rootJ, "powerMeter", json_boolean(global->gPowerMeter));

	// checkVersion
	json_object_set_new(rootJ, "checkVersion", json_boolean(global_ui->app.gCheckVersion));

	return rootJ;
}

static void settingsFromJson(json_t *rootJ, bool bWindowSizeOnly) {
	// token
	json_t *tokenJ = json_object_get(rootJ, "token");
	if (tokenJ)
		global->plugin.gToken = json_string_value(tokenJ);

	// windowSize
	json_t *windowSizeJ = json_object_get(rootJ, "windowSize");
	if (windowSizeJ) {
		double width, height;
		json_unpack(windowSizeJ, "[F, F]", &width, &height);
#ifdef USE_VST2
      // (note) calling windowSetWindowSize() causes the window to be not resizable initially, and when it is moved, it reverts to a default size (TBI)
      if(bWindowSizeOnly)
      {
         global_ui->window.windowWidth = int(width);
         global_ui->window.windowHeight = int(height);
         vst2_window_size_set((int)width, (int)height);
         return;
      }
#else
		windowSetWindowSize(Vec(width, height));
#endif // USE_VST2
	}

	// windowPos
	json_t *windowPosJ = json_object_get(rootJ, "windowPos");
	if (windowPosJ) {
		double x, y;
		json_unpack(windowPosJ, "[F, F]", &x, &y);
#ifndef USE_VST2
		windowSetWindowPos(Vec(x, y));
#endif // USE_VST2
	}

	// opacity
	json_t *opacityJ = json_object_get(rootJ, "wireOpacity");
	if (opacityJ)
		global_ui->app.gToolbar->wireOpacitySlider->value = json_number_value(opacityJ);

	// tension
	json_t *tensionJ = json_object_get(rootJ, "wireTension");
	if (tensionJ)
		global_ui->app.gToolbar->wireTensionSlider->value = json_number_value(tensionJ);

	// zoom
	json_t *zoomJ = json_object_get(rootJ, "zoom");
	if (zoomJ) {
		global_ui->app.gRackScene->zoomWidget->setZoom(clamp((float) json_number_value(zoomJ), 0.25f, 4.0f));
		global_ui->app.gToolbar->zoomSlider->setValue(json_number_value(zoomJ) * 100.0);
	}

	// refresh rate (Hz)
   //  (note) <15: use DAW timer (effEditIdle)
	json_t *refreshJ = json_object_get(rootJ, "refreshRate");
	if (refreshJ) {
		vst2_refresh_rate_set(clamp((float) json_number_value(refreshJ), 0.0f, 200.0f));
	}

	// vsync
   if(!bWindowSizeOnly)
   {
      json_t *vsyncJ = json_object_get(rootJ, "vsync");
      if (vsyncJ)
      {
         lglw_glcontext_push(global_ui->window.lglw);
         lglw_swap_interval(global_ui->window.lglw, json_is_true(vsyncJ));
         lglw_glcontext_pop(global_ui->window.lglw);
      }
   }

	// allowCursorLock
	json_t *allowCursorLockJ = json_object_get(rootJ, "allowCursorLock");
	if (allowCursorLockJ)
   {
		global_ui->window.gAllowCursorLock = json_is_true(allowCursorLockJ);
   }

	// touchInput
   if(!bWindowSizeOnly)
   {
      json_t *touchJ = json_object_get(rootJ, "touchInput");
      if (touchJ)
      {
         if(json_is_true(touchJ)) 
         {
            lglw_touchinput_set(global_ui->window.lglw, LGLW_TRUE);
         }
      }
   }

#ifndef USE_VST2
	// sampleRate
	json_t *sampleRateJ = json_object_get(rootJ, "sampleRate");
	if (sampleRateJ) {
		float sampleRate = json_number_value(sampleRateJ);
		engineSetSampleRate(sampleRate);
	}
#endif // USE_VST2

	// lastPath
	json_t *lastPathJ = json_object_get(rootJ, "lastPath");
	if (lastPathJ)
		global_ui->app.gRackWidget->lastPath = json_string_value(lastPathJ);

	// skipAutosaveOnLaunch
	json_t *skipAutosaveOnLaunchJ = json_object_get(rootJ, "skipAutosaveOnLaunch");
	if (skipAutosaveOnLaunchJ)
		global->settings.gSkipAutosaveOnLaunch = json_boolean_value(skipAutosaveOnLaunchJ);

	// moduleBrowser
	json_t *moduleBrowserJ = json_object_get(rootJ, "moduleBrowser");
	if (moduleBrowserJ)
		appModuleBrowserFromJson(moduleBrowserJ);

	// powerMeter
	json_t *powerMeterJ = json_object_get(rootJ, "powerMeter");
	if (powerMeterJ)
		global->gPowerMeter = json_boolean_value(powerMeterJ);

	// checkVersion
	json_t *checkVersionJ = json_object_get(rootJ, "checkVersion");
	if (checkVersionJ)
		global_ui->app.gCheckVersion = json_boolean_value(checkVersionJ);
}


void settingsSave(std::string filename) {
	info("Saving settings %s", filename.c_str());
	json_t *rootJ = settingsToJson();
	if (rootJ) {
		FILE *file = fopen(filename.c_str(), "w");
		if (!file)
			return;

		json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
		json_decref(rootJ);
		fclose(file);
	}
}

void settingsLoad(std::string filename, bool bWindowSizeOnly) {
	info("Loading settings %s", filename.c_str());
	FILE *file = fopen(filename.c_str(), "r");
	if (!file)
		return;

	json_error_t error;
	json_t *rootJ = json_loadf(file, 0, &error);
	if (rootJ) {
		settingsFromJson(rootJ, bWindowSizeOnly);
		json_decref(rootJ);
	}
	else {
		warn("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
	}

	fclose(file);
}


} // namespace rack

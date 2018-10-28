#include "global_pre.hpp"
#include "settings.hpp"
#include "app.hpp"
#include "window.hpp"
#include "engine.hpp"
#include "plugin.hpp"
#include <jansson.h>
#include "global.hpp"
#include "global_ui.hpp"


#ifdef RACK_HOST
extern void vst2_window_size_set (int _width, int _height);
extern void vst2_refresh_rate_set (float _hz);
extern float vst2_refresh_rate_get (void);
extern void vst2_oversample_realtime_set (float _factor, int _quality);
extern void vst2_oversample_realtime_get (float *_factor, int *_quality);
extern void vst2_oversample_offline_set (float _factor, int _quality);
extern void vst2_oversample_offline_get (float *_factor, int *_quality);
extern void vst2_oversample_offline_check_set (int _bEnable);
extern int32_t vst2_oversample_offline_check_get (void);
extern void vst2_oversample_channels_set (int _numIn, int _numOut);
extern void vst2_oversample_channels_get (int *_numIn, int *_numOut);
extern void vst2_idle_detect_mode_fx_set (int _mode);
extern int vst2_idle_detect_mode_fx_get (void);
extern void vst2_idle_detect_mode_instr_set (int _mode);
extern int vst2_idle_detect_mode_instr_get (void);
#endif // RACK_HOST

namespace rack {

bool b_touchkeyboard_enable = false;  // true=support effEditKey*


static json_t *settingsToJson() {
	// root
	json_t *rootJ = json_object();

#ifdef RACK_HOST

	// token
	json_t *tokenJ = json_string(global->plugin.gToken.c_str());
	json_object_set_new(rootJ, "token", tokenJ);

#if 0
	if (!windowIsMaximized())
#endif
   {
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

   // refresh rate (Hz)
	float refreshRate = vst2_refresh_rate_get();
	json_t *refreshRateJ = json_integer(int(refreshRate));
	json_object_set_new(rootJ, "refreshRate", refreshRateJ);

   // vsync
	int vsync = lglw_swap_interval_get(global_ui->window.lglw);
	json_t *vsyncJ = json_boolean(vsync);
	json_object_set_new(rootJ, "vsync", vsyncJ);

   // fbo
	json_t *fboJ = json_boolean(global_ui->b_fbo);
	json_object_set_new(rootJ, "fbo", fboJ);

   // fbo_shared (dynamically loaded modules)
	json_t *fbo_sharedJ = json_boolean(global_ui->b_fbo_shared);
	json_object_set_new(rootJ, "fbo_shared", fbo_sharedJ);

   // touchInput
   int touchInput = lglw_touchinput_get(global_ui->window.lglw);
	json_t *touchInputJ = json_boolean(touchInput);
	json_object_set_new(rootJ, "touchInput", touchInputJ);

   // touchKbd
   int touchKbd = b_touchkeyboard_enable;
	json_t *touchKbdJ = json_boolean(touchKbd);
	json_object_set_new(rootJ, "touchKbd", touchKbdJ);

   // (realtime) oversampleFactor, oversampleQuality
   {
      float factor;
      int quality;
      vst2_oversample_realtime_get(&factor, &quality);

      json_t *factorJ = json_real(factor);
      json_object_set_new(rootJ, "oversampleFactor", factorJ);

      json_t *qualityJ = json_integer(quality);
      json_object_set_new(rootJ, "oversampleQuality", qualityJ);
   }

   // oversampleOfflineFactor, oversampleOfflineQuality
   {
      float factor;
      int quality;
      vst2_oversample_offline_get(&factor, &quality);

      json_t *factorJ = json_real(factor);
      json_object_set_new(rootJ, "oversampleOfflineFactor", factorJ);

      json_t *qualityJ = json_integer(quality);
      json_object_set_new(rootJ, "oversampleOfflineQuality", qualityJ);
   }

   // oversample offline check (oversampleOffline)
   json_t *offlineJ = json_boolean(vst2_oversample_offline_check_get());
   json_object_set_new(rootJ, "oversampleOffline", offlineJ);

   // oversample input channel limit (oversampleNumIn)
   // oversample output channel limit (oversampleNumOut)
   {
      int numIn;
      int numOut;
      vst2_oversample_channels_get(&numIn, &numOut);

      json_t *numInJ = json_real(numIn);
      json_object_set_new(rootJ, "oversampleNumIn", numInJ);

      json_t *numOutJ = json_real(numOut);
      json_object_set_new(rootJ, "oversampleNumOut", numOutJ);
   }

   // idleDetectInstr
   {
      int idleMode = vst2_idle_detect_mode_instr_get();

      json_t *idleJ = json_integer(idleMode);
      json_object_set_new(rootJ, "idleDetectInstr", idleJ);
   }

   // idleDetectFx
   {
      int idleMode = vst2_idle_detect_mode_fx_get();

      json_t *idleJ = json_integer(idleMode);
      json_object_set_new(rootJ, "idleDetectFx", idleJ);
   }

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

#endif // RACK_HOST

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
#ifdef RACK_HOST
         vst2_window_size_set((int)width, (int)height);
#endif // RACK_HOST
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
#ifdef RACK_HOST
		vst2_refresh_rate_set(clamp((float) json_number_value(refreshJ), 0.0f, 200.0f));
#endif // RACK_HOST
	}

	// vsync
   if(!bWindowSizeOnly)
   {
      json_t *vsyncJ = json_object_get(rootJ, "vsync");
      if (vsyncJ)
      {
         // lglw_glcontext_push(global_ui->window.lglw);
         // lglw_swap_interval_set(global_ui->window.lglw, json_is_true(vsyncJ);
         // lglw_glcontext_pop(global_ui->window.lglw);

         // postpone until first vst2_editor_redraw() call (see window.cpp)
         //  (note) on Linux we need a drawable to set the swap interval
         global_ui->pending_swap_interval = json_is_true(vsyncJ);
      }
   }

   // fbo support (not working with VirtualBox GL driver!)
   json_t *fboJ = json_object_get(rootJ, "fbo");
   if (fboJ)
   {
      global_ui->b_fbo = json_is_true(fboJ);
   }

   // fbo support in dynamically loaded modules (not working with VirtualBox GL driver or Windows NVidia driver)
   json_t *fbo_sharedJ = json_object_get(rootJ, "fbo_shared");
   if (fbo_sharedJ)
   {
      global_ui->b_fbo_shared = json_is_true(fbo_sharedJ);
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

	// touchKbd
   if(!bWindowSizeOnly)
   {
      json_t *touchJ = json_object_get(rootJ, "touchKbd");
      if (touchJ)
      {
         b_touchkeyboard_enable = json_is_true(touchJ);
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

#ifdef RACK_HOST
   // Realtime Oversample factor and quality
   {
      float oversampleFactor = -1.0f;
      int oversampleQuality = -1;

      // Realtime Oversample factor
      {
         json_t *oversampleJ = json_object_get(rootJ, "oversampleFactor");
         if (oversampleJ) {
            oversampleFactor = float(json_number_value(oversampleJ));
         }
      }

      // Realtime Oversample quality (0..10)
      {
         json_t *oversampleJ = json_object_get(rootJ, "oversampleQuality");
         if (oversampleJ) {
            oversampleQuality = int(json_number_value(oversampleJ));
         }
      }

      vst2_oversample_realtime_set(oversampleFactor, oversampleQuality);
   }

   // Offline Oversample factor and quality
   {
      float oversampleFactor = -1.0f;
      int oversampleQuality = -1;

      // Offline Oversample factor
      {
         json_t *oversampleJ = json_object_get(rootJ, "oversampleOfflineFactor");
         if (oversampleJ) {
            oversampleFactor = float(json_number_value(oversampleJ));
         }
      }

      // Offline Oversample quality (0..10)
      {
         json_t *oversampleJ = json_object_get(rootJ, "oversampleOfflineQuality");
         if (oversampleJ) {
            oversampleQuality = int(json_number_value(oversampleJ));
         }
      }

      vst2_oversample_offline_set(oversampleFactor, oversampleQuality);
   }

	json_t *checkOfflineJ = json_object_get(rootJ, "oversampleOffline");
	if (checkOfflineJ)
   {
      vst2_oversample_offline_check_set(json_is_true(checkOfflineJ));
   }

   // Oversample channel limit
   int oversampleNumIn = -1;
   int oversampleNumOut = -1;

	// Oversample input channel limit
   {
      json_t *oversampleJ = json_object_get(rootJ, "oversampleNumIn");
      if (oversampleJ) {
         oversampleNumIn = int(json_number_value(oversampleJ));
      }
   }

	// Oversample output channel limit
   {
      json_t *oversampleJ = json_object_get(rootJ, "oversampleNumOut");
      if (oversampleJ) {
         oversampleNumOut = int(json_number_value(oversampleJ));
      }
   }

   vst2_oversample_channels_set(oversampleNumIn, oversampleNumOut);

	// Idle detection mode (instrument build)
   {
      json_t *idleJ = json_object_get(rootJ, "idleDetectInstr");
      if (idleJ) {
         vst2_idle_detect_mode_instr_set(int(json_number_value(idleJ)));
      }
   }

	// Idle detection mode (FX build)
   {
      json_t *idleJ = json_object_get(rootJ, "idleDetectFx");
      if (idleJ) {
         vst2_idle_detect_mode_fx_set(int(json_number_value(idleJ)));
      }
   }
#endif // RACK_HOST

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

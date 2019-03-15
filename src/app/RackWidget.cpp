#include "global_pre.hpp"
#include "app.hpp"
#include "engine.hpp"
#include "plugin.hpp"
#include "window.hpp"
#include "settings.hpp"
#include "asset.hpp"
#include <map>
#include <algorithm>
#include "osdialog.h"
#include "global.hpp"
#include "global_ui.hpp"


#ifdef USE_VST2
extern void vst2_set_shared_plugin_tls_globals(void);

#ifdef RACK_HOST
extern void  vst2_oversample_realtime_set (float _factor, int _quality);
extern void  vst2_oversample_realtime_get (float *_factor, int *_quality);
extern void  vst2_oversample_channels_set (int _numIn, int _numOut);
extern void  vst2_oversample_channels_get (int *_numIn, int *_numOut);
extern void  vst2_idle_detect_mode_set (int _mode);
extern void  vst2_idle_detect_mode_get (int *_mode);
extern void  vst2_idle_grace_sec_set   (float _sec);
extern float vst2_idle_grace_sec_get   (void);
extern void  vst2_idle_output_sec_set  (float _sec);
extern float vst2_idle_output_sec_get  (void);
#endif // RACK_HOST

#endif // USE_VST2


#define Dprintf printf

#ifdef USE_LOG_PRINTF
extern void log_printf(const char *logData, ...);
#undef Dprintf
#define Dprintf log_printf
#endif // USE_LOG_PRINTF

namespace rack {


static const char *FILTERS = "VeeSeeVST Rack patch (.vcv):vcv";


struct ModuleContainer : Widget {
	void draw(NVGcontext *vg) override {
		// Draw shadows behind each ModuleWidget first, so the shadow doesn't overlap the front.
		for (Widget *child : children) {
			if (!child->visible)
				continue;
			nvgSave(vg);
			nvgTranslate(vg, child->box.pos.x, child->box.pos.y);
			ModuleWidget *w = dynamic_cast<ModuleWidget*>(child);
			w->drawShadow(vg);
			nvgRestore(vg);
		}

		Widget::draw(vg);
	}
};


RackWidget::RackWidget() {
	rails = new FramebufferWidget();
	rails->box.size = Vec();
	rails->oversample = 1.0;
	{
		RackRail *rail = new RackRail();
		rail->box.size = Vec();
		rails->addChild(rail);
	}
	addChild(rails);

	moduleContainer = new ModuleContainer();
	addChild(moduleContainer);

	wireContainer = new WireContainer();
	addChild(wireContainer);
}

RackWidget::~RackWidget() {
}

void RackWidget::clear() {
	wireContainer->activeWire = NULL;
	wireContainer->clearChildren();
	moduleContainer->clearChildren();

	global_ui->app.gRackScene->scrollWidget->offset = Vec(0, 0);
#ifdef USE_VST2
   global->vst2.next_unique_param_base_id = 1;
   global_ui->param_info.placeholder_framecount = (30*30)-1;
#endif // USE_VST2
}

void RackWidget::reset() {
	if (osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, "Clear patch and start over?")) {
		clear();
		// Fails silently if file does not exist
		loadPatch(assetLocal("template.vcv"));
		lastPath = "";
	}
}

void RackWidget::openDialog() {
	std::string dir;
	if (lastPath.empty()) {
		dir = assetLocal("patches");
		systemCreateDirectory(dir);
	}
	else {
		dir = stringDirectory(lastPath);
	}
	osdialog_filters *filters = osdialog_filters_parse(FILTERS);
	char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, filters);
	if (path) {
		loadPatch(path);
		lastPath = path;
		free(path);
	}
	osdialog_filters_free(filters);
}

void RackWidget::saveDialog() {
	if (!lastPath.empty()) {
		savePatch(lastPath);
	}
	else {
		saveAsDialog();
	}
}

void RackWidget::saveAsDialog() {
	std::string dir;
	std::string filename;
	if (lastPath.empty()) {
		dir = assetLocal("patches");
		systemCreateDirectory(dir);
	}
	else {
		dir = stringDirectory(lastPath);
		filename = stringFilename(lastPath);
	}
	osdialog_filters *filters = osdialog_filters_parse(FILTERS);
	char *path = osdialog_file(OSDIALOG_SAVE, dir.c_str(), filename.c_str(), filters);

	if (path) {
		std::string pathStr = path;
		free(path);
		std::string extension = stringExtension(pathStr);
		if (extension.empty()) {
			pathStr += ".vcv";
		}

		savePatch(pathStr);
		lastPath = pathStr;
	}
	osdialog_filters_free(filters);
}

void RackWidget::savePatch(std::string path) {
	info("Saving patch %s", path.c_str());
	json_t *rootJ = toJson();
	if (!rootJ)
		return;

	FILE *file = fopen(path.c_str(), "w");
	if (file) {
		json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
		fclose(file);
	}

	json_decref(rootJ);
}

#ifdef USE_VST2
char *RackWidget::savePatchToString(void) {
   // (note) caller must free() returned string
   info("Saving patch to string");
   char *r = NULL;

	json_t *rootJ = toJson();
	if (!rootJ)
		return NULL;

   r = json_dumps(rootJ, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
	json_decref(rootJ);
   return r;
}
#endif // USE_VST2

void RackWidget::loadPatch(std::string path) {
	info("Loading patch %s", path.c_str());
	FILE *file = fopen(path.c_str(), "r");
	if (!file) {
		// Exit silently
		return;
	}

	json_error_t error;
	json_t *rootJ = json_loadf(file, 0, &error);
	if (rootJ) {
		clear();
		fromJson(rootJ);
		json_decref(rootJ);
	}
	else {
		std::string message = stringf("JSON parsing error at %s %d:%d %s\n", error.source, error.line, error.column, error.text);
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, message.c_str());
	}

	fclose(file);
}

#ifdef USE_VST2
bool RackWidget::loadPatchFromString(const char *_string) {
   info("Loading patch from string");

	if (NULL == _string) {
		// Exit silently
		return false;
	}
   
	json_error_t error;
   Dprintf("call json_loads\n");
	json_t *rootJ = json_loads(_string, 0/*flags*/, &error);
   Dprintf("json_loads returned rootJ=%p\n", rootJ);
	if (rootJ) {
		clear();
#ifdef USE_VST2
      global->vst2.b_patch_loading = true;
#endif
      Dprintf("call fromJson\n");
		fromJson(rootJ);
      // Dprintf("fromJson returned\n");
#ifdef USE_VST2
      global->vst2.b_patch_loading = false;
#endif // USE_VST2
      Dprintf("call json_decref\n");
		json_decref(rootJ);
      Dprintf("json_decref returned\n");
      return true;
	}
	else {
		std::string message = stringf("JSON parsing error at %s %d:%d %s\n", error.source, error.line, error.column, error.text);
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, message.c_str());
	}

	return false;
}
#endif // USE_VST2

void RackWidget::revert() {
	if (lastPath.empty())
		return;
	if (osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, "Revert patch to the last saved state?")) {
		loadPatch(lastPath);
	}
}

void RackWidget::disconnect() {
	if (!osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK_CANCEL, "Remove all patch cables?"))
		return;

	for (Widget *w : moduleContainer->children) {
		ModuleWidget *moduleWidget = dynamic_cast<ModuleWidget*>(w);
		assert(moduleWidget);
		moduleWidget->disconnect();
	}
}

json_t *RackWidget::toJson() {
	// root
	json_t *rootJ = json_object();

	// version
	json_t *versionJ = json_string(global_ui->app.gApplicationVersion.c_str());
	json_object_set_new(rootJ, "version", versionJ);

#ifdef USE_VST2
#ifdef RACK_HOST
   {
      float oversampleFactor;
      int oversampleQuality;
      vst2_oversample_realtime_get(&oversampleFactor, &oversampleQuality);
      
      // Oversample factor
      {
         json_t *oversampleJ = json_real(oversampleFactor);
         json_object_set_new(rootJ, "oversampleFactor", oversampleJ);
      }

      // Oversample quality (0..10)
      {
         json_t *oversampleJ = json_real(oversampleQuality);
         json_object_set_new(rootJ, "oversampleQuality", oversampleJ);
      }
   }
   {
      int oversampleNumIn;
      int oversampleNumOut;
      vst2_oversample_channels_get(&oversampleNumIn, &oversampleNumOut);
      
      // Oversample input channel limit
      {
         json_t *oversampleJ = json_real(oversampleNumIn);
         json_object_set_new(rootJ, "oversampleNumIn", oversampleJ);
      }

      // Oversample output channel limit
      {
         json_t *oversampleJ = json_real(oversampleNumOut);
         json_object_set_new(rootJ, "oversampleNumOut", oversampleJ);
      }
   }

   // Idle detection mode
   {
      int idleDetect;
      vst2_idle_detect_mode_get(&idleDetect);
      
      json_t *idleJ = json_real(idleDetect);
      json_object_set_new(rootJ, "idleDetect", idleJ);
   }

   // Idle note-on grace period (sec)
   {
      float sec = vst2_idle_grace_sec_get();
      
      json_t *idleJ = json_real(sec);
      json_object_set_new(rootJ, "idleGraceSec", idleJ);
   }

   // Idle output silence threshold (sec)
   {
      float sec = vst2_idle_output_sec_get();
      
      json_t *idleJ = json_real(sec);
      json_object_set_new(rootJ, "idleOutputSec", idleJ);
   }
#endif // RACK_HOST
#endif // USE_VST2

	// modules
	json_t *modulesJ = json_array();
	std::map<ModuleWidget*, int> moduleIds;
	int moduleId = 0;
	for (Widget *w : moduleContainer->children) {
		ModuleWidget *moduleWidget = dynamic_cast<ModuleWidget*>(w);
		assert(moduleWidget);
		moduleIds[moduleWidget] = moduleId;
		moduleId++;
		// module
		json_t *moduleJ = moduleWidget->toJson();
		json_array_append_new(modulesJ, moduleJ);
	}
	json_object_set_new(rootJ, "modules", modulesJ);

	// wires
	json_t *wires = json_array();
	for (Widget *w : wireContainer->children) {
		WireWidget *wireWidget = dynamic_cast<WireWidget*>(w);
		assert(wireWidget);
		// Only serialize WireWidgets connected on both ends
		if (!(wireWidget->outputPort && wireWidget->inputPort))
			continue;
		// wire
		json_t *wire = wireWidget->toJson();

		// Get the modules at each end of the wire
		ModuleWidget *outputModuleWidget = wireWidget->outputPort->getAncestorOfType<ModuleWidget>();
		assert(outputModuleWidget);
		int outputModuleId = moduleIds[outputModuleWidget];

		ModuleWidget *inputModuleWidget = wireWidget->inputPort->getAncestorOfType<ModuleWidget>();
		assert(inputModuleWidget);
		int inputModuleId = moduleIds[inputModuleWidget];

		// Get output/input ports
		int outputId = wireWidget->outputPort->portId;
		int inputId = wireWidget->inputPort->portId;

		json_object_set_new(wire, "outputModuleId", json_integer(outputModuleId));
		json_object_set_new(wire, "outputId", json_integer(outputId));
		json_object_set_new(wire, "inputModuleId", json_integer(inputModuleId));
		json_object_set_new(wire, "inputId", json_integer(inputId));

		json_array_append_new(wires, wire);
	}
	json_object_set_new(rootJ, "wires", wires);

	return rootJ;
}

void RackWidget::fromJson(json_t *rootJ) {
	std::string message;

   // Dprintf("fromJson: 1\n");
   vst2_set_shared_plugin_tls_globals();  // update JSON hashtable seed

   // Dprintf("fromJson: 2\n");

	// version
	std::string version;
	json_t *versionJ = json_object_get(rootJ, "version");
	if (versionJ) {
		version = json_string_value(versionJ);
	}

   // Dprintf("fromJson: 3\n");

	// Detect old patches with ModuleWidget::params/inputs/outputs indices.
	// (We now use Module::params/inputs/outputs indices.)
	int legacy = 0;
	if (stringStartsWith(version, "0.3.") || stringStartsWith(version, "0.4.") || stringStartsWith(version, "0.5.") || version == "" || version == "dev") {
		legacy = 1;
	}
	if (legacy) {
		info("Loading patch using legacy mode %d", legacy);
	}

   // Dprintf("fromJson: 4\n");

#ifdef RACK_HOST
   float oversampleFactor = -1.0f;
   int oversampleQuality = -1;

	// Oversample factor
   {
      json_t *oversampleJ = json_object_get(rootJ, "oversampleFactor");
      if (oversampleJ) {
         oversampleFactor = float(json_number_value(oversampleJ));
      }
   }

   // Dprintf("fromJson: 5\n");

	// Oversample quality (0..10)
   {
      json_t *oversampleJ = json_object_get(rootJ, "oversampleQuality");
      if (oversampleJ) {
         oversampleQuality = int(json_number_value(oversampleJ));
      }
   }

   // Dprintf("fromJson: 6\n");

   vst2_oversample_realtime_set(oversampleFactor, oversampleQuality);

   // Dprintf("fromJson: 7\n");

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

   // Dprintf("fromJson: 8\n");

	// Oversample output channel limit
   {
      json_t *oversampleJ = json_object_get(rootJ, "oversampleNumOut");
      if (oversampleJ) {
         oversampleNumOut = int(json_number_value(oversampleJ));
      }
   }

   // Dprintf("fromJson: 9\n");

   vst2_oversample_channels_set(oversampleNumIn, oversampleNumOut);

   // Dprintf("fromJson: 10\n");

	// Idle detection mode
   {
      json_t *idleJ = json_object_get(rootJ, "idleDetect");
      if (idleJ) {
         vst2_idle_detect_mode_set(int(json_number_value(idleJ)));
      }
   }

	// Idle note-on grace period (sec)
   {
      json_t *idleJ = json_object_get(rootJ, "idleGraceSec");
      if (idleJ) {
         vst2_idle_grace_sec_set(float(json_number_value(idleJ)));
      }
      else {
         vst2_idle_grace_sec_set(0.75f);
      }
   }

	// Idle output silence threshold (sec)
   {
      json_t *idleJ = json_object_get(rootJ, "idleOutputSec");
      if (idleJ) {
         vst2_idle_output_sec_set(float(json_number_value(idleJ)));
      }
      else {
         vst2_idle_output_sec_set(0.2f);
      }
   }
#endif // RACK_HOST

   // Dprintf("fromJson: 11\n");

	// modules
	std::map<int, ModuleWidget*> moduleWidgets;
	json_t *modulesJ = json_object_get(rootJ, "modules");
	if (!modulesJ) return;
	size_t moduleId;
	json_t *moduleJ;
	json_array_foreach(modulesJ, moduleId, moduleJ) {
		// Add "legacy" property if in legacy mode
      // Dprintf("fromJson: 12.1\n");
		if (legacy) {
			json_object_set(moduleJ, "legacy", json_integer(legacy));
		}
      // Dprintf("fromJson: 12.2\n");

		json_t *pluginSlugJ = json_object_get(moduleJ, "plugin");
		if (!pluginSlugJ) continue;
      // Dprintf("fromJson: 12.3\n");
		json_t *modelSlugJ = json_object_get(moduleJ, "model");
		if (!modelSlugJ) continue;
      // Dprintf("fromJson: 12.4\n");
		std::string pluginSlug = json_string_value(pluginSlugJ);
      // Dprintf("fromJson: 12.5\n");
		std::string modelSlug = json_string_value(modelSlugJ);
      // Dprintf("fromJson: 12.6\n");

		Model *model = pluginGetModel(pluginSlug, modelSlug);
      // Dprintf("fromJson: 12.7\n");
		if (!model) {
			message += stringf("Could not find module \"%s\" of plugin \"%s\"\n", modelSlug.c_str(), pluginSlug.c_str());
			continue;
		}
      // Dprintf("fromJson: 12.8\n");

		// Create ModuleWidget
// // #ifdef USE_VST2
// //       if(NULL != model->plugin->set_tls_globals_fxn) {
// //          model->plugin->set_tls_globals_fxn(model->plugin);
// //       }
// // #endif // USE_VST2
      // Dprintf("fromJson: 12.9\n");
		ModuleWidget *moduleWidget = model->createModuleWidget();
      // Dprintf("fromJson: 12.10\n");
		assert(moduleWidget);
      // Dprintf("fromJson: 12.11\n");
		moduleWidget->fromJson(moduleJ);
      // Dprintf("fromJson: 12.12\n");
		moduleContainer->addChild(moduleWidget);
      // Dprintf("fromJson: 12.13\n");
		moduleWidgets[moduleId] = moduleWidget;
      // Dprintf("fromJson: 12.14\n");
	}

   // Dprintf("fromJson: 13\n");

	// wires
	json_t *wiresJ = json_object_get(rootJ, "wires");
	if (!wiresJ) return;
   // Dprintf("fromJson: 14\n");
	size_t wireId;
	json_t *wireJ;
	json_array_foreach(wiresJ, wireId, wireJ) {
      // Dprintf("fromJson: 14.1\n");
		int outputModuleId = json_integer_value(json_object_get(wireJ, "outputModuleId"));
      // Dprintf("fromJson: 14.2\n");
		int outputId = json_integer_value(json_object_get(wireJ, "outputId"));
      // Dprintf("fromJson: 14.3\n");
		int inputModuleId = json_integer_value(json_object_get(wireJ, "inputModuleId"));
      // Dprintf("fromJson: 14.4\n");
		int inputId = json_integer_value(json_object_get(wireJ, "inputId"));
      // Dprintf("fromJson: 14.5\n");

		// Get module widgets
		ModuleWidget *outputModuleWidget = moduleWidgets[outputModuleId];
      // Dprintf("fromJson: 14.6\n");
		if (!outputModuleWidget) continue;
      // Dprintf("fromJson: 14.7\n");
		ModuleWidget *inputModuleWidget = moduleWidgets[inputModuleId];
      // Dprintf("fromJson: 14.8\n");
		if (!inputModuleWidget) continue;
      // Dprintf("fromJson: 14.9\n");

		// Get port widgets
		Port *outputPort = NULL;
		Port *inputPort = NULL;
		if (legacy && legacy <= 1) {
			// Legacy 1 mode
			// The index of the "ports" array is the index of the Port in the `outputs` and `inputs` vector.
			outputPort = outputModuleWidget->outputs[outputId];
			inputPort = inputModuleWidget->inputs[inputId];
		}
		else {
			for (Port *port : outputModuleWidget->outputs) {
				if (port->portId == outputId) {
					outputPort = port;
					break;
				}
			}
			for (Port *port : inputModuleWidget->inputs) {
				if (port->portId == inputId) {
					inputPort = port;
					break;
				}
			}
		}
		if (!outputPort || !inputPort)
			continue;
      // Dprintf("fromJson: 14.10\n");

		// Create WireWidget
		WireWidget *wireWidget = new WireWidget();
      // Dprintf("fromJson: 14.11\n");
		wireWidget->fromJson(wireJ);
      // Dprintf("fromJson: 14.12\n");
		wireWidget->outputPort = outputPort;
		wireWidget->inputPort = inputPort;
		wireWidget->updateWire();
      // Dprintf("fromJson: 14.13\n");
		// Add wire to rack
		wireContainer->addChild(wireWidget);
      // Dprintf("fromJson: 14.14\n");
	}

   // Dprintf("fromJson: 15\n");

	// Display a message if we have something to say
	if (!message.empty()) {
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, message.c_str());
	}

   // Dprintf("fromJson: 16\n");

#ifdef USE_VST2
   global_ui->param_info.placeholder_framecount = (30*30)-10;
   global_ui->param_info.last_param_widget = NULL;
#endif // USE_VST2

   // Dprintf("fromJson: LEAVE\n");
}

void RackWidget::addModule(ModuleWidget *m) {
   rack::global_ui->app.mtx_param.lock();
	moduleContainer->addChild(m);
	m->create();
   rack::global_ui->app.mtx_param.unlock();
}

void RackWidget::deleteModule(ModuleWidget *m) {
   rack::global_ui->app.mtx_param.lock();
	m->_delete();
	moduleContainer->removeChild(m);
   rack::global_ui->app.mtx_param.unlock();
}

ModuleWidget *RackWidget::findModuleWidgetByModule(Module *_module) {
	for (Widget *w : moduleContainer->children) {
		ModuleWidget *moduleWidget = dynamic_cast<ModuleWidget*>(w);
      if(moduleWidget->module == _module)
         return moduleWidget;
   }
   return NULL;   
}

ParamWidget *RackWidget::findParamWidgetAndUniqueParamIdByWidgetRef(const ParamWidget *ref, int *retUniqueParamId) {
	for(Widget *w : moduleContainer->children) {
		ModuleWidget *moduleWidget = dynamic_cast<ModuleWidget*>(w);
      for(ParamWidget *param : moduleWidget->params) {
         if( (void*)param == (void*)ref ) {
            *retUniqueParamId = moduleWidget->module->vst2_unique_param_base_id + param->paramId;
            return param;
         }
      }
   }
   return NULL;
}

void RackWidget::cloneModule(ModuleWidget *m) {
	// Create new module from model
	ModuleWidget *clonedModuleWidget = m->model->createModuleWidget();
	// JSON serialization is the most straightforward way to do this
	json_t *moduleJ = m->toJson();
   global_ui->app.bLoadVSTUniqueParamBaseId = false;
	clonedModuleWidget->fromJson(moduleJ);
   global_ui->app.bLoadVSTUniqueParamBaseId = true;
	json_decref(moduleJ);
	Rect clonedBox = clonedModuleWidget->box;
	clonedBox.pos = m->box.pos;
	requestModuleBoxNearest(clonedModuleWidget, clonedBox);
	addModule(clonedModuleWidget);
}

bool RackWidget::requestModuleBox(ModuleWidget *m, Rect box) {
	if (box.pos.x < 0 || box.pos.y < 0)
		return false;

	for (Widget *child2 : moduleContainer->children) {
		if (m == child2) continue;
		if (box.intersects(child2->box)) {
			return false;
		}
	}
	m->box = box;
	return true;
}

bool RackWidget::requestModuleBoxNearest(ModuleWidget *m, Rect box) {
	// Create possible positions
	int x0 = roundf(box.pos.x / RACK_GRID_WIDTH);
	int y0 = roundf(box.pos.y / RACK_GRID_HEIGHT);
	std::vector<Vec> positions;
	for (int y = max(0, y0 - 8); y < y0 + 8; y++) {
		for (int x = max(0, x0 - 400); x < x0 + 400; x++) {
			positions.push_back(Vec(x * RACK_GRID_WIDTH, y * RACK_GRID_HEIGHT));
		}
	}

	// Sort possible positions by distance to the requested position
	std::sort(positions.begin(), positions.end(), [box](Vec a, Vec b) {
		return a.minus(box.pos).norm() < b.minus(box.pos).norm();
	});

	// Find a position that does not collide
	for (Vec position : positions) {
		Rect newBox = box;
		newBox.pos = position;
		if (requestModuleBox(m, newBox))
			return true;
	}
	return false;
}

void RackWidget::step() {
	// Expand size to fit modules
	Vec moduleSize = moduleContainer->getChildrenBoundingBox().getBottomRight();
	// We assume that the size is reset by a parent before calling step(). Otherwise it will grow unbounded.
	box.size = box.size.max(moduleSize);

	// Adjust size and position of rails
	Widget *rail = rails->children.front();
	Rect bound = getViewport(Rect(Vec(), box.size));
	if (!rails->box.contains(bound)) {
		Vec cellMargin = Vec(20, 1);
		rails->box.pos = bound.pos.div(RACK_GRID_SIZE).floor().minus(cellMargin).mult(RACK_GRID_SIZE);
		rails->box.size = bound.size.plus(cellMargin.mult(RACK_GRID_SIZE).mult(2));
		rails->dirty = true;

		rail->box.size = rails->box.size;
	}

#ifndef USE_VST2
	// Autosave every 15 seconds
	if (global_ui->window.gGuiFrame % (60 * 15) == 0) {
		savePatch(assetLocal("autosave.vcv"));
		settingsSave(assetLocal("settings.json"));
	}
#endif // USE_VST2

	Widget::step();
}

void RackWidget::draw(NVGcontext *vg) {
   // printf("xxx RackWidget::draw\n");
	Widget::draw(vg);
}

void RackWidget::onMouseMove(EventMouseMove &e) {
	OpaqueWidget::onMouseMove(e);
	lastMousePos = e.pos;
}

void RackWidget::onMouseDown(EventMouseDown &e) {
	Widget::onMouseDown(e);
	if (e.consumed)
		return;

	if (e.button == 1) {
		appModuleBrowserCreate();
	}
	e.consumed = true;
	e.target = this;
}

void RackWidget::onZoom(EventZoom &e) {
	rails->box.size = Vec();
	Widget::onZoom(e);
}


} // namespace rack

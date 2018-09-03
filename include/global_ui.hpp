#pragma once

#include <set>
#include <map>

#include "util/math.hpp"  // Vec
#include "tags.hpp"  // ModelTag enum
#include "widgets.hpp"


struct NVGcontext;


namespace rack {


struct Font;
struct Widget;
struct RackWidget;
struct Toolbar;
struct RackScene;
struct Scene;
struct Model;
struct ParamWidget;
struct TextField;


//
// the structure fields reflect the original file locations
//  (e.g. 'window' was originally located in 'window.cpp')
//
struct GlobalUI {

   struct {
      void *lglw; // lglw_t
      NVGcontext *gVg;
      NVGcontext *gFramebufferVg;
      std::shared_ptr<Font> gGuiFont;
      float gPixelRatio;
      float gWindowRatio;
      bool gAllowCursorLock;
      int gGuiFrame;
      Vec gMousePos;

      std::string lastWindowTitle;

      int windowX;
      int windowY;
      int windowWidth;
      int windowHeight;

      std::map<std::string, std::weak_ptr<Font>> font_cache;
      std::map<std::string, std::weak_ptr<Image>> image_cache;
      std::map<std::string, std::weak_ptr<SVG>> svg_cache;
   } window;

   struct {
      Widget *gHoveredWidget;
      Widget *gDraggedWidget;
      Widget *gDragHoveredWidget;
      Widget *gFocusedWidget;
      Widget *gTempWidget;
   } widgets;

   struct {
      Scene *gScene;
   } ui;

   struct {
      std::set<Model*> sFavoriteModels;
      std::string sAuthorFilter;
      ModelTag sTagFilter;
   } module_browser;

   struct {
      std::string gApplicationName;
      std::string gApplicationVersion;
      std::string gApiHost;
      std::string gLatestVersion;
      bool gCheckVersion;

      RackWidget *gRackWidget;
      Toolbar *gToolbar;
      RackScene *gRackScene;

      std::mutex mtx_param;

      bool bLoadVSTUniqueParamBaseId;  // temp. false while cloning ModuleWidget
   } app;

   struct {
      int bnd_icon_image;
      int bnd_font;
   } blendish;

   struct {
      const ParamWidget *last_param_widget;  // never dereferenced, may have already been deleted. unset after redraw().
      int last_param_gid;      // updated during redraw()
      float last_param_value;  // updated in onMouseMove() and onChange(). corresponding param may not exist anymore.
      float value_clipboard;
      TextField *tf_id;
      TextField *tf_value;
      bool b_lock;  // true=don't update info (e.g. when receiving VST parameter updates from host)
      int placeholder_framecount;
   } param_info;


   void init(void) {
      
      window.lglw = NULL;
      window.gVg = NULL;
      window.gFramebufferVg = NULL;
      window.gPixelRatio = 1.0;
      window.gWindowRatio = 1.0;
      window.gAllowCursorLock = true;
      window.gGuiFrame = 0;
      window.windowX = 0;
      window.windowY = 0;
      window.windowWidth = 0;
      window.windowHeight = 0;

      widgets.gHoveredWidget = NULL;
      widgets.gDraggedWidget = NULL;
      widgets.gDragHoveredWidget = NULL;
      widgets.gFocusedWidget = NULL;
      widgets.gTempWidget = NULL;

      ui.gScene = NULL;

      module_browser.sTagFilter = ModelTag::NO_TAG;

      app.gApplicationName = "VeeSeeVST Rack";
      app.gApplicationVersion = TOSTRING(VERSION);
      app.gApiHost = "https://api.vcvrack.com";
      // app.gApiHost = "http://localhost:8081";
      app.gCheckVersion = true;

      app.gRackWidget = NULL;
      app.gToolbar = NULL;
      app.gRackScene = NULL;
  
      app.bLoadVSTUniqueParamBaseId = true;

      blendish.bnd_icon_image = -1;
      blendish.bnd_font = -1;

      param_info.last_param_widget = NULL;
      param_info.last_param_gid = 0;
      param_info.last_param_value = 0.0f;
      param_info.value_clipboard = 0.0f;
      param_info.tf_id = NULL;
      param_info.tf_value = NULL;
      param_info.b_lock = false;
      param_info.placeholder_framecount = 0;
   } 

};


} // namespace rack

#pragma once

#include <set>
#include <map>

#include "util/math.hpp"  // Vec
#include "tags.hpp"  // ModelTag enum
#include "widgets.hpp"


struct GLFWwindow;
struct NVGcontext;


namespace rack {


struct Font;
struct Widget;
struct RackWidget;
struct Toolbar;
struct RackScene;
struct Scene;
struct Model;


//
// the structure fields reflect the original file locations
//  (e.g. 'window' was originally located in 'window.cpp')
//
struct GlobalUI {

   struct {
      GLFWwindow *gWindow;
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

#ifdef USE_VST2
   struct {
      volatile int b_close_window;
      volatile int b_hide_window;
#ifdef WIN32
      void *parent_hwnd;
      bool b_queued_maximize_window;
#endif // WIN32
   } vst2;
#endif // USE_VST2

   void init(void) {
      window.gWindow = NULL;
      window.gVg = NULL;
      window.gFramebufferVg = NULL;
      window.gPixelRatio = 1.0;
      window.gWindowRatio = 1.0;
// #ifdef USE_VST2
//       window.gAllowCursorLock = false;
// #else
      window.gAllowCursorLock = true;
// #endif // USE_VST2
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

#ifdef USE_VST2
      vst2.b_close_window = 0;
      vst2.b_hide_window = 0;
#ifdef WIN32
      vst2.parent_hwnd = 0;
      vst2.b_queued_maximize_window = false;
#endif
#endif // USE_VST2
   } 

};


} // namespace rack

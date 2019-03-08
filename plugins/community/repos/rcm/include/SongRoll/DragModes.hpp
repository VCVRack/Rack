#include "rack.hpp"

using namespace rack;

namespace rack_plugin_rcm {

namespace SongRoll {

  struct SongRollModule;
  struct SongRollWidget;

  struct ModuleDragType {
    SongRollWidget* widget;
    SongRollModule* module;

    ModuleDragType(SongRollWidget* widget, SongRollModule* module);
    virtual ~ModuleDragType();

    virtual void onDragMove(EventDragMove& e) = 0;
  };

  struct StandardModuleDragging : public ModuleDragType {
    StandardModuleDragging(SongRollWidget* widget, SongRollModule* module);
    virtual ~StandardModuleDragging();

    void onDragMove(EventDragMove& e) override;
  };

  struct ColourDragging : public ModuleDragType {
    ColourDragging(SongRollWidget* widget, SongRollModule* module);
    virtual ~ColourDragging();

    void onDragMove(EventDragMove& e) override;
  };

}

} // namespace rack_plugin_rcm

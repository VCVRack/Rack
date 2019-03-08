#include "rack.hpp"
#include "../ModuleDragType.hpp"

using namespace rack;

namespace rack_plugin_rcm {

struct Auditioner;
struct PatternData;
struct Transport;
struct WidgetState;
struct UnderlyingRollAreaWidget;

struct PianoRollDragType : ModuleDragType {
	PianoRollDragType();
	virtual ~PianoRollDragType();
};


struct PlayPositionDragging : public PianoRollDragType {
	Auditioner* auditioner;
	UnderlyingRollAreaWidget* widget;
	Transport* transport;

	PlayPositionDragging(Auditioner* auditioner, UnderlyingRollAreaWidget* widget, Transport* transport);
	virtual ~PlayPositionDragging();

	void setNote(Vec mouseRel);
	void onDragMove(EventDragMove& e) override;
};

struct LockMeasureDragging : public PianoRollDragType {
	std::chrono::time_point<std::chrono::high_resolution_clock> longPressStart;

	WidgetState* state;
	Transport* transport;

	LockMeasureDragging(WidgetState* state, Transport* transport);
	virtual ~LockMeasureDragging();

	void onDragMove(EventDragMove& e) override;
};

struct KeyboardDragging : public PianoRollDragType {
	float offset = 0;
	WidgetState* state;

  KeyboardDragging(WidgetState* state);
  virtual ~KeyboardDragging();

	void onDragMove(EventDragMove& e) override;
};

struct NotePaintDragging : public PianoRollDragType {
	int lastDragBeatDiv = -1000;
	int lastDragPitch = -1000;
	bool makeStepsActive = true;
	int retriggerBeatDiv = 0;

	UnderlyingRollAreaWidget* widget;
	PatternData* patternData;
	Transport* transport;
	Auditioner* auditioner;

	NotePaintDragging(UnderlyingRollAreaWidget* widget, PatternData* patternData, Transport* transport, Auditioner* auditioner);
	virtual ~NotePaintDragging();

	void onDragMove(EventDragMove& e) override;
};

struct VelocityDragging : public PianoRollDragType {
	UnderlyingRollAreaWidget* widget;
	PatternData* patternData;
	Transport* transport;
	WidgetState* state;

  int pattern;
	int measure;
	int division;

	bool showLow = false;

	VelocityDragging(UnderlyingRollAreaWidget* widget, PatternData* patternData, Transport* transport, WidgetState* state, int pattern, int measure, int division);
	virtual  ~VelocityDragging();

	void onDragMove(EventDragMove& e) override;
};

} // namespace rack_plugin_rcm

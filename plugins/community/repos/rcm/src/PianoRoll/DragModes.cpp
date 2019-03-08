#include "window.hpp"
#include "../../include/PianoRoll/DragModes.hpp"
#include "../../include/PianoRoll/RollAreaWidget.hpp"
#include "../../include/PianoRoll/Auditioner.hpp"
#include "../../include/PianoRoll/PatternData.hpp"
#include "../../include/PianoRoll/Transport.hpp"

namespace rack_plugin_rcm {

static const float VELOCITY_SENSITIVITY = 0.0015f;
static const float KEYBOARDDRAG_SENSITIVITY = 0.1f;

PianoRollDragType::PianoRollDragType() {}

PianoRollDragType::~PianoRollDragType() {}


PlayPositionDragging::PlayPositionDragging(Auditioner* auditioner, UnderlyingRollAreaWidget* widget, Transport* transport): auditioner(auditioner), widget(widget), transport(transport) {
	setNote(Vec(0,0));
}

PlayPositionDragging::~PlayPositionDragging() {
	auditioner->stop();
}

void PlayPositionDragging::onDragMove(EventDragMove& e) {
	setNote(e.mouseRel);
}

void PlayPositionDragging::setNote(Vec mouseRel) {
  Vec pos(widget->lastMouseDown.x + mouseRel.x, widget->lastMouseDown.y + mouseRel.y);
	widget->lastMouseDown = pos;

	Rect roll(Vec(0,0), Vec(widget->box.size.x, widget->box.size.y));
	widget->reserveKeysArea(roll);

  auto beatDivs = widget->getBeatDivs(roll);
  bool beatDivFound = false;
  BeatDiv cellBeatDiv;

  for (auto const& beatDiv: beatDivs) {
    if (Rect(Vec(beatDiv.pos.x, 0), Vec(beatDiv.size.x, widget->box.size.y)).contains(pos)) {
      cellBeatDiv = beatDiv;
      beatDivFound = true;
      break;
    }
  }

  if (beatDivFound) {
    transport->setMeasure(widget->state->currentMeasure);
    transport->setStepInMeasure(cellBeatDiv.num);
    auditioner->start(transport->currentStepInPattern());
  } else {
		auditioner->stop();
  }
}

LockMeasureDragging::LockMeasureDragging(WidgetState* state, Transport* transport) : state(state), transport(transport) {
	longPressStart = std::chrono::high_resolution_clock::now();
	state->measureLockPressTime = 0.f;
	state->dirty = true;
}

LockMeasureDragging::~LockMeasureDragging() {
	state->measureLockPressTime = 0.f;
	state->dirty = true;
}

void LockMeasureDragging::onDragMove(EventDragMove &e) {
	auto currTime = std::chrono::high_resolution_clock::now();
	double pressTime = std::chrono::duration<double>(currTime - longPressStart).count();
	state->measureLockPressTime = clamp(pressTime, 0.f, 1.f);
	state->dirty = true;
	if (pressTime >= 1.f) {

		if (!transport->isLocked() || (transport->currentMeasure() != state->currentMeasure)) {
			transport->lockMeasure();

			if (transport->currentMeasure() != state->currentMeasure) {
				// We just locked the measure, but the play point is outside the selected measure - move the play point into the last note of the current measure
				transport->setMeasure(state->currentMeasure);
			}
		} else {
			transport->unlockMeasure();
		}

		longPressStart = std::chrono::high_resolution_clock::now();
	}
}

KeyboardDragging::KeyboardDragging(WidgetState* state) : state(state) {
  windowCursorLock();
}

KeyboardDragging::~KeyboardDragging() {
  windowCursorUnlock();
}

void KeyboardDragging::onDragMove(EventDragMove& e) {
	float speed = 1.f;
	float range = 1.f;

	float delta = KEYBOARDDRAG_SENSITIVITY * e.mouseRel.y * speed * range;
	if (windowIsModPressed()) {
		delta /= 16.f;
	}

	offset += delta;

	while (offset >= 1.f) {
		state->lowestDisplayNote = clamp(state->lowestDisplayNote + 1, -1 * 12, 8 * 12);
		state->dirty = true;
		offset -= 1;
	}

	while (offset <= -1.f) {
		state->lowestDisplayNote = clamp(state->lowestDisplayNote - 1, -1 * 12, 8 * 12);
		state->dirty = true;
		offset += 1;
	}
}

NotePaintDragging::NotePaintDragging(UnderlyingRollAreaWidget* widget, PatternData* patternData, Transport* transport, Auditioner* auditioner) : widget(widget), patternData(patternData), transport(transport), auditioner(auditioner) {
  Vec pos = widget->lastMouseDown;

	std::tuple<bool, BeatDiv, Key> cell = widget->findCell(pos);
	if (!std::get<0>(cell)) {
		return;
	}

	int beatDiv = std::get<1>(cell).num;
	int pitch = std::get<2>(cell).pitch();

	bool wasAlreadyActive = patternData->isStepActive(transport->currentPattern(), widget->state->currentMeasure, beatDiv);
	bool wasAlreadyRetriggered = patternData->isStepRetriggered(transport->currentPattern(), widget->state->currentMeasure, beatDiv);

	retriggerBeatDiv = !wasAlreadyActive || wasAlreadyRetriggered ? beatDiv : -1;

	if (pitch == patternData->getStepPitch(transport->currentPattern(), widget->state->currentMeasure, beatDiv)) {
		makeStepsActive = !patternData->isStepActive(transport->currentPattern(), widget->state->currentMeasure, beatDiv);
	} else {
		makeStepsActive = true;
	}
}

NotePaintDragging::~NotePaintDragging() {
	if (makeStepsActive) {
		auditioner->stop();
	}
}

void NotePaintDragging::onDragMove(EventDragMove& e) {
  Vec pos(widget->lastMouseDown.x + e.mouseRel.x, widget->lastMouseDown.y + e.mouseRel.y);
	widget->lastMouseDown = pos;

	std::tuple<bool, BeatDiv, Key> cell = widget->findCell(pos);
	if (!std::get<0>(cell)) {
		auditioner->stop();
		return;
	}

	int beatDiv = std::get<1>(cell).num;
	int pitch = std::get<2>(cell).pitch();

	if (lastDragBeatDiv != beatDiv || lastDragPitch != pitch) {
		lastDragBeatDiv = beatDiv;
		lastDragPitch = pitch;
		if (makeStepsActive) {
			bool wasAlreadyActive = patternData->isStepActive(transport->currentPattern(), widget->state->currentMeasure, beatDiv);

			patternData->setStepActive(transport->currentPattern(), widget->state->currentMeasure, beatDiv, true);
			patternData->setStepPitch(transport->currentPattern(), widget->state->currentMeasure, beatDiv, pitch);
			
			if (beatDiv < retriggerBeatDiv) {
				patternData->setStepRetrigger(transport->currentPattern(), widget->state->currentMeasure, retriggerBeatDiv, false);
				retriggerBeatDiv = beatDiv;
			}

			patternData->setStepRetrigger(transport->currentPattern(), widget->state->currentMeasure, beatDiv, beatDiv == retriggerBeatDiv);

			if (!wasAlreadyActive) {
				patternData->setStepVelocity(transport->currentPattern(), widget->state->currentMeasure, beatDiv, 0.75);
			}

			patternData->adjustVelocity(transport->currentPattern(), widget->state->currentMeasure, beatDiv, 0.f);

			auditioner->start(beatDiv + (patternData->getStepsPerMeasure(transport->currentPattern()) * widget->state->currentMeasure));
			auditioner->retrigger();
		} else {
			patternData->setStepActive(transport->currentPattern(), widget->state->currentMeasure, beatDiv, false);
	    patternData->setStepRetrigger(transport->currentPattern(), widget->state->currentMeasure, beatDiv, false);
		}
	};
}

VelocityDragging::VelocityDragging(UnderlyingRollAreaWidget* widget, PatternData* patternData, Transport* transport, WidgetState* state, int pattern, int measure, int division) 
  : widget(widget),
		patternData(patternData),
		transport(transport),
		state(state),
		pattern(pattern),
    measure(measure),
    division(division) {
  windowCursorLock();

	Rect roll(Vec(0,0), Vec(widget->box.size.x, widget->box.size.y));
	widget->reserveKeysArea(roll);

	roll.size.y = roll.size.y / 2.f;
	showLow = roll.contains(widget->lastMouseDown);
}

VelocityDragging::~VelocityDragging() {
	windowCursorUnlock();
	state->displayVelocityHigh = -1;
	state->displayVelocityLow = -1;
	state->dirty = true;
}

void VelocityDragging::onDragMove(EventDragMove& e) {
  Vec pos(widget->lastMouseDown.x + e.mouseRel.x, widget->lastMouseDown.y + e.mouseRel.y);
	widget->lastMouseDown = pos;

	float speed = 1.f;
	float range = 1.f;
	float delta = VELOCITY_SENSITIVITY * -e.mouseRel.y * speed * range;
	if (windowIsModPressed()) {
		delta /= 16.f;
	}

	float newVelocity = patternData->adjustVelocity(transport->currentPattern(), measure, division, delta);
	if (showLow) {
		state->displayVelocityHigh = -1;
		state->displayVelocityLow = newVelocity;
	} else {
		state->displayVelocityHigh = newVelocity;
		state->displayVelocityLow = -1;
	}
	state->dirty = true;
}

} // namespace rack_plugin_rcm

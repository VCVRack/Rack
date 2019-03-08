#include "rack.hpp"
#include "../rcm.h"
#include "../../include/PianoRoll/Transport.hpp"
#include "../../include/PianoRoll/PatternData.hpp"

using namespace rack;

namespace rack_plugin_rcm {

Transport::Transport(PatternData* patternData) {
  this->patternData = patternData;

  locked = false;
  running = true;
  recording = false;
  pendingRecording = false;
}

int Transport::currentPattern() {
  return pattern;
}

int Transport::currentMeasure() {
  return stepInPattern / patternData->getStepsPerMeasure(pattern);
}

int Transport::currentStepInMeasure() {
  return stepInPattern % patternData->getStepsPerMeasure(pattern);
}

int Transport::currentStepInPattern() {
  return stepInPattern;
}

bool Transport::isLastStepOfPattern() {
  return stepInPattern == (patternData->getStepsInPattern(pattern) - 1);
}

void Transport::setPattern(int pattern) {
  pattern = clamp(pattern, 0, 63);
  if (pattern != this->pattern) {
    dirty = true;
    this->pattern = pattern;
    this->stepInPattern = -1;
  }
}

void Transport::setMeasure(int measure) {
  int firstStepInPattern = patternData->getStepsPerMeasure(pattern) * measure;
  int lastStepInPattern = firstStepInPattern + patternData->getStepsPerMeasure(pattern) - 1;
  if (stepInPattern < firstStepInPattern || stepInPattern > lastStepInPattern) {
    dirty = true;
    stepInPattern = lastStepInPattern;
  }
}

void Transport::setStepInMeasure(int stepInMeasure) {
  int newStepInPattern = (currentMeasure() * patternData->getStepsPerMeasure(pattern)) + (stepInMeasure % patternData->getStepsPerMeasure(pattern));
  dirty |= (newStepInPattern != stepInPattern);
  stepInPattern = newStepInPattern;
}

void Transport::setStepInPattern(int stepInPattern) {
  dirty |= (this->stepInPattern != stepInPattern);
  this->stepInPattern = stepInPattern;
}

void Transport::advancePattern(int offset) {
  dirty |= (offset != 0);
  setPattern(pattern + offset);
}

void Transport::advanceStep() {
  if (!running) {
    return;
  }

  dirty = true;

  int firstStepInLoop = 0;
  int measure = currentMeasure();
  stepInPattern = (stepInPattern + 1) % patternData->getStepsInPattern(pattern);
  int newMeasure = currentMeasure();

  if (locked) {
    firstStepInLoop = measure * patternData->getStepsPerMeasure(pattern);

    if (measure != newMeasure) {
      // We moved on to another measure in the pattern, adjust back to the start of our locked measure
      stepInPattern = firstStepInLoop;
    }
  }

  if (recording && stepInPattern == firstStepInLoop) {
    pendingRecording = false;
    recording = false;
  }

  if (pendingRecording && stepInPattern == firstStepInLoop) {
    pendingRecording = false;
    recording = true;
  }
}

void Transport::lockMeasure() {
  dirty |= (locked == false);
  locked = true;
}

void Transport::unlockMeasure() {
  dirty |= (locked == true);
  locked = false;
}

bool Transport::isLocked() {
  return locked;
}

void Transport::toggleRun() {
  dirty = true;
  running = !running;
}

void Transport::setRun(bool running) {
  dirty |= this->running != running;
  this->running = running;
}

bool Transport::isRunning() {
  return running;
}

void Transport::toggleRecording() {
  dirty = true;
  if (!recording) {
    pendingRecording = !pendingRecording;
  } else {
    recording = false;
    pendingRecording = false;
  }
}

bool Transport::isRecording() {
  return recording;
}

bool Transport::isPendingRecording() {
  return pendingRecording;
}

void Transport::reset() {
  dirty = true;
  stepInPattern = -1;
}

bool Transport::consumeDirty() {
  bool wasdirty = dirty;
  dirty = false;
  return wasdirty;
}

} // namespace rack_plugin_rcm

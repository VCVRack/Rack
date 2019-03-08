#include "../../include/PianoRoll/PatternData.hpp"
#include "rack.hpp"

using namespace rack;

namespace rack_plugin_rcm {

static const int MAX_PATTERNS=64;

PatternData::PatternData() {
  patterns.resize(MAX_PATTERNS);
  reset();
}

int PatternData::getStepsInPattern(int pattern) const {
  pattern = clamp(pattern, 0, patterns.size()-1);
  return getStepsPerMeasure(pattern) * patterns[pattern].numberOfMeasures;
}

int PatternData::getStepsPerMeasure(int pattern) const {
  pattern = clamp(pattern, 0, patterns.size()-1);
  return patterns[pattern].beatsPerMeasure * patterns[pattern].divisionsPerBeat;
}

void PatternData::setMeasures(int pattern, int measures) {
  dirty = true;
  pattern = clamp(pattern, 0, patterns.size()-1);
  while ((int)patterns[pattern].measures.size() <= measures) {
    Measure newMeasure;
    newMeasure.steps.resize(getStepsPerMeasure(pattern));
    patterns[pattern].measures.push_back(newMeasure);
  }
  patterns[pattern].numberOfMeasures = measures;
}

int PatternData::getMeasures(int pattern) const {
  pattern = clamp(pattern, 0, patterns.size()-1);
  return patterns[pattern].numberOfMeasures;
}

void PatternData::setBeatsPerMeasure(int pattern, int beats) {
  dirty = true;
  pattern = clamp(pattern, 0, patterns.size()-1);
  patterns[pattern].beatsPerMeasure = beats;

  for(auto& measure : patterns[pattern].measures) {
    if ((int)measure.steps.size() < getStepsPerMeasure(pattern)) {
      measure.steps.resize(getStepsPerMeasure(pattern));
    }
  }
}

void PatternData::setDivisionsPerBeat(int pattern, int divisions) {
  dirty = true;
  pattern = clamp(pattern, 0, patterns.size()-1);
  int previousSteps = getStepsPerMeasure(pattern);

  if (patterns[pattern].divisionsPerBeat != divisions) {
    patterns[pattern].divisionsPerBeat = divisions;
    reassignSteps(pattern, previousSteps, getStepsPerMeasure(pattern));
  }
}

int PatternData::getBeatsPerMeasure(int pattern) const {
  pattern = clamp(pattern, 0, patterns.size()-1);
  return patterns[pattern].beatsPerMeasure;
}

int PatternData::getDivisionsPerBeat(int pattern) const {
  pattern = clamp(pattern, 0, patterns.size()-1);
  return patterns[pattern].divisionsPerBeat;
}

void PatternData::copyPattern(int pattern) {
  pattern = clamp(pattern, 0, patterns.size()-1);

  copyPatternData(patterns[pattern], copiedPattern);
}

void PatternData::copyMeasure(int pattern, int measure) {
  pattern = clamp(pattern, 0, patterns.size()-1);
  measure = clamp(measure, 0, patterns[pattern].measures.size()-1);

  copyMeasureData(patterns[pattern].measures[measure], copiedMeasure);
}

void PatternData::pastePattern(int targetPattern) {
  dirty = true;
  targetPattern = clamp(targetPattern, 0, patterns.size()-1);

  copyPatternData(copiedPattern, patterns[targetPattern]);
}

void PatternData::pasteMeasure(int targetPattern, int targetMeasure) {
  dirty = true;
  targetPattern = clamp(targetPattern, 0, patterns.size()-1);
  targetMeasure = clamp(targetMeasure, 0, patterns[targetPattern].measures.size()-1);

  copyMeasureData(copiedMeasure, patterns[targetPattern].measures[targetMeasure]);
}

void PatternData::clearPatternSteps(int pattern) {
  dirty = true;
  pattern = clamp(pattern, 0, patterns.size()-1);

  for(auto& measure : patterns[pattern].measures) {
    for(auto& step : measure.steps) {
      step.active = false;
      step.retrigger = false;
    }
  }
}

void PatternData::reset() {
  dirty = true;
  size_t i;
  for (i = 0; i < patterns.size(); i++) {
    setMeasures(i, 1);
    setBeatsPerMeasure(i, 4);
    setDivisionsPerBeat(i, 4);
    clearPatternSteps(i);
  }
}

void PatternData::copyPatternData(const Pattern& sourcePattern, Pattern& targetPattern) {
  targetPattern.numberOfMeasures = sourcePattern.numberOfMeasures;
  targetPattern.beatsPerMeasure = sourcePattern.beatsPerMeasure;
  targetPattern.divisionsPerBeat = sourcePattern.divisionsPerBeat;
  targetPattern.measures.resize(sourcePattern.measures.size());

  int i = 0;
  for(const auto& measure : sourcePattern.measures) {
    copyMeasureData(measure, targetPattern.measures[i]);
    i += 1;
  }
}

void PatternData::copyMeasureData(const Measure& sourceMeasure, Measure& targetMeasure) {
  targetMeasure.steps.resize(sourceMeasure.steps.size());

  int i = 0;
  for(const auto& step : sourceMeasure.steps) {
    copyStepData(step, targetMeasure.steps[i]);
    i += 1;
  }
}

void PatternData::copyStepData(const Step& sourceStep, Step& targetStep) {
  targetStep.pitch = sourceStep.pitch;
  targetStep.velocity = sourceStep.velocity;
  targetStep.retrigger = sourceStep.retrigger;
  targetStep.active = sourceStep.active;
}

json_t *PatternData::toJson() const {
  json_t *patternsJ = json_array();
  int max = 0;
  for (int i = 0; i < (int)patterns.size(); i++) {
    for (int j = 0; j < (int)patterns[i].measures.size(); j++) {
      for (int k = 0; k < (int)patterns[i].measures[j].steps.size(); k++) {
        if (patterns[i].measures[j].steps[k].active) {
          max = i;
        }
      }
    }
  }

  int i = 0;
  for (const auto& pattern : patterns) {
    if (i > max) {
      break;
    }

    i++;

    json_t *patternJ = json_object();
    json_object_set_new(patternJ, "numberOfMeasures", json_integer(pattern.numberOfMeasures));
    json_object_set_new(patternJ, "beatsPerMeasure", json_integer(pattern.beatsPerMeasure));
    json_object_set_new(patternJ, "divisionsPerBeat", json_integer(pattern.divisionsPerBeat));

    json_t *measuresJ = json_array();
    for (const auto& measure : pattern.measures) {
      json_t *measureJ = json_object();
      json_t *notesJ = json_array();

      for (const auto& step : measure.steps) {
        json_t *noteJ = json_object();

        json_object_set_new(noteJ, "pitch", json_integer(step.pitch));
        json_object_set_new(noteJ, "velocity", json_real(step.velocity));
        json_object_set_new(noteJ, "retrigger", json_boolean(step.retrigger));
        json_object_set_new(noteJ, "active", json_boolean(step.active));
  
        json_array_append_new(notesJ, noteJ);
      }

      json_object_set_new(measureJ, "notes", notesJ);
      json_array_append_new(measuresJ, measureJ);
    }

    json_object_set_new(patternJ, "measures", measuresJ);
    json_array_append_new(patternsJ, patternJ);
  }

  return patternsJ;
}

void PatternData::fromJson(json_t *patternsJ) {
  dirty = true;
  reset();

  size_t i;
  json_t *patternJ;
  json_array_foreach(patternsJ, i, patternJ) {
    if (i >= patterns.size()) {
      continue;
    }
    json_t *numberOfMeasuresJ = json_object_get(patternJ, "numberOfMeasures");
    if (numberOfMeasuresJ) {
      setMeasures(i, json_integer_value(numberOfMeasuresJ));
    }

    json_t *beatsPerMeasureJ = json_object_get(patternJ, "beatsPerMeasure");
    if (beatsPerMeasureJ) {
      setBeatsPerMeasure(i, json_integer_value(beatsPerMeasureJ));
    }

    json_t *divisionsPerBeatJ = json_object_get(patternJ, "divisionsPerBeat");
    if (divisionsPerBeatJ) {
      setDivisionsPerBeat(i, json_integer_value(divisionsPerBeatJ));
    }

    json_t *measuresJ = json_object_get(patternJ, "measures");
    if (measuresJ) {
      size_t j;
      json_t *measureJ;
      json_array_foreach(measuresJ, j, measureJ) {
        
        if (patterns[i].measures.size() <= j) {
          patterns[i].measures.resize(j + 1);
        }

        json_t *notesJ = json_object_get(measureJ, "notes");
        if (notesJ) {
          size_t k;
          json_t *noteJ;
          json_array_foreach(notesJ, k, noteJ) {

            if (patterns[i].measures[j].steps.size() <= k) {
              patterns[i].measures[j].steps.resize(k + 1);
            }

            json_t *pitchJ = json_object_get(noteJ, "pitch");
            if (pitchJ) {
              info("Loading Pitch: %d/%d, %d/%d, %d/%d", i, patterns.size(), j, patterns[i].measures.size() ,k, patterns[i].measures[j].steps.size());
              patterns[i].measures[j].steps[k].pitch = json_integer_value(pitchJ);
            }

            json_t *velocityJ = json_object_get(noteJ, "velocity");
            if (velocityJ) {
              patterns[i].measures[j].steps[k].velocity = json_number_value(velocityJ);
            }

            json_t *retriggerJ = json_object_get(noteJ, "retrigger");
            if (retriggerJ) {
              patterns[i].measures[j].steps[k].retrigger = json_boolean_value(retriggerJ);
            }

            json_t *activeJ = json_object_get(noteJ, "active");
            if (activeJ) {
              patterns[i].measures[j].steps[k].active = json_boolean_value(activeJ);
            }
          }
        }
      }
    }
  }
}

void PatternData::reassignSteps(int pattern, int fromSteps, int toSteps) {
  dirty = true;
  pattern = clamp(pattern, 0, patterns.size()-1);

  float scale = (float)toSteps / (float)fromSteps;
  int smear = rack::max(1, (int)floor(scale));
  for (auto& measure: patterns[pattern].measures) {

    std::vector<Step> scratch;
    scratch.resize(toSteps);

    for (int i = 0; i < fromSteps; i++) {
      int newPos = floor(i * scale);
      if ((int)measure.steps.size() <= i) { 
        continue;
      }

      if (measure.steps[i].active) {
        // Copy note to new location
        // Keep it's gate length by smearing the note across multiple locations if necessary
        for (int n = 0; n < smear; n++) {
          bool retrigger = scratch[newPos].retrigger || (measure.steps[i].active && measure.steps[i].retrigger);
          copyStepData(measure.steps[i], scratch[newPos + n]);
          if (n > 0) {
            // don't retrigger smeared notes
            scratch[newPos + n].retrigger = false;
          } else {
            scratch[newPos + n].retrigger = retrigger;
          }
        }
      }
    }

    measure.steps.resize(toSteps);
    for (int i = 0; i < toSteps; i++) {
      copyStepData(scratch[i], measure.steps[i]);
    }
  }
}

float PatternData::adjustVelocity(int pattern, int measure, int step, float delta) {
  dirty = true;
  pattern = clamp(pattern, 0, patterns.size()-1);
  measure = clamp(measure, 0, patterns[pattern].measures.size()-1);
  step = clamp(step, 0, patterns[pattern].measures[measure].steps.size()-1);

  float velocity = 0.f;

  // find first note in the current group
  int pitch = patterns[pattern].measures[measure].steps[step].pitch;
  while (step > 0 && patterns[pattern].measures[measure].steps[step-1].active && patterns[pattern].measures[measure].steps[step-1].pitch == pitch) {
    if (patterns[pattern].measures[measure].steps[step].retrigger) {
      break;
    }

    step--;
  }

  // Adjust the velocity of that note
  velocity = clamp(patterns[pattern].measures[measure].steps[step].velocity + delta, 0.f, 1.f);

  // Apply new velocity to all notes in the group
  while (step < (int)patterns[pattern].measures[measure].steps.size() && patterns[pattern].measures[measure].steps[step].active && patterns[pattern].measures[measure].steps[step].pitch == pitch) {
    patterns[pattern].measures[measure].steps[step].velocity = velocity;
    step++;

    if (patterns[pattern].measures[measure].steps[step].retrigger) {
      break;
    }
  }

  return velocity;
}

void PatternData::toggleStepActive(int pattern, int measure, int step) {
  dirty = true;
  pattern = clamp(pattern, 0, patterns.size()-1);
  measure = clamp(measure, 0, patterns[pattern].measures.size()-1);
  step = clamp(step, 0, patterns[pattern].measures[measure].steps.size()-1);

  if (!patterns[pattern].measures[measure].steps[step].active) {
    patterns[pattern].measures[measure].steps[step].active = true;
    patterns[pattern].measures[measure].steps[step].velocity = 0.75f;
  } else {
    patterns[pattern].measures[measure].steps[step].active = false;
    patterns[pattern].measures[measure].steps[step].retrigger = false;
  }

  // match the velocity of the new note to any other notes in the group
  adjustVelocity(pattern, measure, step, 0.f);
}

void PatternData::setStepActive(int pattern, int measure, int step, bool active) {
  dirty = true;
  pattern = clamp(pattern, 0, patterns.size()-1);
  measure = clamp(measure, 0, patterns[pattern].measures.size()-1);
  step = clamp(step, 0, patterns[pattern].measures[measure].steps.size()-1);

  patterns[pattern].measures[measure].steps[step].active = active;

  // match the velocity of the new note to any other notes in the group
  adjustVelocity(pattern, measure, step, 0.f);
}

void PatternData::toggleStepRetrigger(int pattern, int measure, int step) {
  dirty = true;
  pattern = clamp(pattern, 0, patterns.size()-1);
  measure = clamp(measure, 0, patterns[pattern].measures.size()-1);
  step = clamp(step, 0, patterns[pattern].measures[measure].steps.size()-1);

  if (!patterns[pattern].measures[measure].steps[step].active) {
    return;
  }

  patterns[pattern].measures[measure].steps[step].retrigger = !patterns[pattern].measures[measure].steps[step].retrigger;
  // match the velocity of the new note to any other notes in the group
  adjustVelocity(pattern, measure, step, 0.f);
}

bool PatternData::isStepActive(int pattern, int measure, int step) const {
  pattern = clamp(pattern, 0, patterns.size()-1);
  measure = clamp(measure, 0, patterns[pattern].measures.size()-1);
  step = clamp(step, 0, patterns[pattern].measures[measure].steps.size()-1);

  return patterns[pattern].measures[measure].steps[step].active;
}

bool PatternData::isStepRetriggered(int pattern, int measure, int step) const {
  pattern = clamp(pattern, 0, patterns.size()-1);
  measure = clamp(measure, 0, patterns[pattern].measures.size()-1);
  step = clamp(step, 0, patterns[pattern].measures[measure].steps.size()-1);

  return patterns[pattern].measures[measure].steps[step].retrigger;
}

void PatternData::setStepRetrigger(int pattern, int measure, int step, bool retrigger) {
  dirty = true;
  pattern = clamp(pattern, 0, patterns.size()-1);
  measure = clamp(measure, 0, patterns[pattern].measures.size()-1);
  step = clamp(step, 0, patterns[pattern].measures[measure].steps.size()-1);

  patterns[pattern].measures[measure].steps[step].retrigger = retrigger;
}

float PatternData::getStepVelocity(int pattern, int measure, int step) const {
  pattern = clamp(pattern, 0, patterns.size()-1);
  measure = clamp(measure, 0, patterns[pattern].measures.size()-1);
  step = clamp(step, 0, patterns[pattern].measures[measure].steps.size()-1);

  return patterns[pattern].measures[measure].steps[step].velocity;
}

void PatternData::increaseStepVelocityTo(int pattern, int measure, int step, float velocity) {
  dirty = true;
  pattern = clamp(pattern, 0, patterns.size()-1);
  measure = clamp(measure, 0, patterns[pattern].measures.size()-1);
  step = clamp(step, 0, patterns[pattern].measures[measure].steps.size()-1);

  patterns[pattern].measures[measure].steps[step].velocity = max(patterns[pattern].measures[measure].steps[step].velocity, velocity);
}

void PatternData::setStepVelocity(int pattern, int measure, int step, float velocity) {
  dirty = true;
  pattern = clamp(pattern, 0, patterns.size()-1);
  measure = clamp(measure, 0, patterns[pattern].measures.size()-1);
  step = clamp(step, 0, patterns[pattern].measures[measure].steps.size()-1);

  patterns[pattern].measures[measure].steps[step].velocity = velocity;
}

int PatternData::getStepPitch(int pattern, int measure, int step) const {
  pattern = clamp(pattern, 0, patterns.size()-1);
  measure = clamp(measure, 0, patterns[pattern].measures.size()-1);
  step = clamp(step, 0, patterns[pattern].measures[measure].steps.size()-1);

  return patterns[pattern].measures[measure].steps[step].pitch;
}

void PatternData::setStepPitch(int pattern, int measure, int step, int pitch) {
  dirty = true;
  pattern = clamp(pattern, 0, patterns.size()-1);
  measure = clamp(measure, 0, patterns[pattern].measures.size()-1);
  step = clamp(step, 0, patterns[pattern].measures[measure].steps.size()-1);

  patterns[pattern].measures[measure].steps[step].pitch = pitch;
}

bool PatternData::consumeDirty() {
  bool wasdirty = dirty;
  dirty = false;
  return wasdirty;
}

} // namespace rack_plugin_rcm


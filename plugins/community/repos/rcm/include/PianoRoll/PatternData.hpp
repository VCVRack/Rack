#include "rack.hpp"
#include <vector>

namespace rack_plugin_rcm {

struct Step;

class PatternData {
public:
  PatternData();

  int getStepsInPattern(int pattern) const;
  int getStepsPerMeasure(int pattern) const;

  void setMeasures(int pattern, int measures);
  int getMeasures(int pattern) const;

  void setBeatsPerMeasure(int pattern, int beats);
  void setDivisionsPerBeat(int pattern, int divisions);

  int getBeatsPerMeasure(int pattern) const;
  int getDivisionsPerBeat(int pattern) const;

  void copyPattern(int pattern);
  void copyMeasure(int pattern, int measure);
  void pastePattern(int targetPattern);
  void pasteMeasure(int targetPattern, int targetMeasure);

	void toggleStepActive(int pattern, int measure, int step);
	void setStepActive(int pattern, int measure, int step, bool active);
  void setStepPitch(int pattern, int measure, int step, int pitch);
	void toggleStepRetrigger(int pattern, int measure, int step);
	void setStepRetrigger(int pattern, int measure, int step, bool retrigger);
  void setStepVelocity(int pattern, int measure, int step, float velocity);
  void increaseStepVelocityTo(int pattern, int measure, int step, float targetVelocity);

  bool isStepActive(int pattern, int measure, int step) const;
  bool isStepRetriggered(int pattern, int measure, int step) const;
  float getStepVelocity(int pattern, int measure, int step) const;
  int getStepPitch(int pattern, int measure, int step) const;

	float adjustVelocity(int pattern, int measure, int step, float delta);

  void clearPatternSteps(int pattern);
  void reset();

  json_t *toJson() const;
  void fromJson(json_t *rootJ);

  bool dirty = true;
  bool consumeDirty();

private:
  struct Step {
    int pitch = 0;
    float velocity = 0.f;
    bool retrigger = false;
    bool active = false;
  };

  struct Measure {
    std::vector<Step> steps;
  };

  struct Pattern {
    std::vector<Measure> measures;
    int numberOfMeasures = 1;
    int beatsPerMeasure = 4;
    int divisionsPerBeat = 4;
  };

  std::vector<Pattern> patterns;

  void copyPatternData(const Pattern& sourcePattern, Pattern& targetPattern);
  void copyMeasureData(const Measure& sourceMeasure, Measure& targetMeasure);
  void copyStepData(const Step& sourceStep, Step& targetStep);

	void reassignSteps(int pattern, int fromSteps, int toSteps);

  Pattern copiedPattern;
  Measure copiedMeasure;
};

} // namespace rack_plugin_rcm

namespace rack_plugin_rcm {

struct PatternData;


class Transport {
public:
  Transport(PatternData* data);

  int currentPattern();
  int currentMeasure();
  int currentStepInMeasure();
  int currentStepInPattern();
  bool isLastStepOfPattern();

  void setPattern(int pattern);
  void setMeasure(int measure);
  void setStepInMeasure(int step);
  void setStepInPattern(int step);

  void advancePattern(int offset);
  void advanceStep();

  void lockMeasure();
  void unlockMeasure();
  bool isLocked();

  void toggleRun();
  void setRun(bool running);
  bool isRunning();

  void toggleRecording();
  bool isRecording();
  bool isPendingRecording();

  void reset();

  bool dirty = true;
  bool consumeDirty();

private:
  int pattern = 0;
  int stepInPattern = -1;

  bool locked = false;
  bool running = true;
  bool recording = false;
  bool pendingRecording = false;

  PatternData* patternData;
};

} // namespace rack_plugin_rcm

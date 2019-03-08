
namespace rack_plugin_rcm {

class Auditioner {
public:
  void start(int step);
  void retrigger();
  void stop();

  bool isAuditioning();
  int stepToAudition();
  bool consumeRetrigger();
  bool consumeStopEvent();

private:
  int step = -1;
  bool needsRetrigger = false;
  bool auditioning = false;
  bool stopPending = false;
};

} // namespace rack_plugin_rcm

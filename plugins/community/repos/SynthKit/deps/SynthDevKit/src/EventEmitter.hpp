#pragma once

#include <cstdint>
#include <vector>

namespace SynthDevKit {
  enum EETypes {
    EVENT_CLEAR = -10,
    EVENT_RESET,
    EVENT_FIRST,
    EVENT_EVEN,
    EVENT_ODD,
    EVENT_MAX = 2048
  };

  const uint16_t TOP_EVENT = EVENT_MAX + 24;

  class EventEmitter {
  public:
    EventEmitter ( );
    void clear (bool all = false);
    void on (int16_t, void (*)(int16_t, float));
    void removeListener (int16_t, void(*)(int16_t, float));
    int16_t listenerCount (int16_t);
    void emit (int16_t, float);
  protected:
    bool has_emitted;
  private:
    int16_t realEvent (int16_t);
    std::vector<void (*)(int16_t, float)> emitters[TOP_EVENT];
  };
}

#pragma once

#include "CV.hpp"
#include "EventEmitter.hpp"

namespace SynthDevKit {
  class CVEvent : public CV, public EventEmitter {
  public:
    CVEvent (float, int16_t);
    void update (float);
    void reset ( );
  private:
    int16_t count;
  };
}

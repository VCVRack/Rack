#pragma once

#include <cstdint>
#include "CV.hpp"

namespace SynthDevKit {
  class DistributedClock {
    public:
      DistributedClock (bool, float);
      void update (uint8_t, float, float);
      void reset ( );
      bool triggerClock ( );
      bool triggerDownstream ( );
    private:
      CV *upstreamCV;
      CV *clockCV;
      bool isMaster;
      bool isActive;
      uint8_t triggerCount;
      uint8_t totalTriggers;
  };
}

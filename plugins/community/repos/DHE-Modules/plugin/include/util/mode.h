#pragma once

namespace DHE {
class Mode {
public:
  virtual void enter(){};
  virtual void step(){};
  virtual void exit(){};
};
} // namespace DHE

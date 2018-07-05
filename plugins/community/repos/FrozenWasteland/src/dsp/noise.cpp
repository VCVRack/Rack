
#include "noise.hpp"

using namespace frozenwasteland::dsp;


Seeds::Seeds() {
  std::random_device rd;
  _generator.seed(rd());
}

unsigned int Seeds::_next() {
  return _generator();
}

Seeds& Seeds::getInstance() {
  static Seeds instance;
  return instance;
}

unsigned int Seeds::next() {
  return getInstance()._next();
};

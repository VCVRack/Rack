#include "cubefader.hpp"
#include <iostream>
#include "util/math.hpp"
#include "rack.hpp"

namespace rack_plugin_CastleRocktronics {

void Cubefader::step() {
  float x, y, z, crossFaded;

  if (params[UNI_BI_TOGGLE].value <= 0.5f) {
    x = rescaleInputBiPolar(inputs[X_CV], params[X_TRIMPOT]);
    y = rescaleInputBiPolar(inputs[Y_CV], params[Y_TRIMPOT]);
    z = rescaleInputBiPolar(inputs[Z_CV], params[Z_TRIMPOT]);
  } else {
    x = rescaleInputUniPolar(inputs[X_CV], params[X_TRIMPOT]);
    y = rescaleInputUniPolar(inputs[Y_CV], params[Y_TRIMPOT]);
    z = rescaleInputUniPolar(inputs[Z_CV], params[Z_TRIMPOT]);
  }

  crossFaded = trilinear(
      inputs[INPUT_000].value, inputs[INPUT_100].value, inputs[INPUT_010].value,
      inputs[INPUT_110].value, inputs[INPUT_001].value, inputs[INPUT_101].value,
      inputs[INPUT_011].value, inputs[INPUT_111].value, x, y, z);

  outputs[OUTPUT].value = antiAlias(crossFaded);
};

float Cubefader::linear(float x0, float x1, float xDist) {
  return (x1 * xDist) + (x0 * (1 - xDist));
};

float Cubefader::bilinear(float xy00, float xy10, float xy01, float xy11,
                          float x, float y) {
  return linear(linear(xy01, xy11, x), linear(xy00, xy10, x), y);
};

float Cubefader::trilinear(float xyz000, float xyz100, float xyz010,
                           float xyz110, float xyz001, float xyz101,
                           float xyz011, float xyz111, float x, float y,
                           float z) {
  return linear(bilinear(xyz000, xyz100, xyz010, xyz110, x, y),
                bilinear(xyz001, xyz101, xyz011, xyz111, x, y), z);
};

float Cubefader::antiAlias(float input) {
  z_1 = (z_1 * aaFactor) + ((1 - aaFactor) * input);
  z_2 = (z_2 * aaFactor) + ((1 - aaFactor) * z_1);
  return z_2;
};

float Cubefader::rescaleInputUniPolar(Input input, Param trim) {
  float scaled = rescalef(input.value * trim.value, 0.0f, 10.0f, 0.0f, 1.0f);
  return clampf(scaled, 0.0f, 1.0f);
};

float Cubefader::rescaleInputBiPolar(Input input, Param trim) {
  float scaled = rescalef(input.value * trim.value, -10.0f, 10.0f, 0.0f, 1.0f);
  return clampf(scaled, 0.0f, 1.0f);
};

} // namespace rack_plugin_CastleRocktronics

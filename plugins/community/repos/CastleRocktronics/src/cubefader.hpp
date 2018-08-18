#include "common.hpp"

using namespace rack;

namespace rack_plugin_CastleRocktronics {

struct Cubefader : Module {
  float z_1 = 0.0f;
  float z_2 = 0.0f;
  float aaFactor = 0.5f;

  enum Params { X_TRIMPOT, Y_TRIMPOT, Z_TRIMPOT, UNI_BI_TOGGLE, NUM_PARAMS };

  enum Inputs {
    INPUT_000,
    INPUT_100,
    INPUT_010,
    INPUT_110,
    INPUT_001,
    INPUT_101,
    INPUT_011,
    INPUT_111,
    X_CV,
    Y_CV,
    Z_CV,
    NUM_INPUTS
  };

  enum Outputs { OUTPUT, NUM_OUTPUTS };

  Cubefader() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, 0) {}

  void step() override;

 private:
  float linear(float x0, float x1, float xDist);
  float bilinear(float xy00, float xy10, float xy01, float xy11, float x,
                 float y);
  float trilinear(float xyz000, float xyz100, float xyz010, float xyz110,
                  float xyz001, float xyz101, float xyz011, float xyz111,
                  float x, float y, float z);

  float antiAlias(float input);

  float rescaleInputUniPolar(Input input, Param trim);
  float rescaleInputBiPolar(Input input, Param trim);
};

} // namespace rack_plugin_CastleRocktronics

using namespace rack_plugin_CastleRocktronics;

#include "cubefader.hpp"
#include "gui_components.hpp"

namespace rack_plugin_CastleRocktronics {

struct CubefaderWidget : ModuleWidget {
  CubefaderWidget(Cubefader *module);

 private:
  int moduleWidth;

  void placeGuiElements();
  void placeAudioInputs(int x, int y, int verticalSpacing);
  void placeCvInputs(int x, int y, int horizontalSpacing);
  void placeOutput(int x, int y);
  void placeSlider(int x, int y);
  void placeTrimpots(int x, int y, int horizontalSpacing);
  void placeScrews();
};

CubefaderWidget::CubefaderWidget(Cubefader *module) : ModuleWidget(module) {
  this->moduleWidth = 12.0f * RACK_GRID_WIDTH;

  box.size = Vec(moduleWidth, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(
        SVG::load(assetPlugin(plugin, "res/panels/cubefader.svg")));
    addChild(panel);
  }

  placeGuiElements();
};

void CubefaderWidget::placeGuiElements() {
  int y = (RACK_GRID_HEIGHT / 4) * 3 - 30;
  int x = (moduleWidth / 3) - 34;
  int horizontalSpacing = 49;
  placeCvInputs(x, y, horizontalSpacing);

  placeOutput(x + (horizontalSpacing * 2) + 1, y + horizontalSpacing + 3);

  addParam(createParam<DelvinToggleTwo>(
      Vec(x + 7, y + 60), module, Cubefader::UNI_BI_TOGGLE, 0.0f, 1.0f, 1.0f));

  placeTrimpots(x + 4, y - 24, horizontalSpacing);

  x = (moduleWidth / 6) - 20;
  y = (RACK_GRID_HEIGHT / 4) - 9;
  int verticalSpacing = 89;
  placeAudioInputs(x, y, verticalSpacing);

  placeScrews();
}

void CubefaderWidget::placeAudioInputs(int startingX, int startingY,
                                       int spacing) {
  for (int i = 0; i != Cubefader::INPUT_001; i += 2) {
    Cubefader::Inputs inputLeft = static_cast<Cubefader::Inputs>(i);
    Cubefader::Inputs inputRight = static_cast<Cubefader::Inputs>(i + 1);
    int y = startingY + (spacing * (i / 2));

    addInput(
        createInput<SevenHalfKnurled>(Vec(startingX, y), module, inputLeft));
    addInput(createInput<SevenHalfKnurled>(Vec(startingX + spacing, y), module,
                                           inputRight));
  }

  for (int i = Cubefader::INPUT_001; i != Cubefader::X_CV; i += 2) {
    Cubefader::Inputs inputLeft = static_cast<Cubefader::Inputs>(i);
    Cubefader::Inputs inputRight = static_cast<Cubefader::Inputs>(i + 1);
    int x = startingX + (spacing / 2);
    int y = startingY + (spacing * ((i - 4) / 2)) - (spacing / 3);

    addInput(createInput<SevenHalfKnurled>(Vec(x, y), module, inputLeft));
    addInput(
        createInput<SevenHalfKnurled>(Vec(x + spacing, y), module, inputRight));
  }
}

void CubefaderWidget::placeOutput(int x, int y) {
  addOutput(
      createOutput<SevenHalfKnurled>(Vec(x, y), module, Cubefader::OUTPUT));
}

void CubefaderWidget::placeCvInputs(int x, int y, int horizontalSpacing) {
  addInput(createInput<SevenHalfKnurled>(Vec(x, y), module, Cubefader::X_CV));
  addInput(createInput<SevenHalfKnurled>(Vec(x + horizontalSpacing, y), module,
                                         Cubefader::Y_CV));
  addInput(createInput<SevenHalfKnurled>(
      Vec(x + (horizontalSpacing * 2) + 1, y), module, Cubefader::Z_CV));
}

void CubefaderWidget::placeTrimpots(int x, int y, int horizontalSpacing) {
  addParam(createParam<CastleTrimpot>(Vec(x, y), module, Cubefader::X_TRIMPOT,
                                      -2.0f, 2.0f, 1.0f));
  addParam(createParam<CastleTrimpot>(Vec(x + horizontalSpacing, y), module,
                                      Cubefader::Y_TRIMPOT, -2.0f, 2.0f, 1.0f));
  addParam(createParam<CastleTrimpot>(Vec(x + (horizontalSpacing * 2) + 1, y),
                                      module, Cubefader::Z_TRIMPOT, -2.0f, 2.0f,
                                      1.0f));
}

void CubefaderWidget::placeScrews() {
  addChild(createScrew<MThreeScrew>(Vec(RACK_GRID_WIDTH + 1, 2)));
  addChild(
      createScrew<MThreeScrew>(Vec((box.size.x - 2 * RACK_GRID_WIDTH) + 1, 2)));
  addChild(createScrew<MThreeScrew>(
      Vec(RACK_GRID_WIDTH + 1, RACK_GRID_HEIGHT - RACK_GRID_WIDTH + 1)));
  addChild(
      createScrew<MThreeScrew>(Vec((box.size.x - 2 * RACK_GRID_WIDTH) + 1,
                                   RACK_GRID_HEIGHT - RACK_GRID_WIDTH + 1)));
}

} // namespace rack_plugin_CastleRocktronics

using namespace rack_plugin_CastleRocktronics;

RACK_PLUGIN_MODEL_INIT(CastleRocktronics, Cubefader) {
   // p->addModel(createModel<CubefaderWidget>("CastleRocktronics",
   //                                          "CR-V01_Cubefader", "Cubefader",
   //                                          UTILITY_TAG, MIXER_TAG));
   return Model::create<Cubefader, CubefaderWidget>("CastleRocktronics",
                                                    "CR-V01_Cubefader", "Cubefader",
                                                    UTILITY_TAG, MIXER_TAG);
}

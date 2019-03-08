#include "Plateau.hpp"

Plateau::Plateau() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
    reverb.setSampleRate(engineGetSampleRate());
    wet = 0.5f;
    dry = 1.f;
    preDelay = 0.f;
    preDelayCVSens = preDelayNormSens;
    size = 1.f;
    diffusion = 1.f;
    decay = 0.f;
    inputDampLow = 0.f;
    inputDampHigh = 10.f;
    reverbDampLow = 0.f;
    reverbDampHigh = 10.f;
    modSpeed = 0.1f;
    modShape = 0.5f;
    modDepth = 0.0f;

    freezeButtonState = false;
    freezeToggle = false;
    freezeToggleButtonState = false;
    freeze = false;
    frozen = false;
    tunedButtonState = false;
    diffuseButtonState = false;
    preDelayCVSensState = 0;
    inputSensitivityState = 0;
    outputSaturationState = 0;

    clear = 0;
    cleared = false;
    tuned = 0;
    diffuseInput = 1;

    leftInput = 0.f;
    rightInput = 0.f;
}

void Plateau::step() {
    //Freeze
    if(params[FREEZE_TOGGLE_PARAM].value > 0.5f && !freezeToggleButtonState) {
        freezeToggleButtonState = true;
        freezeToggle = !freezeToggle;
    }
    else if(params[FREEZE_TOGGLE_PARAM].value < 0.5f && freezeToggleButtonState) {
        freezeToggleButtonState = false;
    }
    lights[FREEZE_TOGGLE_LIGHT].value = freezeToggle ? 10.f : 0.f;

    if((params[FREEZE_PARAM].value > 0.5f || inputs[FREEZE_CV_INPUT].value > 0.5f)
    && !freezeButtonState) {
        freeze = freezeToggle ? !freeze : true;
        freezeButtonState = true;
    }
    if(params[FREEZE_PARAM].value <= 0.5f && inputs[FREEZE_CV_INPUT].value <= 0.5f
    && freezeButtonState) {
        freeze = freezeToggle ? freeze : false;
        freezeButtonState = false;
    }

    if(freeze && !frozen) {
        frozen = true;
        reverb.freeze();
    }
    else if(!freeze && frozen){
        frozen = false;
        reverb.unFreeze();
    }
    lights[FREEZE_LIGHT].value = freeze ? 10.f : 0.f;

    // Clear
    if(params[CLEAR_PARAM].value > 0.5f || inputs[CLEAR_CV_INPUT].value > 0.5f) {
        clear = 1;
    }
    else if(params[CLEAR_PARAM].value < 0.5f && inputs[CLEAR_CV_INPUT].value < 0.5f) {
        clear = 0;
    }

    if(params[TUNED_MODE_PARAM].value > 0.5f && tunedButtonState == false) {
        tuned = 1 - tuned;
        tunedButtonState = true;
    }
    else if(params[TUNED_MODE_PARAM].value < 0.5f && tunedButtonState) {
        tunedButtonState = false;
    }
    lights[TUNED_MODE_LIGHT].value = tuned ? 10.f : 0.f;

    if(params[DIFFUSE_INPUT_PARAM].value > 0.5f && diffuseButtonState == false) {
        diffuseInput = 1 - diffuseInput;
        diffuseButtonState = true;
    }
    else if(params[DIFFUSE_INPUT_PARAM].value < 0.5f && diffuseButtonState) {
        diffuseButtonState = false;
    }
    lights[DIFFUSE_INPUT_LIGHT].value = diffuseInput ? 10.f : 0.f;

    if(clear && !cleared) {
        cleared = true;
        reverb.clear();
        lights[CLEAR_LIGHT].value = 10.f;
    }
    else if(!clear && cleared){
        cleared = false;
        lights[CLEAR_LIGHT].value = 0.f;
    }

    // CV
    switch(preDelayCVSensState) {
        case 0: preDelayCVSens = preDelayNormSens; break;
        case 1: preDelayCVSens = preDelayLowSens;
    }
    preDelay = params[PRE_DELAY_PARAM].value;
    preDelay += 0.5f * (powf(2.f, inputs[PRE_DELAY_CV_INPUT].value * preDelayCVSens) - 1.f);
    reverb.setPreDelay(clamp(preDelay, 0.f, 1.f));

    size = inputs[SIZE_CV_INPUT].value * params[SIZE_CV_PARAM].value * 0.1f;
    size += params[SIZE_PARAM].value;
    if(tuned) {
        size = sizeMin * powf(2.f, size * 5.f);
        size = clamp(size, sizeMin, 2.5f);
    }
    else {
        size *= size;
        size = rescale(size, 0.f, 1.f, 0.01f, sizeMax);
        size = clamp(size, 0.01f, sizeMax);
    }
    reverb.setTimeScale(size);

    diffusion = inputs[DIFFUSION_CV_INPUT].value * params[DIFFUSION_CV_PARAM].value;
    diffusion += params[DIFFUSION_PARAM].value;
    diffusion = clamp(diffusion, 0.f, 10.f);
    reverb.plateDiffusion1 = rescale(diffusion, 0.f, 10.f, 0.f, 0.7f);
    reverb.plateDiffusion2 = rescale(diffusion, 0.f, 10.f, 0.f, 0.5f);

    decay = rescale(inputs[DECAY_CV_INPUT].value * params[DECAY_CV_PARAM].value, 0.f, 10.f, 0.1f, 0.999f);
    decay += params[DECAY_PARAM].value;
    decay = clamp(decay, 0.1f, decayMax);
    decay = 1.f - decay;
    decay = 1.f - decay * decay;

    inputDampLow = inputs[INPUT_LOW_DAMP_CV_INPUT].value * params[INPUT_LOW_DAMP_CV_PARAM].value;
    inputDampLow += params[INPUT_LOW_DAMP_PARAM].value;
    inputDampLow = clamp(inputDampLow, 0.f, 10.f);
    inputDampLow = 10.f - inputDampLow;

    inputDampHigh = inputs[INPUT_HIGH_DAMP_CV_INPUT].value * params[INPUT_HIGH_DAMP_CV_PARAM].value;
    inputDampHigh += params[INPUT_HIGH_DAMP_PARAM].value;
    inputDampHigh = clamp(inputDampHigh, 0.f, 10.f);

    reverbDampLow = inputs[REVERB_LOW_DAMP_CV_INPUT].value * params[REVERB_LOW_DAMP_CV_PARAM].value;
    reverbDampLow += params[REVERB_LOW_DAMP_PARAM].value;
    reverbDampLow = clamp(reverbDampLow, 0.f, 10.f);
    reverbDampLow = 10.f - reverbDampLow;

    reverbDampHigh = inputs[REVERB_HIGH_DAMP_CV_INPUT].value * params[REVERB_HIGH_DAMP_CV_PARAM].value;
    reverbDampHigh += params[REVERB_HIGH_DAMP_PARAM].value;
    reverbDampHigh = clamp(reverbDampHigh, 0.f, 10.f);

    reverb.diffuseInput = (double)diffuseInput;

    reverb.decay = decay;
    reverb.inputLowCut = 440.f * powf(2.f, inputDampLow - 5.f);
    reverb.inputHighCut = 440.f * powf(2.f, inputDampHigh - 5.f);
    reverb.reverbLowCut = 440.f * powf(2.f, reverbDampLow - 5.f);
    reverb.reverbHighCut = 440.f * powf(2.f, reverbDampHigh - 5.f);

    modSpeed = inputs[MOD_SPEED_CV_INPUT].value * params[MOD_SPEED_CV_PARAM].value * 0.1f;
    modSpeed += params[MOD_SPEED_PARAM].value;
    modSpeed = clamp(modSpeed, modSpeedMin, modSpeedMax);
    modSpeed *= modSpeed;
    modSpeed = modSpeed * 99.f + 1.f;

    modShape = inputs[MOD_SHAPE_CV_INPUT].value * params[MOD_SHAPE_CV_PARAM].value * 0.1f;
    modShape += params[MOD_SHAPE_PARAM].value;
    modShape = rescale(modShape, 0.f, 1.f, modShapeMin, modShapeMax);
    modShape = clamp(modShape, modShapeMin, modShapeMax);

    modDepth = inputs[MOD_DEPTH_CV_INPUT].value * params[MOD_DEPTH_CV_PARAM].value;
    modDepth = rescale(modDepth, 0.f, 10.f, modDepthMin, modDepthMax);
    modDepth += params[MOD_DEPTH_PARAM].value;
    modDepth = clamp(modDepth, modDepthMin, modDepthMax);

    reverb.modSpeed = modSpeed;
    reverb.modDepth = modDepth;
    reverb.setModShape(modShape);

    leftInput = inputs[LEFT_INPUT].value;
    rightInput = inputs[RIGHT_INPUT].value;
    if(inputs[LEFT_INPUT].active == false && inputs[RIGHT_INPUT].active == true) {
        leftInput = inputs[RIGHT_INPUT].value;
    }
    else if(inputs[LEFT_INPUT].active == true && inputs[RIGHT_INPUT].active == false) {
        rightInput = inputs[LEFT_INPUT].value;
    }

    inputSensitivity = inputSensitivityState ? 0.125893f : 1.f;
    reverb.process(leftInput * 0.1f * inputSensitivity, rightInput * 0.1f * inputSensitivity);

    dry = inputs[DRY_CV_INPUT].value * params[DRY_CV_PARAM].value;
    dry += params[DRY_PARAM].value;
    dry = clamp(dry, 0.f, 1.f);

    wet = inputs[WET_CV_INPUT].value * params[WET_CV_PARAM].value;
    wet += params[WET_PARAM].value;
    wet = clamp(wet, 0.f, 1.f) * 10.f;

    outputs[LEFT_OUTPUT].value = leftInput * dry;
    outputs[RIGHT_OUTPUT].value = rightInput * dry;
    outputs[LEFT_OUTPUT].value += reverb.leftOut * wet;
    outputs[RIGHT_OUTPUT].value += reverb.rightOut * wet;

    if(outputSaturationState) {
        outputs[LEFT_OUTPUT].value = tanhDriveSignal(outputs[LEFT_OUTPUT].value * 0.111f, 0.95f) * 9.999f;
        outputs[RIGHT_OUTPUT].value = tanhDriveSignal(outputs[RIGHT_OUTPUT].value * 0.111f, 0.95f) * 9.999f;
    }
}

void Plateau::onSampleRateChange() {
    reverb.setSampleRate(engineGetSampleRate());
}

void Plateau::reset() {
    diffuseInput = 1;
}

json_t* Plateau::toJson()  {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "frozen", json_boolean(freeze));
    json_object_set_new(rootJ, "freezeToggle", json_boolean(freezeToggle));
    json_object_set_new(rootJ, "panelStyle", json_integer(panelStyle));
    json_object_set_new(rootJ, "tuned", json_integer((int)tuned));
    json_object_set_new(rootJ, "diffuseInput", json_integer((int)diffuseInput));
    json_object_set_new(rootJ, "preDelayCVSens", json_integer((int)preDelayCVSensState));
    json_object_set_new(rootJ, "inputSensitivity", json_integer((int)inputSensitivityState));
    json_object_set_new(rootJ, "outputSaturation", json_integer((int)outputSaturationState));
    return rootJ;
}

void Plateau::fromJson(json_t *rootJ) {
    json_t *frozenJ = json_object_get(rootJ, "frozen");
    freeze = json_boolean_value(frozenJ);

    json_t *freezeToggleJ = json_object_get(rootJ, "freezeToggle");
    freezeToggle = json_boolean_value(freezeToggleJ);

    json_t *panelStyleJ = json_object_get(rootJ, "panelStyle");
    panelStyle = json_integer_value(panelStyleJ);

    json_t *tunedJ = json_object_get(rootJ, "tuned");
    tuned = json_integer_value(tunedJ);

    json_t *diffuseInputJ = json_object_get(rootJ, "diffuseInput");
    diffuseInput = json_integer_value(diffuseInputJ);

    json_t *preDelayCVSensJ = json_object_get(rootJ, "preDelayCVSens");
    preDelayCVSensState = json_integer_value(preDelayCVSensJ);

    json_t *inputSensitivityJ = json_object_get(rootJ, "inputSensitivity");
    inputSensitivityState = json_integer_value(inputSensitivityJ);

    json_t *outputSaturationJ = json_object_get(rootJ, "outputSaturation");
    outputSaturationState = json_integer_value(outputSaturationJ);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void PlateauPanelStyleItem::onAction(EventAction &e) {
    module->panelStyle = panelStyle;
}

void PlateauPanelStyleItem::step() {
    rightText = (module->panelStyle == panelStyle) ? "✔" : "";
    MenuItem::step();
}

void PlateauPreDelayCVSensItem::onAction(EventAction &e) {
    module->preDelayCVSensState = preDelayCVSensState;
}

void PlateauPreDelayCVSensItem::step() {
    rightText = (module->preDelayCVSensState == preDelayCVSensState) ? "✔" : "";
    MenuItem::step();
}

void PlateauInputSensItem::onAction(EventAction &e) {
    module->inputSensitivityState = inputSensitivityState;
}

void PlateauInputSensItem::step() {
    rightText = (module->inputSensitivityState == inputSensitivityState) ? "✔" : "";
    MenuItem::step();
}

void PlateauOutputSaturationItem::onAction(EventAction &e) {
    module->outputSaturationState = outputSaturationState;
}

void PlateauOutputSaturationItem::step() {
    rightText = (module->outputSaturationState == outputSaturationState) ? "✔" : "";
    MenuItem::step();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

PlateauWidget::PlateauWidget(Plateau* module) : ModuleWidget(module) {
    {
        DynamicPanelWidget *panel = new DynamicPanelWidget();
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/PlateauPanelDark.svg")));
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/PlateauPanelLight.svg")));
        box.size = panel->box.size;
        panel->mode = &module->panelStyle;
        addChild(panel);
    }
    addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    // Make jacks
    addInput(Port::create<PJ301MDarkSmall>(module->leftInputPos, Port::INPUT, module, Plateau::LEFT_INPUT));
    addInput(Port::create<PJ301MDarkSmall>(module->rightInputPos, Port::INPUT, module, Plateau::RIGHT_INPUT));
    addInput(Port::create<PJ301MDarkSmall>(module->dryCVPos, Port::INPUT, module, Plateau::DRY_CV_INPUT));
    addInput(Port::create<PJ301MDarkSmall>(module->wetCVPos, Port::INPUT, module, Plateau::WET_CV_INPUT));
    addInput(Port::create<PJ301MDarkSmall>(module->preDelayCVPos, Port::INPUT, module, Plateau::PRE_DELAY_CV_INPUT));
    addInput(Port::create<PJ301MDarkSmall>(module->inputLowDampCVPos, Port::INPUT, module, Plateau::INPUT_LOW_DAMP_CV_INPUT));
    addInput(Port::create<PJ301MDarkSmall>(module->inputHighDampCVPos, Port::INPUT, module, Plateau::INPUT_HIGH_DAMP_CV_INPUT));

    addInput(Port::create<PJ301MDarkSmall>(module->sizeCVPos, Port::INPUT, module, Plateau::SIZE_CV_INPUT));
    addInput(Port::create<PJ301MDarkSmall>(module->diffCVPos, Port::INPUT, module, Plateau::DIFFUSION_CV_INPUT));
    addInput(Port::create<PJ301MDarkSmall>(module->decayCVPos, Port::INPUT, module, Plateau::DECAY_CV_INPUT));
    addInput(Port::create<PJ301MDarkSmall>(module->reverbLowDampCVPos, Port::INPUT, module, Plateau::REVERB_LOW_DAMP_CV_INPUT));
    addInput(Port::create<PJ301MDarkSmall>(module->reverbHighDampCVPos, Port::INPUT, module, Plateau::REVERB_HIGH_DAMP_CV_INPUT));

    addInput(Port::create<PJ301MDarkSmall>(module->modRateCVPos, Port::INPUT, module, Plateau::MOD_SPEED_CV_INPUT));
    addInput(Port::create<PJ301MDarkSmall>(module->modShapeCVPos, Port::INPUT, module, Plateau::MOD_SHAPE_CV_INPUT));
    addInput(Port::create<PJ301MDarkSmall>(module->modDepthCVPos, Port::INPUT, module, Plateau::MOD_DEPTH_CV_INPUT));

    addInput(Port::create<PJ301MDarkSmall>(module->holdCVPos, Port::INPUT, module, Plateau::FREEZE_CV_INPUT));
    addInput(Port::create<PJ301MDarkSmall>(module->clearCVPos, Port::INPUT, module, Plateau::CLEAR_CV_INPUT));

    addOutput(Port::create<PJ301MDarkSmallOut>(module->leftOutputPos, Port::OUTPUT, module, Plateau::LEFT_OUTPUT));
    addOutput(Port::create<PJ301MDarkSmallOut>(module->rightOutputPos, Port::OUTPUT, module, Plateau::RIGHT_OUTPUT));

    // Make knobs

    float minAngle = -0.77f * M_PI;
    float maxAngle = 0.77f * M_PI;
    addParam(createValleyKnob<RoganMedSmallWhite>(module->dryPos, module, Plateau::DRY_PARAM, 0.0f, 1.f, 1.f, minAngle, maxAngle, DynamicKnobMotion::SMOOTH_MOTION));
    addParam(createValleyKnob<RoganMedSmallWhite>(module->wetPos, module, Plateau::WET_PARAM, 0.0f, 1.f, 0.5f, minAngle, maxAngle, DynamicKnobMotion::SMOOTH_MOTION));
    addParam(createValleyKnob<RoganSmallWhite>(module->preDelayPos, module, Plateau::PRE_DELAY_PARAM, 0.f, 0.500f, 0.f, minAngle, maxAngle, DynamicKnobMotion::SMOOTH_MOTION));
    addParam(createValleyKnob<RoganMedGreen>(module->inputLowDampPos, module, Plateau::INPUT_LOW_DAMP_PARAM, 0.f, 10.f, 10.f, minAngle, maxAngle, DynamicKnobMotion::SMOOTH_MOTION));
    addParam(createValleyKnob<RoganMedGreen>(module->inputHighDampPos, module, Plateau::INPUT_HIGH_DAMP_PARAM, 0.f, 10.f, 10.f, minAngle, maxAngle, DynamicKnobMotion::SMOOTH_MOTION));

    addParam(createValleyKnob<RoganMedBlue>(module->sizePos, module, Plateau::SIZE_PARAM, 0.f, 1.f, 0.5f, minAngle, maxAngle, DynamicKnobMotion::SMOOTH_MOTION));
    addParam(createValleyKnob<RoganMedBlue>(module->diffPos, module, Plateau::DIFFUSION_PARAM, 0.f, 10.f, 10.f, minAngle, maxAngle, DynamicKnobMotion::SMOOTH_MOTION));
    addParam(createValleyKnob<RoganMedBlue>(module->decayPos, module, Plateau::DECAY_PARAM, 0.1f, 0.9999f, 0.54995f, minAngle, maxAngle, DynamicKnobMotion::SMOOTH_MOTION));
    addParam(createValleyKnob<RoganMedGreen>(module->reverbLowDampPos, module, Plateau::REVERB_LOW_DAMP_PARAM, 0.0f, 10.f, 10.f, minAngle, maxAngle, DynamicKnobMotion::SMOOTH_MOTION));
    addParam(createValleyKnob<RoganMedGreen>(module->reverbHighDampPos, module, Plateau::REVERB_HIGH_DAMP_PARAM, 0.0f, 10.f, 10.f, minAngle, maxAngle, DynamicKnobMotion::SMOOTH_MOTION));

    addParam(createValleyKnob<RoganMedRed>(module->modRatePos, module, Plateau::MOD_SPEED_PARAM, 0.f, 1.f, 0.f, minAngle, maxAngle, DynamicKnobMotion::SMOOTH_MOTION));
    addParam(createValleyKnob<RoganMedRed>(module->modDepthPos, module, Plateau::MOD_DEPTH_PARAM, 0.f, 16.f, 0.5f, minAngle, maxAngle, DynamicKnobMotion::SMOOTH_MOTION));
    addParam(createValleyKnob<RoganMedRed>(module->modShapePos, module, Plateau::MOD_SHAPE_PARAM, 0.f, 1.f, 0.5f, minAngle, maxAngle, DynamicKnobMotion::SMOOTH_MOTION));

    // Make Attenuverters
    addParam(ParamWidget::create<RoganSmallWhite>(module->dryAttenPos, module, Plateau::DRY_CV_PARAM, -1.f, 1.f, 0.f));
    addParam(ParamWidget::create<RoganSmallWhite>(module->wetAttenPos, module, Plateau::WET_CV_PARAM, -1.f, 1.f, 0.f));
    addParam(ParamWidget::create<RoganSmallGreen>(module->inputLowDampAttenPos, module, Plateau::INPUT_LOW_DAMP_CV_PARAM, -1.f, 1.f, 0.f));
    addParam(ParamWidget::create<RoganSmallGreen>(module->inputHighDampAttenPos, module, Plateau::INPUT_HIGH_DAMP_CV_PARAM, -1.f, 1.f, 0.f));

    addParam(ParamWidget::create<RoganSmallBlue>(module->sizeAttenPos, module, Plateau::SIZE_CV_PARAM, -1.f, 1.f, 0.f));
    addParam(ParamWidget::create<RoganSmallBlue>(module->diffAttenPos, module, Plateau::DIFFUSION_CV_PARAM, -1.f, 1.f, 0.f));
    addParam(ParamWidget::create<RoganSmallBlue>(module->decayAttenPos, module, Plateau::DECAY_CV_PARAM, -1.f, 1.f, 0.f));
    addParam(ParamWidget::create<RoganSmallGreen>(module->reverbLowDampAttenPos, module, Plateau::REVERB_LOW_DAMP_CV_PARAM, -1.f, 1.f, 0.f));
    addParam(ParamWidget::create<RoganSmallGreen>(module->reverbHighDampAttenPos, module, Plateau::REVERB_HIGH_DAMP_CV_PARAM, -1.f, 1.f, 0.f));

    addParam(ParamWidget::create<RoganSmallRed>(module->modRateAttenPos, module, Plateau::MOD_SPEED_CV_PARAM, -1.f, 1.f, 0.f));
    addParam(ParamWidget::create<RoganSmallRed>(module->modShapeAttenPos, module, Plateau::MOD_SHAPE_CV_PARAM, -1.f, 1.f, 0.f));
    addParam(ParamWidget::create<RoganSmallRed>(module->modDepthAttenPos, module, Plateau::MOD_DEPTH_CV_PARAM, -1.f, 1.f, 0.f));

    // Make buttons
    addParam(ParamWidget::create<LightLEDButton>(Vec(7.875, 244.85), module, Plateau::FREEZE_PARAM, 0.f, 10.f, 0.f));
    addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(10.375, 247.35), module, Plateau::FREEZE_LIGHT));

    addParam(ParamWidget::create<LightLEDButton>(Vec(31.375, 256.35), module, Plateau::FREEZE_TOGGLE_PARAM, 0.f, 10.f, 0.f));
    addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(33.875, 258.85), module, Plateau::FREEZE_TOGGLE_LIGHT));

    addParam(ParamWidget::create<LightLEDButton>(Vec(157.875, 244.85), module, Plateau::CLEAR_PARAM, 0.f, 10.f, 0.f));
    addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(160.375, 247.35), module, Plateau::CLEAR_LIGHT));

    addParam(ParamWidget::create<LightLEDButton>(Vec(13.875, 127.35), module, Plateau::TUNED_MODE_PARAM, 0.f, 10.f, 0.f));
    addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(16.375, 129.85), module, Plateau::TUNED_MODE_LIGHT));

    addParam(ParamWidget::create<LightLEDButton>(Vec(151.875, 127.35), module, Plateau::DIFFUSE_INPUT_PARAM, 0.f, 10.f, 0.f));
    addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(154.375, 129.85), module, Plateau::DIFFUSE_INPUT_LIGHT));
}

void PlateauWidget::appendContextMenu(Menu *menu) {
    Plateau *module = dynamic_cast<Plateau*>(this->module);
    assert(module);

    menu->addChild(construct<MenuLabel>());
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Panel style"));
    menu->addChild(construct<PlateauPanelStyleItem>(&MenuItem::text, "Dark", &PlateauPanelStyleItem::module,
                                                    module, &PlateauPanelStyleItem::panelStyle, 0));
    menu->addChild(construct<PlateauPanelStyleItem>(&MenuItem::text, "Light", &PlateauPanelStyleItem::module,
                                                      module, &PlateauPanelStyleItem::panelStyle, 1));

    menu->addChild(construct<MenuLabel>());
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Predelay CV Sensitivity"));
    menu->addChild(construct<PlateauPreDelayCVSensItem>(&MenuItem::text, "Normal (1x)", &PlateauPreDelayCVSensItem::module,
                                                        module, &PlateauPreDelayCVSensItem::preDelayCVSensState, 0));
    menu->addChild(construct<PlateauPreDelayCVSensItem>(&MenuItem::text, "Low (0.5x)", &PlateauPreDelayCVSensItem::module,
                                                        module, &PlateauPreDelayCVSensItem::preDelayCVSensState, 1));

    menu->addChild(construct<MenuLabel>());
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Input Sensitivity"));
    menu->addChild(construct<PlateauInputSensItem>(&MenuItem::text, "0 dB", &PlateauInputSensItem::module,
                                                        module, &PlateauInputSensItem::inputSensitivityState, 0));
    menu->addChild(construct<PlateauInputSensItem>(&MenuItem::text, "-18 dB", &PlateauInputSensItem::module,
                                                        module, &PlateauInputSensItem::inputSensitivityState, 1));

    menu->addChild(construct<MenuLabel>());
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Output Saturation"));
    menu->addChild(construct<PlateauOutputSaturationItem>(&MenuItem::text, "Off", &PlateauOutputSaturationItem::module,
                                                          module, &PlateauOutputSaturationItem::outputSaturationState, 0));
    menu->addChild(construct<PlateauOutputSaturationItem>(&MenuItem::text, "On", &PlateauOutputSaturationItem::module,
                                                          module, &PlateauOutputSaturationItem::outputSaturationState, 1));
}

RACK_PLUGIN_MODEL_INIT(Valley, Plateau) {
   Model *modelPlateau = Model::create<Plateau, PlateauWidget>(TOSTRING(SLUG), "Plateau", "Plateau", REVERB_TAG);
   return modelPlateau;
}

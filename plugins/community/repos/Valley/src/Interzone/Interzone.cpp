#include "Interzone.hpp"

Interzone::Interzone() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
    calcGTable(engineGetSampleRate());
    filter.setSampleRate(engineGetSampleRate());
    filter.setCutoff(5.f);
    filter.setNLP(true);
    highpass.setSampleRate(engineGetSampleRate());

    lfoSlew.setSampleRate(engineGetSampleRate());
    lfoSlew.setCutoffFreq(14000.f);
    osc.setSampleRate(engineGetSampleRate());
    glide.setSampleRate(engineGetSampleRate());

    lfo.setSampleRate(engineGetSampleRate());
    lfoSlew.setSampleRate(engineGetSampleRate());
    lfoSlew.setCutoffFreq(14000.f);

    gateSlew.setSampleRate(engineGetSampleRate());
    gateSlew.setCutoffFreq(90.f);

    env.setSampleRate(engineGetSampleRate());
    sampleAndHold = 0.f;
}

void Interzone::step() {
    lfo.setFrequency(0.1f * powf(2.f, params[LFO_RATE_PARAM].value + params[LFO_FINE_PARAM].value + inputs[LFO_RATE_INPUT].value));
    lfo.sync(inputs[LFO_SYNC_INPUT].value);
    lfo.trigger(inputs[LFO_TRIG_INPUT].value);
    lfo.process();
    pink.process();
    noise = params[NOISE_TYPE_PARAM].value > 0.5f ? pink.getValue() : lfo.out[DLFO::NOISE_WAVE];

    outputs[LFO_SINE_OUTPUT].value = lfo.out[DLFO::SINE_WAVE] * 5.f;
    outputs[LFO_TRI_OUTPUT].value = lfo.out[DLFO::TRI_WAVE] * 5.f;
    outputs[LFO_SAW_UP_OUTPUT].value = lfo.out[DLFO::SAW_UP_WAVE] * 5.f;
    outputs[LFO_SAW_DOWN_OUTPUT].value = lfo.out[DLFO::SAW_DOWN_WAVE] * 5.f;
    outputs[LFO_PULSE_OUTPUT].value = lfo.out[DLFO::SQUARE_WAVE] * 5.f;
    outputs[LFO_SH_OUTPUT].value = lfo.out[DLFO::SH_WAVE] * 5.f;
    outputs[LFO_NOISE_OUTPUT].value = noise * 5.f;

    lfoSlew.setCutoffFreq(1760.f * pow(2.f, (params[LFO_SLEW_PARAM].value * 2.f) * -6.f));
    lfoSlew.input = lfo.out[(int)params[LFO_WAVE_PARAM].value];
    lfoSlew.process();
    lfoValue = params[LFO_SLEW_PARAM].value > 0.001f ? lfoSlew.output : lfoSlew.input;
    lights[LFO_LIGHT].value = lfoValue;

    // CV Input conditioning
    gateLevel = (inputs[GATE_INPUT].value + params[ENV_MANUAL_PARAM].value) > 0.5f ? 1.f : 0.f;
    lights[ENV_LIGHT].value = gateLevel;
    env.attackTime = params[ENV_ATTACK_PARAM].value;
    env.decayTime = params[ENV_DECAY_PARAM].value;
    env.sustain = params[ENV_SUSTAIN_PARAM].value;
    env.releaseTime = params[ENV_RELEASE_PARAM].value;
    env.loop = params[ENV_CYCLE_PARAM].value > 0.5f ? true : false;
    env.timeScale = params[ENV_LENGTH_PARAM].value > 0.5f ? 0.1f : 1.f;
    env.process(gateLevel, inputs[TRIG_INPUT].value);

    pitch = params[COARSE_MODE_PARAM].value > 0.5f ? semitone(params[COARSE_PARAM].value + 0.04) : params[COARSE_PARAM].value;
    pitch -= 1.f;
    pitch += (int)params[OCTAVE_PARAM].value + params[FINE_PARAM].value;
    pitch += inputs[VOCT_INPUT_1].value + inputs[VOCT_INPUT_2].value;
    glide.setCutoffFreq(330.f * pow(2.f, (params[GLIDE_PARAM].value * 2.f) * -7.f));
    glide.input = pitch;
    pitch = glide.process();

    oscPitchMod = params[PITCH_MOD_SOURCE_PARAM].value > 0.5f ? (params[PITCH_MOD_ENV_POL_PARAM].value * 2.f - 1.f) * env.value : lfoValue;
    osc.setFrequency(261.626f * powf(2.f, pitch + oscPitchMod * params[PITCH_MOD_PARAM].value * params[PITCH_MOD_PARAM].value));

    switch((int)params[PW_MOD_SOURCE_PARAM].value) {
        case 0: pwm = inputs[PW_MOD_INPUT].value * -0.1f; break;
        case 1: pwm = (lfoValue * -0.5f - 0.5f); break;
        case 2: pwm = -(params[PW_MOD_ENV_POL_PARAM].value * 2.f - 1.f) * env.value;
    }

    pwm *= params[PW_MOD_PARAM].value;
    pwm += params[PW_PARAM].value;
    osc._pwm = clamp(pwm, 0.0f, 0.5f);
    osc.setSubOctave((int)params[SUB_OCTAVE_PARAM].value);

    filterCutoff = env.value * (params[FILTER_ENV_POL_PARAM].value * 2.f - 1.f) * params[FILTER_ENV_PARAM].value * 10.0f;
    filterCutoff += lfoValue * params[FILTER_MOD_PARAM].value * params[FILTER_MOD_PARAM].value * 5.f;
    filterCutoff += pitch * params[FILTER_VOCT_PARAM].value;
    filterCutoff += inputs[FILTER_CUTOFF_INPUT_1].value * params[FILTER_CV_1_PARAM].value;
    filterCutoff += inputs[FILTER_CUTOFF_INPUT_2].value * params[FILTER_CV_2_PARAM].value;
    filterCutoff += params[FILTER_CUTOFF_PARAM].value;
    filter.setCutoff(filterCutoff);
    filter.setQ(params[FILTER_Q_PARAM].value + inputs[FILTER_RES_INPUT].value);
    filter.set4Pole(params[FILTER_POLES_PARAM].value);

    outputs[ENV_POSITIVE_OUTPUT].value = env.value * 5.f;
    outputs[ENV_NEGATIVE_OUTPUT].value = env.value * -5.f;

    // Main synth process
    osc.process();
    outputs[SAW_OUTPUT].value = osc._saw * 5.f;
    outputs[PULSE_OUTPUT].value = osc._pulse * 5.f;
    subWave = params[SUB_WAVE_PARAM].value > 0.f ? osc._subSaw : osc._subPulse;
    params[SUB_WAVE_PARAM].value < 0.f ? osc.setSubWave(2) : osc.setSubWave(1);
    outputs[SUB_OUTPUT].value = subWave * 5.f;

    mix = osc._saw * params[SAW_LEVEL_PARAM].value;
    mix += osc._pulse * params[PULSE_LEVEL_PARAM].value;
    mix += subWave * params[SUB_LEVEL_PARAM].value;
    mix += noise * params[NOISE_LEVEL_PARAM].value;
    mix += inputs[EXT_INPUT].value * params[EXT_LEVEL_PARAM].value;
    outputs[MIX_OUTPUT].value = mix;
    mix *= 2.f;

    highpass.setCutoffFreq(440.f * powf(2.0, params[FILTER_HPF_PARAM].value - 5.f));
    highpass.input = filter.process(mix + lfo.out[DLFO::NOISE_WAVE] * 8e-5f) * 5.f;
    output = highpass.process();
    outputs[FILTER_OUTPUT].value = highpass.output;

    gateSlew.input = gateLevel;
    gateSlew.process();
    outputLevel = params[VCA_SOURCE_PARAM].value > 0.5f ? gateSlew.output : env.value;
    outputLevel += inputs[VCA_LEVEL_CV_INPUT].value * params[VCA_LEVEL_CV_PARAM].value * 0.1f;
    outputLevel = clamp(outputLevel, -1.f, 1.f);
    outputs[VCA_OUTPUT].value = output * outputLevel;
}

void Interzone::onSampleRateChange() {
    calcGTable(engineGetSampleRate());
    osc.setSampleRate(engineGetSampleRate());
    filter.setSampleRate(engineGetSampleRate());
    highpass.setSampleRate(engineGetSampleRate());
    lfo.setSampleRate(engineGetSampleRate());
    lfoSlew.setSampleRate(engineGetSampleRate());
    gateSlew.setSampleRate(engineGetSampleRate());
    env.setSampleRate(engineGetSampleRate());
    glide.setSampleRate(engineGetSampleRate());
    pink.setSampleRate(engineGetSampleRate());
}

json_t* Interzone::toJson()  {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "panelStyle", json_integer(panelStyle));
    return rootJ;
}

void Interzone::fromJson(json_t *rootJ) {
    json_t *panelStyleJ = json_object_get(rootJ, "panelStyle");
    panelStyle = json_integer_value(panelStyleJ);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void InterzonePanelStyleItem::onAction(EventAction &e) {
    module->panelStyle = panelStyle;
}

void InterzonePanelStyleItem::step() {
    rightText = (module->panelStyle == panelStyle) ? "✔" : "";
    MenuItem::step();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

InterzoneWidget::InterzoneWidget(Interzone* module) : ModuleWidget(module) {
    {
        DynamicPanelWidget *panel = new DynamicPanelWidget();
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/InterzonePanelDark.svg")));
        panel->addPanel(SVG::load(assetPlugin(plugin, "res/InterzonePanelLight.svg")));

        box.size = panel->box.size;
        panel->mode = &module->panelStyle;
        addChild(panel);
    }

    addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    // Params

    addParam(ParamWidget::create<OrangeSlider>(VCOGlideSliderPos, module, Interzone::GLIDE_PARAM, 0.f, 1.f, 0.f));
    addParam(ParamWidget::create<OrangeSlider>(VCOModSliderPos, module, Interzone::PITCH_MOD_PARAM, 0.f, 1.f, 0.f));
    addParam(ParamWidget::create<OrangeSlider>(VCOWidthSliderPos, module, Interzone::PW_PARAM, 0.5f, 0.f, 0.5f));
    addParam(ParamWidget::create<OrangeSlider>(VCOPWMSliderPos, module, Interzone::PW_MOD_PARAM, 0.f, 0.5f, 0.f));
    addParam(createValleyKnob<RoganMedOrange>(VCOOctavePos, module, Interzone::OCTAVE_PARAM, -2.f,
                                              2.f, 0.f, octaveMinAngle, octaveMaxAngle,
                                              DynamicKnobMotion::SNAP_MOTION));

    addParam(ParamWidget::create<RoganSmallOrange>(VCOCoarsePos, module, Interzone::COARSE_PARAM, 0.f, 2.f, 1.f));
    addParam(ParamWidget::create<RoganSmallOrange>(VCOFinePos, module, Interzone::FINE_PARAM, -0.0833333f, 0.0833333f, 0.f));
    addParam(ParamWidget::create<CKSS>(VCOModEnvPolPos, module, Interzone::PITCH_MOD_ENV_POL_PARAM, 0.f, 1.f, 1.f));
    addParam(ParamWidget::create<CKSS>(VCOModSourcePos, module, Interzone::PITCH_MOD_SOURCE_PARAM, 0.0f, 1.f, 0.f));
    addParam(ParamWidget::create<CKSS>(VCOPWMEnvPolPos, module, Interzone::PW_MOD_ENV_POL_PARAM, 0.f, 1.f, 0.f));
    addParam(ParamWidget::create<CKSS>(VCOCoarseModePos, module, Interzone::COARSE_MODE_PARAM, 0.f, 1.f, 0.f));
    addParam(ParamWidget::create<CKSSThree>(VCOPWMSourcePos, module, Interzone::PW_MOD_SOURCE_PARAM, 0.0f, 2.f, 1.f));
    addParam(ParamWidget::create<YellowStepSlider>(VCOSubOctPos, module, Interzone::SUB_OCTAVE_PARAM, 0.f, 6.f, 1.f));

    addParam(ParamWidget::create<GreenSlider>(MixerSawLevelPos, module, Interzone::SAW_LEVEL_PARAM, 0.f, 1.f, 0.8f));
    addParam(ParamWidget::create<GreenSlider>(MixerPulseLevelPos, module, Interzone::PULSE_LEVEL_PARAM, 0.f, 1.f, 0.f));
    addParam(ParamWidget::create<GreenSlider>(MixerSubLevelPos, module, Interzone::SUB_LEVEL_PARAM, 0.f, 1.f, 0.f));
    addParam(ParamWidget::create<CKSSThree>(MixerSubWavePos, module, Interzone::SUB_WAVE_PARAM, -1.0f, 1.f, 0.0f));
    addParam(ParamWidget::create<CKSS>(MixerNoiseTypePos, module, Interzone::NOISE_TYPE_PARAM, 0.0f, 1.f, 0.0f));
    addParam(ParamWidget::create<GreenSlider>(MixerNoiseLevelPos, module, Interzone::NOISE_LEVEL_PARAM, 0.f, 1.f, 0.f));
    addParam(ParamWidget::create<GreenSlider>(MixerExtInLevelPos, module, Interzone::EXT_LEVEL_PARAM, 0.f, 1.f, 0.f));

    addParam(ParamWidget::create<BlueSlider>(FilterCutoffPos, module, Interzone::FILTER_CUTOFF_PARAM, 0.f, 10.0f, 10.f));
    addParam(ParamWidget::create<BlueSlider>(FilterResPos, module, Interzone::FILTER_Q_PARAM, 0.f, 10.f, 0.f));
    addParam(ParamWidget::create<BlueSlider>(FilterHPFPos, module, Interzone::FILTER_HPF_PARAM, 0.f, 10.0f, 0.f));
    addParam(ParamWidget::create<CKSS>(FilterPolesPos, module, Interzone::FILTER_POLES_PARAM, 0.f, 1.f, 1.f));
    addParam(ParamWidget::create<OrangeSlider>(FilterEnvPos, module, Interzone::FILTER_ENV_PARAM, 0.f, 1.f, 0.f));
    addParam(ParamWidget::create<OrangeSlider>(FilterLFOPos, module, Interzone::FILTER_MOD_PARAM, 0.f, 1.f, 0.f));
    addParam(ParamWidget::create<OrangeSlider>(FilterVOctPos, module, Interzone::FILTER_VOCT_PARAM, 0.f, 1.f, 0.f));
    addParam(ParamWidget::create<CKSS>(FilterEnvPolPos, module, Interzone::FILTER_ENV_POL_PARAM, 0.f, 1.f, 1.f));
    addParam(ParamWidget::create<RoganSmallBlue>(FilterCV1Pos, module, Interzone::FILTER_CV_1_PARAM, -1.0f, 1.f, 0.f));
    addParam(ParamWidget::create<RoganSmallBlue>(FilterCV2Pos, module, Interzone::FILTER_CV_2_PARAM, -1.0f, 1.f, 0.f));


    addParam(ParamWidget::create<RedSlider>(EnvAttackPos, module, Interzone::ENV_ATTACK_PARAM, 0.f, 1.f, 0.f));
    addParam(ParamWidget::create<RedSlider>(EnvDecayPos, module, Interzone::ENV_DECAY_PARAM, 0.f, 1.f, 0.f));
    addParam(ParamWidget::create<RedSlider>(EnvSustainPos, module, Interzone::ENV_SUSTAIN_PARAM, 0.f, 1.f, 1.f));
    addParam(ParamWidget::create<RedSlider>(EnvReleasePos, module, Interzone::ENV_RELEASE_PARAM, 0.f, 1.f, 0.f));
    addParam(ParamWidget::create<CKSS>(EnvLengthPos, module, Interzone::ENV_LENGTH_PARAM, 0.0f, 1.f, 0.f));
    addParam(ParamWidget::create<CKSS>(EnvCyclePos, module, Interzone::ENV_CYCLE_PARAM, 0.0f, 1.f, 0.f));
    addParam(ParamWidget::create<LightLEDButton>(EnvManualPos, module, Interzone::ENV_MANUAL_PARAM, 0.f, 1.f, 0.f));
    addChild(ModuleLightWidget::create<MediumLight<RedLight>>(EnvManualPos.plus(Vec(2.5f, 2.5f)), module, Interzone::ENV_LIGHT));

    addParam(ParamWidget::create<GreenSlider>(LFORatePos, module, Interzone::LFO_RATE_PARAM, 0.f, 11.f, 0.f));
    addParam(ParamWidget::create<RoganSmallOrange>(LFOFinePos, module, Interzone::LFO_FINE_PARAM, -0.5f, 0.5f, 0.f));
    addParam(ParamWidget::create<RoganSmallOrange>(LFOSlewPos, module, Interzone::LFO_SLEW_PARAM, 0.f, 1.f, 0.f));
    addParam(createValleyKnob<RoganMedOrange>(LFOWavePos, module, Interzone::LFO_WAVE_PARAM,
                                              0.f, 6.f, 0.f, lfoWaveMinAngle, lfoWaveMaxAngle,
                                              DynamicKnobMotion::SNAP_MOTION));

    addParam(ParamWidget::create<CKSS>(VCASourcePos, module, Interzone::VCA_SOURCE_PARAM, 0.0f, 1.f, 0.0f));
    addParam(ParamWidget::create<RoganSmallOrange>(VCALevelPos, module, Interzone::VCA_LEVEL_CV_PARAM, -1.f, 1.f, 0.f));

    // Lights
    addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(249.244, 155.875), module, Interzone::LFO_LIGHT));

    // IO
    addInput(Port::create<PJ301MDarkSmall>(VOctIn1Pos, Port::INPUT, module, Interzone::VOCT_INPUT_1));
    addInput(Port::create<PJ301MDarkSmall>(VOctIn2Pos, Port::INPUT, module, Interzone::VOCT_INPUT_2));
    addInput(Port::create<PJ301MDarkSmall>(PWMInPos, Port::INPUT, module, Interzone::PW_MOD_INPUT));
    addOutput(Port::create<PJ301MDarkSmallOut>(SawOutPos, Port::OUTPUT, module, Interzone::SAW_OUTPUT));
    addOutput(Port::create<PJ301MDarkSmallOut>(PulseOutPos, Port::OUTPUT, module, Interzone::PULSE_OUTPUT));
    addOutput(Port::create<PJ301MDarkSmallOut>(SubOutPos, Port::OUTPUT, module, Interzone::SUB_OUTPUT));

    addInput(Port::create<PJ301MDarkSmall>(MixerExtInPos, Port::INPUT, module, Interzone::EXT_INPUT));
    addOutput(Port::create<PJ301MDarkSmallOut>(MixerOutPos, Port::OUTPUT, module, Interzone::MIX_OUTPUT));

    addInput(Port::create<PJ301MDarkSmall>(LFORateInPos, Port::INPUT, module, Interzone::LFO_RATE_INPUT));
    addInput(Port::create<PJ301MDarkSmall>(LFOTrigInPos, Port::INPUT, module, Interzone::LFO_TRIG_INPUT));
    addInput(Port::create<PJ301MDarkSmall>(LFOSyncInPos, Port::INPUT, module, Interzone::LFO_SYNC_INPUT));
    addOutput(Port::create<PJ301MDarkSmallOut>(LFOSineOutPos, Port::OUTPUT, module, Interzone::LFO_SINE_OUTPUT));
    addOutput(Port::create<PJ301MDarkSmallOut>(LFOTriOutPos, Port::OUTPUT, module, Interzone::LFO_TRI_OUTPUT));
    addOutput(Port::create<PJ301MDarkSmallOut>(LFOSawUpPos, Port::OUTPUT, module, Interzone::LFO_SAW_UP_OUTPUT));
    addOutput(Port::create<PJ301MDarkSmallOut>(LFOSawDownPos, Port::OUTPUT, module, Interzone::LFO_SAW_DOWN_OUTPUT));
    addOutput(Port::create<PJ301MDarkSmallOut>(LFOPulseOutPos, Port::OUTPUT, module, Interzone::LFO_PULSE_OUTPUT));
    addOutput(Port::create<PJ301MDarkSmallOut>(LFOSHOutPos, Port::OUTPUT, module, Interzone::LFO_SH_OUTPUT));
    addOutput(Port::create<PJ301MDarkSmallOut>(LFONoiseOutPos, Port::OUTPUT, module, Interzone::LFO_NOISE_OUTPUT));

    addInput(Port::create<PJ301MDarkSmall>(FilterCutoffIn1Pos, Port::INPUT, module, Interzone::FILTER_CUTOFF_INPUT_1));
    addInput(Port::create<PJ301MDarkSmall>(FilterCutoffIn2Pos, Port::INPUT, module, Interzone::FILTER_CUTOFF_INPUT_2));
    addInput(Port::create<PJ301MDarkSmall>(FilterResInPos, Port::INPUT, module, Interzone::FILTER_RES_INPUT));
    addOutput(Port::create<PJ301MDarkSmallOut>(FilterOutPos, Port::OUTPUT, module, Interzone::FILTER_OUTPUT));

    addInput(Port::create<PJ301MDarkSmall>(EnvGateInPos, Port::INPUT, module, Interzone::GATE_INPUT));
    addInput(Port::create<PJ301MDarkSmall>(EnvTrigInPos, Port::INPUT, module, Interzone::TRIG_INPUT));
    addOutput(Port::create<PJ301MDarkSmallOut>(EnvPositiveOutPos, Port::OUTPUT, module, Interzone::ENV_POSITIVE_OUTPUT));
    addOutput(Port::create<PJ301MDarkSmallOut>(EnvNegativeOutPos, Port::OUTPUT, module, Interzone::ENV_NEGATIVE_OUTPUT));

    addOutput(Port::create<PJ301MDarkSmallOut>(VCAOutPos, Port::OUTPUT, module, Interzone::VCA_OUTPUT));
    addInput(Port::create<PJ301MDarkSmall>(VCALevelCVPos, Port::INPUT, module, Interzone::VCA_LEVEL_CV_INPUT));
}

void InterzoneWidget::appendContextMenu(Menu *menu) {
    Interzone *module = dynamic_cast<Interzone*>(this->module);
    assert(module);

    menu->addChild(construct<MenuLabel>());
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Panel style"));
    menu->addChild(construct<InterzonePanelStyleItem>(&MenuItem::text, "Dark", &InterzonePanelStyleItem::module,
                                                    module, &InterzonePanelStyleItem::panelStyle, 0));
    menu->addChild(construct<InterzonePanelStyleItem>(&MenuItem::text, "Light", &InterzonePanelStyleItem::module,
                                                      module, &InterzonePanelStyleItem::panelStyle, 1));
}

RACK_PLUGIN_MODEL_INIT(Valley, Interzone) {
   Model *modelInterzone = Model::create<Interzone, InterzoneWidget>(TOSTRING(SLUG), "Interzone", "Interzone",
                                                                     SYNTH_VOICE_TAG);
   return modelInterzone;
}

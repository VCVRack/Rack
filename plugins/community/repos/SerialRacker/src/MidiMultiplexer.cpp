// A Knobulator.
//
// Read the documentation at
// https://github.com/apbianco/SerialRacker/blob/master/README.md

#include "SerialRacker.hpp"

#include "dsp/digital.hpp"
#include "dsp/filter.hpp"

namespace rack_plugin_SerialRacker {

struct MidiMultiplexer : Module {
  // This parametrizes the module:
  // - The witdth of the input:
  //
  //   <-- kNinput -->
  //   (o) (o) (o) (o)
  //
  static const int kNInput = 4;
  // - The size of the output: (each output is made of line of
  //   width kNinput)
  //   
  //   <-- kNinput -->
  //   (o) (o) (o) (o) |
  //   (o) (o) (o) (o) | kNOutput
  //   (o) (o) (o) (o) |
  //   (o) (o) (o) (o) |
  static const int kNOut = 4;
  
  enum ParamIds {
    CHANNELS_PARAM,
    PREV_INPUT_BUTTON,
    NEXT_INPUT_BUTTON,
    NUM_PARAMS
  };
  enum InputIds {
    NEXT_INPUT,
    ENUMS(IN_INPUT, kNInput),
    NUM_INPUTS
  };
  enum OutputIds {
    ENUMS(OUT_OUTPUT, kNOut * kNInput),
    NUM_OUTPUTS
  };
  enum LightIds {
    ENUMS(CHANNEL_LIGHT, kNOut),
    NUM_LIGHTS
  };

  // The clock input to move to the next output row and the button to
  // move to the next and previous channel.
  SchmittTrigger clockTrigger;
  SchmittTrigger nextTrigger;
  SchmittTrigger prevTrigger;

  // Limiter to help produce the final output
  SlewLimiter channelFilter[kNInput][kNOut];

  // Text fields: the name given to an output row and a text field to
  // display CV values.
  TextField *row_label = nullptr;
  TextField *cv_values = nullptr;

  // The current output row.
  int channel = 0;

  // True when json has finished loaded. This is used to handle the
  // refresh of values right after they have been loaded.
  bool json_loaded = false;

  // Sampled outut values - these are issues on the outputs each time
  // the module runs.
  float sampled_values[kNInput][kNOut] = {
    {0.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 0.0f}};

  // Sampled current knob values. They are used to determine whether
  // knob values have just changed (and which one changed.)
  float knob_values[4] = {-1.0f, -1.0f, -1.0f, -1.0f};

  // The CV values to display
  float cv_values_to_display[kNInput] = {-1.0f, -1.0f, -1.0f, -1.0f};

  // The row labels to display.
  std::string rowDisplay[kNOut] = {SET_ROW_NAME(1), SET_ROW_NAME(2),
				   SET_ROW_NAME(3), SET_ROW_NAME(4)};

  MidiMultiplexer() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
    for (int i = 0; i < kNInput; ++i) {
      for (int j = 0; j < kNOut; ++j) {
	channelFilter[i][j].rise = 0.01f;
	channelFilter[i][j].fall = 0.01f;
      }
    }
    // Read the initial knob values
    for (int i = 0; i < kNInput; ++i) {
      knob_values[i] = inputs[IN_INPUT + i].value;
      cv_values_to_display[i] = knob_values[i];
    }
  }
  
  void step() override {
    // Determine current channel and whether the channel has changed.
    int new_channel = channel;
    int max_number_channels = kNOut - (int) params[CHANNELS_PARAM].value;

    if (clockTrigger.process(inputs[NEXT_INPUT].value / 2.f)) {
      new_channel++;
    }
    if (nextTrigger.process(params[NEXT_INPUT_BUTTON].value)) {
      new_channel++;
    }
    if (prevTrigger.process(params[PREV_INPUT_BUTTON].value)) {
      new_channel--;
    }
    // The maximum number of channels can be dynamically adjusted. We
    // recompute the limits now.
    if (new_channel < 0) {
      new_channel = max_number_channels - 1;
    }
    new_channel %= max_number_channels;
    bool channel_changed = (new_channel != channel ? true : false);
    channel = new_channel;

    // We're determining whether a knob has changed: we compare its
    // actual value with the value we had for it before. When we've
    // loaded json, we just have knob values taken from the sampled
    // values array which would have been loaded.
    bool knob_changed_values[kNInput] = {false, false, false, false};
    // This remembers whether, overall, a knob value changed.
    bool any_knob_changed = false;
    for (int i = 0; i < kNInput; ++i) {
      if (inputs[IN_INPUT + i].value != knob_values[i]) {
	knob_changed_values[i] = true;
	any_knob_changed = true;
	knob_values[i] = inputs[IN_INPUT + i].value;
      }
    }
    // Run the channel filters first
    for (int i = 0; i < kNInput; ++i) {
      for (int j = 0; j < kNOut; ++j) {
	  channelFilter[i][j].process(channel == j ? 1.0f : 0.0f);
      }
    }

    // Set all outputs
    for (int i = 0; i < kNInput; ++i) {
      // We're going to process one column of input/output at a time,
      // corresponding to the input at hand:
      //
      //      .---- i
      //      V
      // (o) (o) (o)
      //
      //     (o)  |<---- j
      //     (o)  |
      //     (o)* |<---- channel
      //     (o)  |
      //
      // Compute the output for each row of output and decide whether
      // we update or capture the output
      for (int j = 0; j < kNOut; ++j) {
	int output_index = OUT_OUTPUT + (j * kNInput) + i;
	// When we're processing the channel
	if (j == channel) {
	  // If the current knob value changed, just use the output from the
	  // knob and set the sampled value to be that value.
	  if (knob_changed_values[i]) {
	    // Compute the output and store it directly as a sampled
	    // value.
	    sampled_values[i][j] = (channelFilter[i][j].out *
				    inputs[IN_INPUT + i].value);
	  }
	  // All output from the current channel are copied as values
	  // to be displayed.
	  cv_values_to_display[i] = sampled_values[i][j];
	}
	// An now just output the sampled value. This is valid for all
	// rows, regardless of whether it's the active one or not.
	outputs[output_index].value = sampled_values[i][j];
      }
    }

    // Visual updates:
    //
    // Set the lights
    for (int i = 0; i < kNOut; ++i) {
      lights[CHANNEL_LIGHT + i].setBrightness(channelFilter[channel][i].out);
    }
    // Set the values. We limit the output to 9.99 to avoid having the
    // output take two lines. We do that only when we've registered a
    // change: json loaded, a knob value changed or a channel changed.
    if (any_knob_changed || json_loaded || channel_changed) {
      char cv_values_string[1024];
      for (int i = 0; i < kNInput; ++i) {
	if (cv_values_to_display[i] > 9.99) {
	  cv_values_to_display[i] = 9.99;
	}
      }
      sprintf(cv_values_string, "%4.2f %4.2f %4.2f %4.2f",
	      cv_values_to_display[0], cv_values_to_display[1], 
	      cv_values_to_display[2], cv_values_to_display[3]);
      if (cv_values) {
	cv_values->text.assign(cv_values_string);
      }
      info("update");
    }
    // Set the label of the currently selected channel. We do that
    // only when we've registered a change: json loaded or the channel
    // has changed.
    if (row_label) {
      if (channel_changed || json_loaded) {
	row_label->text = rowDisplay[channel];
      } else {
	rowDisplay[channel] = row_label->text;
      }
    }
    
    // After the first update iteration, we can declare that JSON has
    // been loaded.
    json_loaded = false;
  }

  // Saving and loading the module configuration to a file. We define:
  // 
  // sampled_values:    An array of sampled values
  // output_row_labels: The name assigned (by the user) to the output row
  // channel_value:     Which output row is currently active.
  json_t *toJson() override {
    json_t *root = json_object();
    // Save the sampled values
    json_t *values = json_array();
    for (int i = 0; i < kNInput; ++i) {
      for (int j = 0; j < kNOut; ++j) {
	json_t *value = json_real(sampled_values[i][j]);
	json_array_append_new(values, value);
      }
    }
    json_object_set_new(root, "sampled_values", values);

    // Save the output labels
    json_t *labels = json_array();
    for (int i = 0; i < kNOut; ++i) {
      json_t *label = json_string(rowDisplay[i].c_str());
      json_array_append_new(labels, label);
    }
    json_object_set_new(root, "output_row_labels", labels);

    // Save the current channel
    json_object_set_new(root, "channel_value", json_integer(channel));

    return root;
  }

  void fromJson(json_t *root) override {
    json_t *values = json_object_get(root, "sampled_values");
    if (values == nullptr) {
      return;
    }
    for (int i = 0; i < kNInput; ++i) {
      for (int j = 0; j < kNOut; ++j) {
	json_t *value = json_array_get(values, i * kNInput + j);
	if (value) {
	  sampled_values[i][j] = json_real_value(value);
	}
      }
    }
    json_t *labels = json_object_get(root, "output_row_labels");
    for (int i = 0; i < kNOut; ++i) {
      json_t *label = json_array_get(labels, i);
      if (label) {
	rowDisplay[i] = json_string_value(label);
      }
    }
    json_t *channel_value = json_object_get(root, "channel_value");
    if (channel_value) {
      channel = json_integer_value(channel_value);
    }
    
    json_loaded = true;
  }
};

// Helper routine to display items just reading their X/Y positions in
// InkScape.
Vec AVec(float x, float y) {
  return Vec(x, 128.499 - y);
}

Vec PortVec(float x, float y) {
  return AVec(x, y + 8.467);
}

struct MidiMultiplexerWidget : ModuleWidget {
  MidiMultiplexerWidget(MidiMultiplexer *module);
};

MidiMultiplexerWidget::MidiMultiplexerWidget(MidiMultiplexer *module) :
  ModuleWidget(module) {
  typedef MidiMultiplexer TMidiMultiplexer;
  setPanel(SVG::load(assetPlugin(plugin, "res/MidiMultiplexer.svg")));

  // Place the screws.
  addChild(Widget::create<ScrewSilver>(
    Vec(RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(
    Vec(2 * MidiMultiplexer::kNOut * RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(
    Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  addChild(Widget::create<ScrewSilver>(
    Vec(2 * MidiMultiplexer::kNOut * RACK_GRID_WIDTH,
	RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  
  // First line: Input jacks
  for (int i = 0; i < MidiMultiplexer::kNOut; ++i) {
    addInput(Port::create<PJ301MPort>(
      mm2px(PortVec(5.121 + 10.682 * i, 100.365)), Port::INPUT, module,
      TMidiMultiplexer::IN_INPUT + i));
  }

  // Next line: advance to next/previous and its associated buttons,
  // the jack that allows to change to advance to the next row and the
  // switch selecting how deep the bank goes
  addParam(ParamWidget::create<CKD6>(
    mm2px(AVec(5.655-1, 84.118+8.408)), module,
    TMidiMultiplexer::PREV_INPUT_BUTTON, 0.0f, 1.0f, 0.0f));
  addInput(Port::create<PJ301MPort>(
    mm2px(PortVec(15.803, 83.522)), Port::INPUT, module,
    TMidiMultiplexer::NEXT_INPUT));
  addParam(ParamWidget::create<CKD6>(
    mm2px(AVec(26.923-1, 84.118+8.408)), module,
    TMidiMultiplexer::NEXT_INPUT_BUTTON, 0.0f, 1.0f, 0.0f));

  addParam(ParamWidget::create<CKSSThree>(
    mm2px(AVec(39.885, 82.663+10.054)), module,
    TMidiMultiplexer::CHANNELS_PARAM, 0.0f, 2.0f, 0.0f));

  // The output label field
  TextField *field = Widget::create<LedDisplayTextField>(
    mm2px(AVec(2.864, 71.041 + 10)));
  field->box.size = mm2px(Vec(45.02, 10));
  field->multiline = false;
  module->row_label = field;
  field->text = SET_ROW_NAME(1);
  addChild(field);

  // The CV values label field
  TextField *cv_values = Widget::create<LedDisplayTextField>(
    mm2px(AVec(2.864, 58.341 + 10)));
  cv_values->box.size = mm2px(Vec(45.02, 10));
  cv_values->multiline = false;
  module->cv_values = cv_values;
  cv_values->text = "0.00 0.00 0.00 0.00";
  addChild(cv_values);

  // The output ports
  for (int j = 0; j < 4; ++j) {
    for (int i = 0; i < MidiMultiplexer::kNOut; ++i) {
      addOutput(Port::create<PJ301MPort>(
        mm2px(PortVec(5.121 + 10 * i, 42.333 - (9.245 * j))),
	Port::OUTPUT, module, TMidiMultiplexer::OUT_OUTPUT + (4 * j) + i));
    }
  }

  // The LEDs
  const float led_x = (6.121 + 10 * (MidiMultiplexer::kNOut - 1) + 8.467);
  for (int i = 0; i < MidiMultiplexer::kNOut; ++i) {
    addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      mm2px(PortVec(led_x, 42.333 - (9.245 * i))), module,
      TMidiMultiplexer::CHANNEL_LIGHT + i));
  }
}

} // namespace rack_plugin_SerialRacker

using namespace rack_plugin_SerialRacker;

RACK_PLUGIN_MODEL_INIT(SerialRacker, MidiMultiplexer) {
   Model *modelMidiMultiplexer = Model::create<MidiMultiplexer,
                                               MidiMultiplexerWidget>(
                                                  "SerialRacker", "MidiMultiplexer", "Midi Multiplexer", UTILITY_TAG);
   return modelMidiMultiplexer;
}


namespace rack_plugin_BaconMusic {

template <typename TBase>
struct Glissinator : public TBase {
  enum ParamIds {
    GLISS_TIME,

    NUM_PARAMS
  };

  enum InputIds {
    SOURCE_INPUT,
    NUM_INPUTS
  };

  enum OutputIds {
    SLID_OUTPUT,
    GLISSING_GATE,
    NUM_OUTPUTS
  };

  enum LightIds {
    SLIDING_LIGHT,
    NUM_LIGHTS
  };

  float priorIn;
  float targetIn;
  int offsetCount;

  // Hey thanks https://stackoverflow.com/a/4643091
  using TBase::params;
  using TBase::inputs;
  using TBase::outputs;
  using TBase::lights;

  Glissinator() : TBase( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ) {
    offsetCount = -1;
    params[ GLISS_TIME ].value = 0.1;
  }


  
  void step() override
  {
    float glist_sec = params[ GLISS_TIME ].value;
    int shift_time = engineGetSampleRate() * glist_sec;
    if( shift_time < 10 ) shift_time = 10;
    
    float thisIn = inputs[ SOURCE_INPUT ].value;

    // This means I am being intialized
    if( offsetCount < 0 )
      {
        priorIn = thisIn;
        offsetCount = 0;
      }

    bool inGliss = offsetCount != 0;
    float thisOut = thisIn;

    // When I begin the cycle, the shift_time may be a different shift_time than the
    // prior cycle. This is not a problem unless the shift time is now shorter than
    // the offset_time. If that's the case we have basically finished the gliss.
    // This check used to be at the end of the loop but that lead to one bad value
    // even with the >=
    if( offsetCount >= shift_time )
      {
        offsetCount = 0;
        priorIn = thisIn;
        targetIn  = thisIn;
        inGliss = false;
      }

    // I am not glissing
    if( ! inGliss )
      {
        // But I have a new target, so start glissing by setting offset count to 1.
        if( thisIn != priorIn )
          {
            targetIn = thisIn;
            offsetCount = 1;
            inGliss = true;
          }
      }

    // I am glissing (note this is NOT in an else since inGliss can be reset above)
    if( inGliss )
      {
        // OK this means my note has changed underneath me so I have to simulate my
        // starting point.
        if( thisIn != targetIn )
          {
            // This "-1" is here because we want to know the LAST known step - so at the prior
            // offset count. Without this a turnaround will tick above the turnaround point for one
            // sample.
            float lastKnown = ( ( shift_time - (offsetCount-1) ) * priorIn +
                                (offsetCount-1) * targetIn) / shift_time;
            targetIn = thisIn;
            priorIn = lastKnown;
            offsetCount = 0;
          }

        // Then the output is just the weighted sum of the prior input and this input.
        thisOut = ( ( shift_time - offsetCount ) * priorIn +
                    offsetCount * thisIn ) / shift_time;

        // and step along one.
        offsetCount ++;
      }

    lights[ SLIDING_LIGHT ].value = inGliss ? 1 : 0;
    outputs[ SLID_OUTPUT ].value = thisOut;
    outputs[ GLISSING_GATE ].value = inGliss ? 10 : 0;
  }
};

} // namespace rack_plugin_BaconMusic

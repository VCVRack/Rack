# trowaSoft-VCV
<div>
<img src="http://www.geekasaurusrex.net/image.axd?picture=2017%2f12%2fdemo_screenshot.jpg" />
</div>

trowaSoft Modules plugin for [VCV Rack](https://github.com/VCVRack/Rack) v0.5.x and v0.6.x. 
Current pack includes [trigSeq &amp; trigSeq64](#trigseq--trigseq64), [voltSeq](#voltseq), [multiWave](#multiwave) (new 2018-06-24), [multiScope](#multiscope), and [cvOSCcv](#cvosccv).

For more information about these modules, please visit:
http://www.geekasaurusrex.net/page/trowaSoft-Sequencer-Modules-for-VCV-Rack.aspx.

For more information about Rack, please visit:
https://vcvrack.com/.

If you like the modules and wish to donate, you may do so [here](https://paypal.me/j4s0n). Any donation is much appreciated! Also note though that:
+ $5 USD buys the 'dev team' a Jack &amp; Diet (programming fuel) at a neighborhood bar.
+ $10 USD buys the designer a bottle of wine.

## Binaries/Builds
Any builds that are currently available are at [Github Releases page](https://github.com/j4s0n-c/trowaSoft-VCV/releases) and [geeksaurusrex](http://www.geekasaurusrex.net/page/trowaSoft-Sequencer-Modules-for-VCV-Rack.aspx). 
Recent builds should also be available in the [VCV plugin manager](https://vcvrack.com/plugins.html).

**VCV Rack v0.6.x**:
**2018-06-24**: The latest version is [v0.6.3](https://github.com/j4s0n-c/trowaSoft-VCV/releases/tag/v0.6.3) (for Rack v0.6.1).

**VCV Rack v0.5.x**:
2018-02-17: The last version is [v0.5.5.2](https://github.com/j4s0n-c/trowaSoft-VCV/releases/tag/v0.5.5.2). No more versions for Rack 0.5.x will be developed.


To build for your platform, please visit the [VCV rack documentation](https://github.com/VCVRack/Rack#setting-up-your-development-environment).


## Sequencers
Currently there are three (3) sequencer modules.

### trigSeq &amp; trigSeq64
<div>
<img width="390" src="http://www.geekasaurusrex.net/image.axd?picture=2017%2f12%2ftrigSeq_screenshot.jpg" />
<img width="390" src="http://www.geekasaurusrex.net/image.axd?picture=2017%2f12%2ftrigSeq64_screenshot.jpg" />
</div>
  
These are basic boolean on/off pad step sequencers (0V or 10V), based off the [Fundamentals SEQ3 sequencer](https://github.com/VCVRack/Fundamental).

+ **trigSeq** is 16-step; **trigSeq64** is 64-step.
+ Now 64 patterns. ~~16 patterns.~~
+ 16 channels (outputs).
+ Output modes: **TRIG** (trigger), **RTRG** (retrigger), **GATE** (continuous) (0 or 10V).
+ Inputs: Pattern, BPM, (step) Length, Clock, Reset.
+ Copy & Paste of channel or entire pattern.
+ Open Sound Control (OSC) interface (as of v.0.5.5.1). [more info](https://github.com/j4s0n-c/trowaSoft-VCV/wiki/Open-Sound-Control-(OSC)-Interface)
+ Advanced Randomization options (as of v.0.5.5.2) for all patterns, current edit pattern, or only the displayed channel. Chose from 'normal random' or 'structured' random patterns.
+ Hold mouse down and set multiple pads by dragging.

### voltSeq
<div>
<img width="390" src="http://www.geekasaurusrex.net/image.axd?picture=2017%2f12%2fvoltSeq_screenshot.jpg" />
</div>
  
**voltSeq** is a variable voltage output step sequencer (-10V to +10V), based off the [Fundamentals SEQ3 sequencer](https://github.com/VCVRack/Fundamental).

+ **voltSeq** is 16-step.
+ Now 64 patterns. ~~16 patterns.~~
+ 16 channels (outputs).
+ Output modes:
    + **VOLT** - Voltage (-10V to +10V): Output whatever voltage you want.
    + **NOTE** - Midi Note (-5V to +5V) ~~(-4V to +6V)~~: Output notes (12 notes per 1 V; 10 octaves). [Base pitch (0V)](https://github.com/j4s0n-c/trowaSoft-VCV/issues/9) is now C4 (as of v0.5.5.2). Range is C-1 to C9 as of v0.6.0.
    + **PATT** - Pattern (-10V to +10V): To control the currently playing Pattern (or Length) on another **trigSeq** or **voltSeq**. (Now 1 to 64 in range).  
+ Inputs: Pattern, BPM, (step) Length, Clock, Reset.
+ Copy & Paste of channel or entire pattern.
+ Open Sound Control (OSC) interface (as of v.0.5.5.1). [more info](https://github.com/j4s0n-c/trowaSoft-VCV/wiki/Open-Sound-Control-(OSC)-Interface)
+ Advanced Randomization options (as of v.0.5.5.2) for all patterns, current edit pattern, or only the displayed channel. Chose from 'normal random' or 'structured' random patterns.
+ Shift Values (as of v0.5.5.2): +/- 1 Volt or 1 Octave or 1 Pattern for all patterns, current edit pattern, or only the displayed channel.

## multiWave
<div>
<img width="700" src="https://github.com/j4s0n-c/trowaSoft-VCV/blob/master/screenshots/multiWave_screenshot.png?raw=true" />
</div>

**multiWave** is a digital oscillator module with three (3) oscillators/clocks, each with two (2) configurable wave channel outputs. This module has been made to complement <a href="#multiscope">multiScope</a>
and is new in v0.6.3.

+ Screen User Controls:
    + Click on a value to edit it directly (a text box should appear and allow you to type the value).
    + **Tab** or **Tab-Shift** will iterate through the editable text boxes. 
    + Valid for all displayed values except for WAVE and AUX (AUX is only an editable textbox for pulse width when SQR/rectangle wave is selected).

+ CV Inputs & User Controls per Oscillator:
    + **SYNC** - (Right hand side) Reset/sync the oscillator (to phase 0). Currently this is CV only (no UI control).
    + **AMPL** - Amplitude (-10V to +10V).
    + **FREQ** - Frequency (1V/Oct) for the oscillator clock.  
    The Frequency knob rotates 360&deg;. Hold down the **Shift** key for coarser control or the **Control** key for finer control while dragging up/down.
    + **PHASE** - Phase Shift (-10V to +10V).
    + **OFFSET** - Offset (-10V to +10V).
    + CV Inputs & User Controls per Channel Output:
        + **WAV** - Waveform Type (-5V to +5V): SIN, TRI, SAW, SQR.
        + **AUX** - Aux (-5V to +5V). If the CV input is active then, the knob value is ignored.  
        Currently only SAW and SQR have functions: 
		    + SAW: Slope (pos |/| or neg |\\|). 0 or positive CV for positive slope.
			+ SQR: Pulse Width.
			+ SIN and TRI adjustments will be added later when/if we think of another parameter for these waveforms.  
        + **PHASE** - Phase Shift (-10V to +10V). Value is relative to the oscillator clock.
        + **MOD** - Amplitude modulation (-10V to +10V). Knob controls the mix between the raw signal and the modded signal.
        + **\*** - Button for modulation type (Digital or Ring). Currently this is UI only (no CV input).

+ CV Outputs per Oscillator:
    + **SYNC** - Triggers whenever the period restarts.
    + CV Outputs per Channel Output:
        + **X&lt;n&gt; or Y&lt;n&gt;** - RAW waveform without amplitude modulation (**MOD**).
        + **MOD** - The modulated waveform (based on the MOD knob and the incoming MOD signal input).

## multiScope
<div>
<img width="700" src="https://github.com/j4s0n-c/trowaSoft-VCV/blob/master/screenshots/multiScope_screenshot_02.png?raw=true" />
</div>

**multiScope** is a visual effects scope, with lissajous mode, that allows three (3) waveforms to be drawn on the same screen/canvas. (code based on [JW Modules FullScope](https://github.com/jeremywen) and [Fundamental Scope](https://github.com/VCVRack/Fundamental))

**WARNING**: New version seems to crash on Mac OS. If anyone more adept at OSX programming wants to figure out why, it would be **MUCH** appreciated. Otherwise, whenever there is time we will try to figure it out, but the multiScope module has been degraded to low priority.

+ CV Inputs per Channel:
    + **X** - X-value (horizontal component).
    + **Y** - Y-value (vertical component).
    + **C** - Color/hue (0V to +5V).
    + **A** - Alpha channel (0V to +5V).
    + **BLANK** - Blank ON or OFF. By default, Blank is off. ON is any input <=0 (really < 0.1V), otherwise it will be OFF.  
    You can use a **trigSeq** (in **CONT** mode, synchronized with a **voltSeq**) to control / hide lines that you do not wish to be shown.
	+ **FC** - Fill Color hue (0V to +5V)
    + **FA** - Fill alpha channel (0V to +5V).
    + **R** - Rotation (-10V to +10V). Will either be a rotational rate or if the **ABS** button is on, it will be the absolute angular position.
    + **T** - Time.
    + **TH** - Line Thickness.
  
+ User Controls per Channel:
    + **X** - Offset (OFF) & Scale (SCL) knobs.
    + **Y** - Offset (OFF) & Scale (SCL) knobs.
    + **LNK** - (Toggle) Link the X-scale and Y-scale knobs together so they will change together (have the same value).
    + **C** - Color knob. If an input is active on the Color port, this is ignored. Highest setting will yield White now.
    + **A** - Alpha channel knob. If an input is active on the Alpha port, this is ignored.
    + **FC** - Fill Color knob. If an input is active on the Fill Color port, this is ignored. Highest setting yield give White now.
    + **Fill Color** - (Toggle) Fill on/off.
    + **FA** - Fill alpha channel knob. If an input is active on the Alpha port, this is ignored.
    + _Rotation Controls_:
        + **R** - Rotation knob. If an input is active on the Rotation port, this is ignored.
        + **ABS** - (Toggle) Turning ABS on will make the rotation inputs control the absolute angular position instead of a rate.		
    + **T** - Time adjustment knob. Will be used along with the Time input port.
    + **TH** - Line Thickness. If an input is active on the Thickness port, this is ignored.
	+ **EFFECT** - Effect knob.
    + **X*Y** - (Toggle) Toggle lissajous mode on / off (default is on).

+ User Controls for entire module:
    + **INFO** - (Toggle) Toggle input parameter information on / off (default is on). Located on the right-hand-side (RHS) bar.
	+ **BG COLOR** - (Toggle) Toggle on-screen Background Color picker on / off (default is on). Located on the right-hand-side (RHS) bar.
	+ **Background Color Picker** - Displayed on screen. Hue-Saturation-Light (HSL) sliders to pick the background color.

## cvOSCcv
<div>
<img width="700" src="https://github.com/j4s0n-c/trowaSoft-VCV/blob/master/screenshots/cvOSCcv_screenshot_01.png?raw=true" />
</div>
  
**cvOSCcv** is a simple, generic Open Sound Control (OSC) module for outputting Rack CVs to OSC and reading in simple OSC messages into Rack CVs. This module is new in version 0.6.0.

+ **CV Inputs** - CV => OSC (8 Channels), each channel:
    + **TRG** - If active, then OSC messages will output the **VAL** CV input when triggered.
    + **VAL** - The value that will output over OSC. Currently sent as a float.
      If there is no trigger present, the module will output whenever **VAL** changes at least 0.05 up to 100 Hz.
+ **CV Outputs** - OSC => CV (8 channels), each channel:
    + **TRG** - (0-10V) Triggers whenever an OSC message is received.
	+ **VAL** - (Gate) Outputs the last OSC value received.
+ **User Controls**:
	+ **CONFIG** - (Toggle) Button to toggle the configuration view. When an OSC connection is active, a blue light will appear on the button.
	+ **OSC IP Address** - The IP address of the OSC client/server.  Default is `127.0.0.1`.
	+ **Out Port** - Port for sending messages. 
	+ **In Port** - Port for receiving messages. Currently, trowaSoft modules can NOT share the same ports.
	+ **Namespace** - The OSC namespace. Default is `trowacv`.
	+ **Auto Con** - Automatically reconnect on load from save. The connection will be restore if the connection was active (in the save file) and this is checked.
    + Per Channel:  
        + **Address** - Endpoint address. Default is `/ch/{channel #}`.
		+ **ADV** - (as of v0.6.2) Advanced settings for simple value conversions. Specify simple OSC data types (float, int, bool) and the CV and OSC ranges.

	NOTE: To save Channel Address changes after a connection is active, simply hide the configuration screen again.


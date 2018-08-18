# Tcl/Tk Demo GUI for the Synthesis Toolkit (STK)
# by Gary P. Scavone, CCRMA, Stanford University, 1995--2017.

# Set initial control values
set pitch 64.0
set press 64.0
set velocity 96.0
set cont1 0.0
set cont2 10.0
set cont4 20.0
set cont7 128.0
set cont11 64.0
set cont44 24.0
set patchnum 0
set oldpatch 0
set temp 0

# Configure main window
wm title . "STK Demo GUI"
wm iconname . "demo"
. config -bg black

# Configure instrument change menu.  Use a unique number for each
# voice.  The STK program change value is found by dividing by 100,
# while the remainder corresponds to within-class preset values.
menu .menu -tearoff 0
menu .menu.instrument -tearoff 0
.menu add cascade -label "Instruments" -menu .menu.instrument \
    -underline 0
.menu.instrument add radio -label "Clarinet" -variable patchnum \
    -value 0 -command { patchChange $patchnum }
.menu.instrument add radio -label "BlowHole" -variable patchnum \
    -value 100 -command { patchChange $patchnum }
.menu.instrument add radio -label "Saxofony" -variable patchnum \
    -value 200 -command { patchChange $patchnum }
.menu.instrument add radio -label "Flute" -variable patchnum \
    -value 300 -command { patchChange $patchnum }
.menu.instrument add radio -label "Brass" -variable patchnum \
    -value 400 -command { patchChange $patchnum }
.menu.instrument add radio -label "Blown Bottle" -variable patchnum \
    -value 500 -command { patchChange $patchnum }
.menu.instrument add radio -label "Bowed String" -variable patchnum \
    -value 600 -command { patchChange $patchnum }
.menu.instrument add radio -label "Plucked String" -variable patchnum \
    -value 700 -command { patchChange $patchnum }
.menu.instrument add radio -label "Stiff String" -variable patchnum \
    -value 800 -command { patchChange $patchnum }
.menu.instrument add radio -label "Sitar" -variable patchnum \
    -value 900 -command { patchChange $patchnum }
.menu.instrument add radio -label "Mandolin" -variable patchnum \
    -value 1000 -command { patchChange $patchnum }
.menu.instrument add radio -label "Rhodey" -variable patchnum \
    -value 1100 -command { patchChange $patchnum }
.menu.instrument add radio -label "Wurley" -variable patchnum \
    -value 1200 -command { patchChange $patchnum }
.menu.instrument add radio -label "Tubular Bell" -variable patchnum \
    -value 1300 -command { patchChange $patchnum }
.menu.instrument add radio -label "Heavy Metal" -variable patchnum \
    -value 1400 -command { patchChange $patchnum }
.menu.instrument add radio -label "Percussive Flute" -variable patchnum \
    -value 1500 -command { patchChange $patchnum }
.menu.instrument add radio -label "B3 Organ" -variable patchnum \
    -value 1600 -command { patchChange $patchnum }
.menu.instrument add radio -label "FM Voice" -variable patchnum \
    -value 1700 -command { patchChange $patchnum }
.menu.instrument add radio -label "Formant Voice" -variable patchnum \
    -value 1800 -command { patchChange $patchnum }
.menu.instrument add radio -label "Moog" -variable patchnum \
    -value 1900 -command { patchChange $patchnum }
.menu.instrument add radio -label "Simple" -variable patchnum \
    -value 2000 -command { patchChange $patchnum }
.menu.instrument add radio -label "Drum Kit" -variable patchnum \
    -value 2100 -command { patchChange $patchnum }
.menu.instrument add radio -label "Banded Bar" -variable patchnum \
    -value 2200 -command { patchChange $patchnum }
.menu.instrument add radio -label "Banded Marimba" -variable patchnum \
    -value 2201 -command { patchChange $patchnum }
.menu.instrument add radio -label "Banded Glass" -variable patchnum \
    -value 2202 -command { patchChange $patchnum }
.menu.instrument add radio -label "Banded Bowl" -variable patchnum \
    -value 2203 -command { patchChange $patchnum }
.menu.instrument add radio -label "Maraca" -variable patchnum \
    -value 2300 -command { patchChange $patchnum }
.menu.instrument add radio -label "Cabasa" -variable patchnum \
    -value 2301 -command { patchChange $patchnum }
.menu.instrument add radio -label "Sekere" -variable patchnum \
    -value 2302 -command { patchChange $patchnum }
.menu.instrument add radio -label "Tambourine" -variable patchnum \
    -value 2303 -command { patchChange $patchnum }
.menu.instrument add radio -label "Sleigh Bells" -variable patchnum \
    -value 2304 -command { patchChange $patchnum }
.menu.instrument add radio -label "Bamboo Chimes" -variable patchnum \
    -value 2305 -command { patchChange $patchnum } \
    -columnbreak 1

.menu.instrument add radio -label "Sandpaper" -variable patchnum \
    -value 2306 -command { patchChange $patchnum }
.menu.instrument add radio -label "Coke Can" -variable patchnum \
    -value 2307 -command { patchChange $patchnum }
.menu.instrument add radio -label "Sticks" -variable patchnum \
    -value 2308 -command { patchChange $patchnum }
.menu.instrument add radio -label "Crunch" -variable patchnum \
    -value 2309 -command { patchChange $patchnum }
.menu.instrument add radio -label "Big Rocks" -variable patchnum \
    -value 2310 -command { patchChange $patchnum }
.menu.instrument add radio -label "Little Rocks" -variable patchnum \
    -value 2311 -command { patchChange $patchnum }
.menu.instrument add radio -label "NeXT Mug" -variable patchnum \
    -value 2312 -command { patchChange $patchnum }
.menu.instrument add radio -label "Mug & Penny" -variable patchnum \
    -value 2313 -command { patchChange $patchnum }
.menu.instrument add radio -label "Mug & Nickle" -variable patchnum \
    -value 2314 -command { patchChange $patchnum }
.menu.instrument add radio -label "Mug & Dime" -variable patchnum \
    -value 2315 -command { patchChange $patchnum }
.menu.instrument add radio -label "Mug & Quarter" -variable patchnum \
    -value 2316 -command { patchChange $patchnum }
.menu.instrument add radio -label "Mug & Franc" -variable patchnum \
    -value 2317 -command { patchChange $patchnum }
.menu.instrument add radio -label "Mug & Peso" -variable patchnum \
    -value 2318 -command { patchChange $patchnum }
.menu.instrument add radio -label "Guiro" -variable patchnum \
    -value 2319 -command { patchChange $patchnum }
.menu.instrument add radio -label "Wrench" -variable patchnum \
    -value 2320 -command { patchChange $patchnum }
.menu.instrument add radio -label "Water Drops" -variable patchnum \
    -value 2321 -command { patchChange $patchnum }
.menu.instrument add radio -label "Tuned Bamboo" -variable patchnum \
    -value 2322 -command { patchChange $patchnum }
.menu.instrument add radio -label "Marimba" -variable patchnum \
    -value 2400 -command { patchChange $patchnum }
.menu.instrument add radio -label "Vibraphone" -variable patchnum \
    -value 2401 -command { patchChange $patchnum }
.menu.instrument add radio -label "Agogo Bell" -variable patchnum \
    -value 2402 -command { patchChange $patchnum }
.menu.instrument add radio -label "Wood 1" -variable patchnum \
    -value 2403 -command { patchChange $patchnum }
.menu.instrument add radio -label "Reso" -variable patchnum \
    -value 2404 -command { patchChange $patchnum }
.menu.instrument add radio -label "Wood 2" -variable patchnum \
    -value 2405 -command { patchChange $patchnum }
.menu.instrument add radio -label "Beats" -variable patchnum \
    -value 2406 -command { patchChange $patchnum }
.menu.instrument add radio -label "Two Fixed" -variable patchnum \
    -value 2407 -command { patchChange $patchnum }
.menu.instrument add radio -label "Clump" -variable patchnum \
    -value 2408 -command { patchChange $patchnum }
.menu.instrument add radio -label "2D Mesh" -variable patchnum \
    -value 2500 -command { patchChange $patchnum }
.menu.instrument add radio -label "Resonate" -variable patchnum \
    -value 2600 -command { patchChange $patchnum }
.menu.instrument add radio -label "Police Whistle" -variable patchnum \
    -value 2700 -command { patchChange $patchnum }

. configure -menu .menu

# Configure message box
label .note -font {Times 12 normal} -background white \
    -foreground darkred -relief raised \
    -wraplength 300 -width 60 \
    -text "Select instruments using the menu above. Impulsively excited instruments can be plucked/struck using the NoteOn button or the spacebar."
pack .note -padx 5 -pady 10

# Configure bitmap display
if {[file isdirectory bitmaps]} {
    set bitmappath bitmaps
} else {
    set bitmappath tcl/bitmaps
}

button .pretty -bitmap @$bitmappath/Klar.xbm \
    -background white -foreground black
pack .pretty -padx 5 -pady 10

# Configure "note on" buttons
frame .noteOn -bg black

button .noteOn.on -text NoteOn -bg grey66 -command { noteOn $pitch $press }
button .noteOn.off -text NoteOff -bg grey66 -command { noteOff $pitch 127.0 }
button .noteOn.exit -text "Exit Program" -bg grey66 -command myExit
pack .noteOn.on -side left -padx 5
pack .noteOn.off -side left -padx 5 -pady 10
pack .noteOn.exit -side left -padx 5 -pady 10

pack .noteOn

# Configure sliders
frame .left -bg black
frame .right -bg black

scale .left.volume -from 0 -to 128 -length 200 \
    -command {printWhatz "ControlChange    0.0  1 " 7} \
    -orient horizontal -label "Volume" \
    -tickinterval 32 -showvalue true -bg grey66  \
    -variable cont7

scale .left.bPressure -from 0 -to 128 -length 200 \
    -command {changePress } -variable press \
    -orient horizontal -label "Breath Pressure" \
    -tickinterval 32 -showvalue true -bg grey66

scale .left.pitch -from 0 -to 128 -length 200 \
    -command {changePitch } -variable pitch \
    -orient horizontal -label "MIDI Note Number" \
    -tickinterval 32 -showvalue true -bg grey66

scale .left.cont2 -from 0 -to 128 -length 200 \
    -command {printWhatz "ControlChange    0.0  1 " 2} \
    -orient horizontal -label "Reed Stiffness" \
    -tickinterval 32 -showvalue true -bg grey66  \
    -variable cont2

scale .right.reverb -from 0 -to 128 -length 200 \
    -command {printWhatz "ControlChange    0.0  1 " 44} \
    -orient horizontal -label "Reverb Mix" \
    -tickinterval 32 -showvalue true -bg grey66  \
    -variable cont44

scale .right.cont4 -from 0 -to 128 -length 200 \
    -command {printWhatz "ControlChange    0.0  1 " 4} \
    -orient horizontal -label "Breath Noise" \
    -tickinterval 32 -showvalue true -bg grey66  \
    -variable cont4

scale .right.cont11 -from 0 -to 128 -length 200 \
    -command {printWhatz "ControlChange    0.0  1 " 11} \
    -orient horizontal -label "Vibrato Rate" \
    -tickinterval 32 -showvalue true -bg grey66  \
    -variable cont11 

scale .right.cont1 -from 0 -to 128 -length 200 \
    -command {printWhatz "ControlChange    0.0  1 " 1} \
    -orient horizontal -label "Vibrato Amount" \
    -tickinterval 32 -showvalue true -bg grey66  \
    -variable cont1

pack .left.volume -padx 10 -pady 10
pack .left.bPressure -padx 10 -pady 10
pack .left.pitch -padx 10 -pady 10
pack .left.cont2 -padx 10 -pady 10
pack .right.reverb -padx 10 -pady 10
pack .right.cont4 -padx 10 -pady 10
pack .right.cont11 -padx 10 -pady 10
pack .right.cont1 -padx 10 -pady 10

pack .left -side left
pack .right -side right

# DrumKit popup window
set p .drumwindow

proc myExit {} {
    global pitch
    puts [format "ExitProgram"]
    flush stdout
    close stdout
    exit
}

proc noteOn {pitchVal pressVal} {
    puts [format "NoteOn           0.0  1  %3.2f  %3.2f" $pitchVal $pressVal]
    flush stdout
}

proc noteOff {pitchVal pressVal} {
    puts [format "NoteOff          0.0  1  %3.2f %3.2f" $pitchVal $pressVal]
    flush stdout
}

# Set bindings
bind . <KeyPress> { noteOn $pitch $press }
bind . <Destroy> +myExit

proc playDrum {value}	{
    global velocity
    puts [format "NoteOn           0.0  1  %3i  %3.2f" $value $velocity]
    flush stdout
}

proc printWhatz {tag value1 value2 } {
    puts [format "%s %2i  %3.2f" $tag $value1 $value2]
    flush stdout
}

proc changePress {value} {
    global patchnum
    if { $patchnum<700 || ($patchnum>900 && $patchnum<2500) || $patchnum>=2600 } {
      puts [format "AfterTouch       0.0  1  %3.2f" $value]
      flush stdout
    }
}

proc changePitch {value} {
    puts [format "PitchChange      0.0  1  %3.2f" $value]
    flush stdout
}

proc patchChange {value} {
    global bitmappath cont1 cont2 cont4 cont11 oldpatch press pitch temp
    if {$value!=$oldpatch} {
        set program [expr $value / 100]
        puts [format "ProgramChange    0.0  1  %2i" $program]
        flush stdout

        # This stuff below sets up the correct bitmaps, slider labels, and control
        # parameters.
        if {$program==0}	{ # Clarinet
            .pretty config -bitmap @$bitmappath/Klar.xbm
            .left.bPressure config -state normal -label "Breath Pressure"
            .left.pitch config -state normal -label "MIDI Note Number"
            .left.cont2 config -state normal -label "Reed Stiffness"
            .right.cont4 config -state normal -label "Breath Noise"
            .right.cont11 config -state normal -label "Vibrato Rate"
            .right.cont1 config -state normal -label "Vibrato Amount"
            set cont1 20.0
            set cont2 64.0
            set cont4 20.0
            set cont11 64.0
            printWhatz "ControlChange    0.0  1 " 1  $cont1
            printWhatz "ControlChange    0.0  1 " 2  $cont2
            printWhatz "ControlChange    0.0  1 " 4  $cont4
            printWhatz "ControlChange    0.0  1 " 11 $cont11
        }
        if {$program==1}	{ # BlowHole
            .pretty config -bitmap @$bitmappath/Klar.xbm
            .left.bPressure config -state normal -label "Breath Pressure"
            .left.pitch config -state normal -label "MIDI Note Number"
            .left.cont2 config -state normal -label "Reed Stiffness"
            .right.cont4 config -state normal -label "Breath Noise"
            .right.cont11 config -state normal -label "Tonehole Openness"
            .right.cont1 config -state normal -label "Register Vent Openness"
            set cont1 0.0
            set cont2 64.0
            set cont4 20.0
            set cont11 0.0
            printWhatz "ControlChange    0.0  1 " 1  $cont1
            printWhatz "ControlChange    0.0  1 " 2  $cont2
            printWhatz "ControlChange    0.0  1 " 4  $cont4
            printWhatz "ControlChange    0.0  1 " 11 $cont11
        }
        if {$program==2}	{ # Saxofony
            .pretty config -bitmap @$bitmappath/prcFunny.xbm
            .left.bPressure config -state normal -label "Breath Pressure"
            .left.pitch config -state normal -label "MIDI Note Number"
            .left.cont2 config -state normal -label "Reed Stiffness"
            .right.cont4 config -state normal -label "Breath Noise"
            .right.cont11 config -state normal -label "Blow Position"
            .right.cont1 config -state normal -label "Vibrato Amount"
            set cont1 20.0
            set cont2 64.0
            set cont4 20.0
            set cont11 26.0
            printWhatz "ControlChange    0.0  1 " 1  $cont1
            printWhatz "ControlChange    0.0  1 " 2  $cont2
            printWhatz "ControlChange    0.0  1 " 4  $cont4
            printWhatz "ControlChange    0.0  1 " 11 $cont11
        }
        if {$program==3}	{ # Flute
            .pretty config -bitmap @$bitmappath/KFloot.xbm
            .left.bPressure config -state normal -label "Breath Pressure"
            .left.pitch config -state normal -label "MIDI Note Number"
            .left.cont2 config -state normal -label "Embouchure Adjustment"
            .right.cont4 config -state normal -label "Breath Noise"
            .right.cont11 config -state normal -label "Vibrato Rate"
            .right.cont1 config -state normal -label "Vibrato Amount"
            set cont1 20.0
            set cont2 64.0
            set cont4 20.0
            set cont11 64.0
            printWhatz "ControlChange    0.0  1 " 1  $cont1
            printWhatz "ControlChange    0.0  1 " 2  $cont2
            printWhatz "ControlChange    0.0  1 " 4  $cont4
            printWhatz "ControlChange    0.0  1 " 11 $cont11
        }
        if {$program==4}	{ # Brass
            .pretty config -bitmap @$bitmappath/KHose.xbm
            .left.bPressure config -state normal -label "Breath Pressure"
            .left.pitch config -state normal -label "MIDI Note Number"
            .left.cont2 config -state normal -label "Lip Adjustment"
            .right.cont4 config -state normal -label "Slide Length"
            .right.cont11 config -state normal -label "Vibrato Rate"
            .right.cont1 config -state normal -label "Vibrato Amount"
            set cont1 0.0
            set cont2 64.0
            set cont4 20.0
            set cont11 64.0
            set press 80.0
            printWhatz "ControlChange    0.0  1 " 1  $cont1
            printWhatz "ControlChange    0.0  1 " 2  $cont2
            printWhatz "ControlChange    0.0  1 " 4  $cont4
            printWhatz "ControlChange    0.0  1 " 11 $cont11
            puts [format "NoteOn           0.0  1  %3.2f  %3.2f" $pitch $press]
        }
        if {$program==5}	{ # Bottle
            .pretty config -bitmap @$bitmappath/prcFunny.xbm
            .left.bPressure config -state normal -label "Breath Pressure"
            .left.pitch config -state normal -label "MIDI Note Number"
            .left.cont2 config -state disabled -label "Disabled"
            .right.cont4 config -state normal -label "Breath Noise"
            .right.cont11 config -state normal -label "Vibrato Rate"
            .right.cont1 config -state normal -label "Vibrato Amount"
            set cont1 20.0
            set cont4 20.0
            set cont11 64.0
            printWhatz "ControlChange    0.0  1 " 1  $cont1
            printWhatz "ControlChange    0.0  1 " 4  $cont4
            printWhatz "ControlChange    0.0  1 " 11 $cont11
        }
        if {$program==6}	{ # Bowed String
            .pretty config -bitmap @$bitmappath/KFiddl.xbm
            .left.bPressure config -state normal -label "Volume"
            .left.pitch config -state normal -label "MIDI Note Number"
            .left.cont2 config -state normal -label "Bow Pressure"
            .right.cont4 config -state normal -label "Bow Position"
            .right.cont11 config -state normal -label "Vibrato Rate"
            .right.cont1 config -state normal -label "Vibrato Amount"
            set cont1 4.0
            set cont2 64.0
            set cont4 24.0
            set cont11 64.0
            printWhatz "ControlChange    0.0  1 " 1  $cont1
            printWhatz "ControlChange    0.0  1 " 2  $cont2
            printWhatz "ControlChange    0.0  1 " 4  $cont4
            printWhatz "ControlChange    0.0  1 " 11 $cont11
        }
        if {$program==7}	{ # Yer Basic Pluck
            .pretty config -bitmap @$bitmappath/KPluk.xbm
            .left.bPressure config -state normal -label "Pluck Strength"
            .left.pitch config -state normal -label "MIDI Note Number"
            .left.cont2 config -state disabled -label "Disabled"
            .right.cont4 config -state disabled -label "Disabled"
            .right.cont11 config -state disabled -label "Disabled"
            .right.cont1 config -state disabled -label "Disabled"
        }
        if {$program==8}	{ # Stiff String
            .pretty config -bitmap @$bitmappath/KPluk.xbm
            .left.bPressure config -state normal -label "Pluck Strength"
            .left.pitch config -state normal -label "MIDI Note Number"
            .left.cont2 config -state disabled -label "Disabled"
            .right.cont4 config -state normal -label "Pickup Position"
            .right.cont11 config -state normal -label "String Sustain"
            .right.cont1 config -state normal -label "String Stretch"
            set cont1 10.0
            set cont4 64.0
            set cont11 96.0
            printWhatz "ControlChange    0.0  1 " 1  $cont1
            printWhatz "ControlChange    0.0  1 " 4  $cont4
            printWhatz "ControlChange    0.0  1 " 11 $cont11
        }
        if {$program==9}	{ # Sitar
            .pretty config -bitmap @$bitmappath/KPluk.xbm
            .left.bPressure config -state normal -label "Pluck Strength"
            .left.pitch config -state normal -label "MIDI Note Number"
            .left.cont2 config -state disabled -label "Disabled"
            .right.cont4 config -state disabled -label "Disabled"
            .right.cont11 config -state disabled -label "Disabled"
            .right.cont1 config -state disabled -label "Disabled"
        }
        if {$program==10}	{ # Mandolin
            .pretty config -bitmap @$bitmappath/KPluk.xbm
            .left.bPressure config -state normal -label "Microphone Position and Gain"
            .left.pitch config -state normal -label "MIDI Note Number"
            .left.cont2 config -state normal -label "Mandolin Body Size"
            .right.cont4 config -state normal -label "Pick Position"
            .right.cont11 config -state normal -label "String Sustain"
            .right.cont1 config -state normal -label "String Detune"
            set cont1 10.0
            set cont2 64.0
            set cont4 64.0
            set cont11 96.0
            set press 64.0
            printWhatz "ControlChange    0.0  1 " 1  $cont1
            printWhatz "ControlChange    0.0  1 " 2  $cont2
            printWhatz "ControlChange    0.0  1 " 4  $cont4
            printWhatz "ControlChange    0.0  1 " 11 $cont11
            changePress $press
        }
        if {$program>=11 && $program <=16}	{ # FM Instruments
            .pretty config -bitmap @$bitmappath/KFMod.xbm
            .left.bPressure config -state normal -label "ADSR 2 and 4 Targets"
            .left.pitch config -state normal -label "MIDI Note Number"
            .left.cont2 config -state normal -label "Modulator Index"
            .right.cont4 config -state normal -label "FM Pair Crossfader"
            .right.cont11 config -state normal -label "Vibrato Rate"
            .right.cont1 config -state normal -label "Vibrato Amount"
        }
        if {$program>=17 && $program <=18}	{ # FM Voices or Voice Formants
            .pretty config -bitmap @$bitmappath/KVoiceFM.xbm
            .left.bPressure config -state normal -label "Loudness (Spectral Tilt)"
            .left.pitch config -state normal -label "MIDI Note Number"
            .right.cont4 config -state normal -label "Vowel (Bass, Tenor, Alto, Sop.)"
            .right.cont11 config -state normal -label "Vibrato Rate"
            .right.cont1 config -state normal -label "Vibrato Amount"
            if {$program == 17} {
                .left.cont2 config -state normal -label "Formant Q"
            }
            if {$program == 18} {
                .left.cont2 config -state normal -label "Voiced/Unvoiced Mix"
            }
            set cont1 26.0
            printWhatz "ControlChange    0.0  1 " 1  $cont1
        }
        if {$program==19}	{ # Moog
            .pretty config -bitmap @$bitmappath/prcFunny.xbm
            .left.bPressure config -state normal -label "Volume"
            .left.pitch config -state normal -label "MIDI Note Number"
            .left.cont2 config -state normal -label "Filter Q"
            .right.cont4 config -state normal -label "Filter Sweep Rate"
            .right.cont11 config -state normal -label "Vibrato Rate"
            .right.cont1 config -state normal -label "Vibrato Amount"
            printWhatz "ControlChange    0.0  1 " 1  $cont1
            printWhatz "ControlChange    0.0  1 " 2  $cont2
            printWhatz "ControlChange    0.0  1 " 4  $cont4
            printWhatz "ControlChange    0.0  1 " 11 $cont11
        }
        if {$program==20}	{ # Simple
            .pretty config -bitmap @$bitmappath/prcFunny.xbm
            .left.bPressure config -state normal -label "Volume"
            .left.pitch config -state normal -label "MIDI Note Number"
            .left.cont2 config -state normal -label "Pole Position"
            .right.cont4 config -state normal -label "Noise/Pitched Cross-Fade"
            .right.cont11 config -state normal -label "Envelope Rate"
            .right.cont1 config -state disabled -label "Disabled"
            set cont2 64.0
            set cont4 80.0
            printWhatz "ControlChange    0.0  1 " 2  $cont2
            printWhatz "ControlChange    0.0  1 " 4  $cont4
            printWhatz "ControlChange    0.0  1 " 11 $cont11
        }
        if {$program==21}	{ # Drum Kit
            # Given the vastly different interface for the Drum Kit, we open
            # a new GUI popup window with the appropriate controls and lock
            # focus there until the user hits the "Close" button.  We then
            # switch back to the Clarinet (0) instrument.
            global p
            toplevel $p
            wm title $p "STK DrumKit"
            $p config -bg black
            wm resizable $p 0 0
            grab $p
            scale $p.velocity -from 0 -to 128 -length 100 \
                -variable velocity -orient horizontal -label "Velocity" \
                -tickinterval 64 -showvalue true -bg grey66
            pack $p.velocity -pady 5 -padx 5
            # Configure buttons
            frame $p.buttons -bg black
            frame $p.buttons.left -bg black
            frame $p.buttons.right -bg black

            button $p.buttons.left.bass -text Bass  -bg grey66 \
                -command { playDrum 36 } -width 7
            button $p.buttons.left.snare -text Snare  -bg grey66 \
                -command { playDrum 38 } -width 7
            button $p.buttons.left.tomlo -text LoTom  -bg grey66 \
                -command { playDrum 41 } -width 7
            button $p.buttons.left.tommid -text MidTom  -bg grey66 \
                -command { playDrum 45 } -width 7
            button $p.buttons.left.tomhi -text HiTom  -bg grey66 \
                -command { playDrum 50 } -width 7
            button $p.buttons.left.homer -text Homer -bg grey66 \
                -command { playDrum 90 } -width 7
            button $p.buttons.right.hat -text Hat  -bg grey66 \
                -command { playDrum 42 } -width 7
            button $p.buttons.right.ride -text Ride -bg grey66 \
                -command { playDrum 46 } -width 7
            button $p.buttons.right.crash -text Crash -bg grey66 \
                -command { playDrum 49 } -width 7
            button $p.buttons.right.cowbel -text CowBel -bg grey66 \
                -command { playDrum 56 } -width 7
            button $p.buttons.right.tamb -text Tamb -bg grey66 \
                -command { playDrum 54 } -width 7
            button $p.buttons.right.homer -text Homer -bg grey66 \
                -command { playDrum 90 } -width 7

            pack $p.buttons.left.bass -pady 5
            pack $p.buttons.left.snare -pady 5
            pack $p.buttons.left.tomlo -pady 5
            pack $p.buttons.left.tommid -pady 5
            pack $p.buttons.left.tomhi -pady 5
            pack $p.buttons.left.homer -pady 5
            pack $p.buttons.right.hat -pady 5
            pack $p.buttons.right.ride -pady 5
            pack $p.buttons.right.crash -pady 5
            pack $p.buttons.right.cowbel -pady 5
            pack $p.buttons.right.tamb -pady 5
            pack $p.buttons.right.homer -pady 5

            pack $p.buttons.left -side left -pady 5 -padx 5
            pack $p.buttons.right -side right -pady 5 -padx 5
            pack $p.buttons -padx 5 -pady 10

            set temp $oldpatch
            button $p.close -text "Close" -bg grey66 \
								-command { destroy $p
                    set patchnum $temp
                    patchChange $patchnum}
            pack $p.close -side bottom -padx 5 -pady 10
        }
        if {$program==22}	{ # Banded Waveguide Instruments
            .pretty config -bitmap @$bitmappath/prcFunny.xbm
            .left.bPressure config -state normal -label "Strike/Bow Velocity"
            .left.pitch config -state normal -label "MIDI Note Number"
            .left.cont2 config -state normal -label "Bowing Pressure (0 = Strike)"
            .right.cont4 config -state normal -label "Bow Motion"
            .right.cont11 config -state normal -label "Integration Control"
            .right.cont1 config -state normal -label "Mode Resonance"
            set preset [expr $value-2200]
            set press 100.0
            set cont1 127.0
            set cont2 0.0
            set cont4 0.0
            set cont11 0.0
            puts [format "ControlChange    0.0  1  16  %3.2f" $preset]
            puts [format "NoteOn          0.0  1  %3.2f  %3.2f" $pitch $press]
            printWhatz "ControlChange    0.0  1 " 1  $cont1
            printWhatz "ControlChange    0.0  1 " 2  $cont2
            printWhatz "ControlChange    0.0  1 " 11 $cont11
            flush stdout
        }
        if {$program==23}	{ # Shakers
            .pretty config -bitmap @$bitmappath/phism.xbm
            .left.bPressure config -state normal -label "Shake Energy"
            .left.pitch config -state disabled -label "Disabled"
            .left.cont2 config -state disabled -label "Disabled"
            .right.cont4 config -state normal -label "Number of Objects"
            .right.cont11 config -state normal -label "(<--High) Damping (Low-->)"
            .right.cont1 config -state normal -label "Resonance Center Frequency"
            set pitch [expr $value-2300]
            switch $pitch {
                0 {
                    .pretty config -bitmap @$bitmappath/maraca.xbm
                }
                1 {
                    .pretty config -bitmap @$bitmappath/cabasa.xbm
                }
                3 {
                    .pretty config -bitmap @$bitmappath/tambourine.xbm
                }
                4 {
                    .pretty config -bitmap @$bitmappath/sleighbell.xbm
                }
                5 {
                    .pretty config -bitmap @$bitmappath/bamboo.xbm
                }
                7 {
                    .pretty config -bitmap @$bitmappath/cokecan.xbm
                }
                19 {
                    .pretty config -bitmap @$bitmappath/guiro.xbm
                }
                20 {
                    .pretty config -bitmap @$bitmappath/rachet.xbm
                }
                22 {
                    .pretty config -bitmap @$bitmappath/bamboo.xbm
                }
            }
            set cont1 64.0
            set cont2 64.0
            set cont4 64.0
            set cont11 64.0
            puts [format "NoteOn          0.0  1  %3.2f  %3.2f" $pitch $press]
            printWhatz "ControlChange    0.0  1 " 1  $cont1
            printWhatz "ControlChange    0.0  1 " 2  $cont2
            printWhatz "ControlChange    0.0  1 " 4  $cont4
            printWhatz "ControlChange    0.0  1 " 11 $cont11
            flush stdout
        }
        if {$program==24}	{ # Modal Instruments
            .pretty config -bitmap @$bitmappath/KModal.xbm
            .left.bPressure config -state normal -label "Strike Vigor"
            .left.pitch config -state normal -label "MIDI Note Number"
            .left.cont2 config -state normal -label "Stick Hardness"
            .right.cont4 config -state normal -label "Stick Position"
            set preset [expr $value-2400]
            if {$preset == 1} {
                .right.cont11 config -state normal -label "Vibrato Rate"
            } else {
                .right.cont11 config -state disabled -label "Disabled"
            }
            .right.cont1 config -state normal -label "Direct Stick Mix"
            set cont1 20.0
            set cont2 64.0
            set cont4 64.0
            set cont11 64.0
            puts [format "ControlChange    0.0  1  16  %3.2f" $preset]
            printWhatz "ControlChange    0.0  1 " 1  $cont1
            printWhatz "ControlChange    0.0  1 " 2  $cont2
            printWhatz "ControlChange    0.0  1 " 4  $cont4
            printWhatz "ControlChange    0.0  1 " 11 $cont11
            flush stdout
        }
        if { $program==25 }	{ # Mesh2D
            .pretty config -bitmap @$bitmappath/prcFunny.xbm
            .left.bPressure config -state normal -label "Strike Vigor"
            .left.pitch config -state disabled -label "Disabled"
            .left.cont2 config -state normal -label "X Dimension"
            .right.cont4 config -state normal -label "Y Dimension"
            .right.cont11 config -state normal -label "Mesh Decay"
            .right.cont1 config -state normal -label "X-Y Input Position"
            set cont1 0.0
            set cont2 96.0
            set cont4 120.0
            set cont11 64.0
            printWhatz "ControlChange    0.0  1 " 1  $cont1
            printWhatz "ControlChange    0.0  1 " 2  $cont2
            printWhatz "ControlChange    0.0  1 " 4  $cont4
            printWhatz "ControlChange    0.0  1 " 11 $cont11
            flush stdout
        }
        if { $program==26 }	{ # Resonate
            .pretty config -bitmap @$bitmappath/prcFunny.xbm
            .left.bPressure config -state normal -label "Gain"
            .left.pitch config -state disabled -label "Disabled"
            .left.cont2 config -state normal -label "Resonance Frequency"
            .right.cont4 config -state normal -label "Resonance Radius"
            .right.cont11 config -state normal -label "Notch Frequency"
            .right.cont1 config -state normal -label "Notch Radius"
            set cont2 20.0
            set cont4 120.0
            set cont11 64.0
            set cont1 0.0
            printWhatz "ControlChange    0.0  1 " 4  $cont4
            printWhatz "ControlChange    0.0  1 " 11 $cont11
            printWhatz "ControlChange    0.0  1 " 1  $cont1
            printWhatz "ControlChange    0.0  1 " 2  $cont2
            flush stdout
        }
        if { $program==27 }	{ # Whistle
            .pretty config -bitmap @$bitmappath/prcFunny.xbm
            .left.bPressure config -state normal -label "Gain"
            .left.pitch config -state normal -label "Whistle Pitch"
            .left.cont2 config -state normal -label "Blowing Modulation"
            .right.cont4 config -state normal -label "Noise Gain"
            .right.cont11 config -state normal -label "Fipple Frequency Modulation"
            .right.cont1 config -state normal -label "Fipple Gain Modulation"
            set cont2 64.0
            set cont4 40.0
            set cont11 64.0
            set cont1 64.0
            printWhatz "ControlChange    0.0  1 " 4  $cont4
            printWhatz "ControlChange    0.0  1 " 11 $cont11
            printWhatz "ControlChange    0.0  1 " 1  $cont1
            printWhatz "ControlChange    0.0  1 " 2  $cont2
            flush stdout
        }
        set oldpatch $value
    }
}

bind . <Configure> {+ center_the_toplevel %W }
proc center_the_toplevel { w } {

    # Callback on the <Configure> event for a toplevel
    # that should be centered on the screen

    # Make sure that we aren't configuring a child window
    if { [string equal $w [winfo toplevel $w]] } {

        # Calculate the desired geometry
        set width [winfo reqwidth $w]
        set height [winfo reqheight $w]
        set x [expr { ( [winfo vrootwidth  $w] - $width  ) / 2 }]
        set y [expr { ( [winfo vrootheight $w] - $height ) / 2 }]
        #set y 0

        # Hand the geometry off to the window manager
        wm geometry $w ${width}x${height}+${x}+${y}

        # Unbind <Configure> so that this procedure is
        # not called again when the window manager finishes
        # centering the window.  Also, revert geometry management
        # to internal default for subsequent size changes.
        bind $w <Configure> {}
        wm geometry $w ""
    }

    return
}

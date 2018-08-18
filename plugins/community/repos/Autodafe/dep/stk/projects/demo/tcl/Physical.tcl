# Tcl/Tk Physical Model GUI for the Synthesis Toolkit (STK)

# Set initial control values
set pitch 64.0
set press 64.0
set cont1 0.0
set cont2 20.0
set cont4 64.0
set cont7 128.0
set cont11 64.0
set cont44 24.0
set oldpatch 0
set patchnum 0

# Configure main window
wm title . "STK Physical Model Controller"
wm iconname . "physical"
. config -bg black

# Configure patch change buttons
frame .radios1 -bg black
frame .radios2 -bg black

radiobutton .radios1.clar -text "Clarinet" -bg grey66 \
  -variable patchnum -value 0 -command { patchChange $patchnum }
radiobutton .radios1.hole -text "BlowHole" -bg grey66 \
  -variable patchnum -value 1 -command { patchChange $patchnum }
radiobutton .radios1.fony -text "Saxofony" -bg grey66 \
  -variable patchnum -value 2 -command { patchChange $patchnum }
radiobutton .radios1.flut -text "Flute" -bg grey66 \
  -variable patchnum -value 3 -command { patchChange $patchnum }
radiobutton .radios1.bras -text "Brass" -bg grey66 \
  -variable patchnum -value 4 -command { patchChange $patchnum }
radiobutton .radios1.botl -text "BlowBotl" -bg grey66 \
  -variable patchnum -value 5 -command { patchChange $patchnum }
radiobutton .radios2.bowd -text "Bowed" -bg grey66 \
  -variable patchnum -value 6 -command { patchChange $patchnum }
radiobutton .radios2.pluk -text "Plucked" -bg grey66 \
  -variable patchnum -value 7 -command { patchChange $patchnum }
radiobutton .radios2.karp -text "StifKarp" -bg grey66 \
  -variable patchnum -value 8 -command { patchChange $patchnum }
radiobutton .radios2.sitr -text "Sitar" -bg grey66 \
  -variable patchnum -value 9 -command { patchChange $patchnum }
radiobutton .radios2.mand -text "Mandolin" -bg grey66 \
  -variable patchnum -value 10 -command { patchChange $patchnum }

pack .radios1.clar -side left -padx 5 -pady 10
pack .radios1.hole -side left -padx 5 -pady 10
pack .radios1.fony -side left -padx 5 -pady 10
pack .radios1.flut -side left -padx 5 -pady 10
pack .radios1.bras -side left -padx 5 -pady 10
pack .radios1.botl -side left -padx 5 -pady 10
pack .radios2.bowd -side left -padx 5 -pady 10
pack .radios2.pluk -side left -padx 5 -pady 10
pack .radios2.karp -side left -padx 5 -pady 10
pack .radios2.sitr -side left -padx 5 -pady 10
pack .radios2.mand -side left -padx 5 -pady 10

pack .radios1
pack .radios2

# Configure bitmap display
if {[file isdirectory bitmaps]} {
    set bitmappath bitmaps
} else {
    set bitmappath tcl/bitmaps
}
button .pretty -bitmap @$bitmappath/Klar.xbm \
    -background white -foreground black
pack .pretty -padx 5 -pady 10

# Configure "note-on" buttons
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
    -command {printWhatz "ControlChange    0.0 1 " 2} \
    -orient horizontal -label "Reed Stiffness" \
    -tickinterval 32 -showvalue true -bg grey66  \
    -variable cont2

scale .right.reverb -from 0 -to 128 -length 200 \
    -command {printWhatz "ControlChange    0.0  1 " 44} \
    -orient horizontal -label "Reverb Mix" \
    -tickinterval 32 -showvalue true -bg grey66  \
    -variable cont44

scale .right.cont4 -from 0 -to 128 -length 200 \
    -command {printWhatz "ControlChange    0.0 1 " 4} \
    -orient horizontal -label "Breath Noise" \
    -tickinterval 32 -showvalue true -bg grey66  \
    -variable cont4

scale .right.cont11 -from 0 -to 128 -length 200 \
    -command {printWhatz "ControlChange    0.0 1 " 11} \
    -orient horizontal -label "Vibrato Rate" \
    -tickinterval 32 -showvalue true -bg grey66  \
    -variable cont11 

scale .right.cont1 -from 0 -to 128 -length 200 \
    -command {printWhatz "ControlChange    0.0 1 " 1} \
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

bind . <KeyPress> { noteOn $pitch $press }

# Bind an X windows "close" event with the Exit routine
bind . <Destroy> +myExit

proc myExit {} {
    global pitch
    puts [format "ExitProgram"]
    flush stdout
    close stdout
    exit
}

proc noteOn {pitchVal pressVal} {
    puts [format "NoteOn           0.0 1 %f %f" $pitchVal $pressVal]
    flush stdout
}

proc noteOff {pitchVal pressVal} {
    puts [format "NoteOff          0.0 1 %f %f" $pitchVal $pressVal]
    flush stdout
}

proc patchChange {value} {
    global bitmappath cont1 cont2 cont4 cont11 pitch oldpatch
    puts [format "ProgramChange    0.0 1 %i" $value]
        if {$value==0}	{ # Clarinet
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
        if {$value==1}	{ # BlowHole
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
        if {$value==2}	{ # Saxofony
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
        if {$value==3}	{ # Flute
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
        if {$value==4}	{ # Brass
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
        if {$value==5}	{ # Bottle
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
        if {$value==6}	{ # Bowed String
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
        if {$value==7}	{ # Yer Basic Pluck
            .pretty config -bitmap @$bitmappath/KPluk.xbm
            .left.bPressure config -state normal -label "Pluck Strength"
            .left.pitch config -state normal -label "MIDI Note Number"
            .left.cont2 config -state disabled -label "Disabled"
            .right.cont4 config -state disabled -label "Disabled"
            .right.cont11 config -state disabled -label "Disabled"
            .right.cont1 config -state disabled -label "Disabled"
        }
        if {$value==8}	{ # Stiff String
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
        if {$value==9}	{ # Sitar
            .pretty config -bitmap @$bitmappath/KPluk.xbm
            .left.bPressure config -state normal -label "Pluck Strength"
            .left.pitch config -state normal -label "MIDI Note Number"
            .left.cont2 config -state disabled -label "Disabled"
            .right.cont4 config -state disabled -label "Disabled"
            .right.cont11 config -state disabled -label "Disabled"
            .right.cont1 config -state disabled -label "Disabled"
        }
        if {$value==10}	{ # Mandolin
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
        set oldpatch $value
}

proc printWhatz {tag value1 value2 } {
    puts [format "%s %i %f" $tag $value1 $value2]
    flush stdout
}

proc changePress {value} {
    global patchnum
    if { $patchnum<7 || $patchnum>9 } {
      puts [format "AfterTouch       0.0 1 %f" $value]
      flush stdout
    }
}

proc changePitch {value} {
    puts [format "PitchChange      0.0 1 %.3f" $value]
    flush stdout
}

bind . <Configure> { center_the_toplevel %W }
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
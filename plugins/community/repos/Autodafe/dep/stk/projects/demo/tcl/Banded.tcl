# Tcl/Tk Bowed Bar Model GUI for the Synthesis Toolkit (STK)

set press 64.0
set pitch 64.0
set cont1 127.0
set cont2 0.0
set cont4 0.0
set cont11 0.0
set struckbow 0
set preset 0
 
# Configure main window
wm title . "STK Bowed Bar Controller"
wm iconname . "bowedbar"
. config -bg black

# Configure preset radio buttons
frame .radio1 -bg black

radiobutton .radio1.0 -text Bar -bg grey66  \
    -command {printWhatz "ControlChange    0.0 1 " 16 0} \
    -variable preset -value 0
radiobutton .radio1.1 -text Marimba -bg grey66  \
    -command {printWhatz "ControlChange    0.0 1 " 16 1} \
    -variable preset -value 1
radiobutton .radio1.2 -text GlassHarmonica -bg grey66  \
    -command {printWhatz "ControlChange    0.0 1 " 16 2} \
    -variable preset -value 2
radiobutton .radio1.3 -text PrayerBowl -bg grey66  \
    -command {printWhatz "ControlChange    0.0 1 " 16 3} \
    -variable preset -value 3

pack .radio1.0 -side left -padx 5
pack .radio1.1 -side left -padx 5 -pady 10
pack .radio1.2 -side left -padx 5 -pady 10
pack .radio1.3 -side left -padx 5 -pady 10
pack .radio1

# Configure message box
label .note -font {Times 10 normal} -background white \
    -foreground darkred -relief raised -height 4 \
    -wraplength 300 -width 60 \
    -text "To strike, set the Bow Pressure to zero and hit NoteOn or the spacebar.  To bow, use the 'Bow Velocity' slider to set a fixed velocity or move the 'Bow Motion' slider as if it were the bow (with a non-zero Bow Pressure)."
pack .note -padx 5 -pady 10

# Configure "note-on" buttons
frame .noteOn -bg black
button .noteOn.on -text NoteOn -bg grey66 -command { noteOn $pitch $press }
button .noteOn.off -text NoteOff -bg grey66 -command { noteOff $pitch 127.0 }
button .noteOn.exit -text "Exit Program" -bg grey66 -command myExit
pack .noteOn.on -side left -padx 5
pack .noteOn.off -side left -padx 5 -pady 10
pack .noteOn.exit -side left -padx 5 -pady 10
pack .noteOn -pady 10

# Configure sliders
frame .left -bg black
frame .right -bg black

scale .left.bPressure -from 0 -to 128 -length 200 \
-command {changePress } -variable press\
-orient horizontal -label "Strike/Bow Velocity" \
-tickinterval 32 -showvalue true -bg grey66

scale .left.pitch -from 0 -to 128 -length 200 \
-command {changePitch } -variable pitch \
-orient horizontal -label "MIDI Note Number" \
-tickinterval 32 -showvalue true -bg grey66

scale .left.cont2 -from 0 -to 128 -length 200 \
-command {printWhatz "ControlChange    0.0 1 " 2} \
-orient horizontal -label "Bowing Pressure (0 = Strike)" \
-tickinterval 32 -showvalue true -bg grey66  \
-variable cont2

scale .right.cont4 -from 0 -to 128 -length 200 \
-command {printWhatz "ControlChange    0.0 1 " 4} \
-orient horizontal -label "Bowing Motion" \
-tickinterval 32 -showvalue true -bg grey66  \
-variable cont4

scale .right.cont11 -from 0 -to 128 -length 200 \
-command {printWhatz "ControlChange    0.0 1 " 11} \
-orient horizontal -label "Integration" \
-tickinterval 32 -showvalue true -bg grey66  \
-variable cont11 

scale .right.reson -from 0 -to 128 -length 200 \
-command {printWhatz "ControlChange    0.0 1 " 1} \
-orient horizontal -label "Mode Resonance" \
-tickinterval 32 -showvalue true -bg grey66  \
-variable cont1

. config -bg grey20

pack .left.bPressure -padx 10 -pady 10
pack .left.pitch	-padx 10 -pady 10
pack .left.cont2	-padx 10 -pady 10
pack .right.cont4	-padx 10 -pady 10
pack .right.cont11 	-padx 10 -pady 10
pack .right.reson 	-padx 10 -pady 10

pack .left -side left
pack .right -side right

bind all <KeyPress> { noteOn $pitch $press }

# Bind an X windows "close" event with the Exit routine
bind . <Destroy> +myExit

proc myExit {} {
    global pitch
    puts [format "NoteOff          0.0 1 %f 127" $pitch ]
    flush stdout
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
    global patch
    set patch $value
    puts [format "ProgramChange    0.0 1 %i" $value]
    flush stdout
}

proc printWhatz {tag value1 value2 } {
    puts [format "%s %i %f" $tag $value1 $value2]
    flush stdout
}

proc changePress {value} {
    puts [format "AfterTouch       0.0 1 %f" $value]
    flush stdout
}

proc changePitch {value} {
    puts [format "PitchChange      0.0 1 %.3f" $value]
    flush stdout
}

proc activateVel {} {
  global pitch
  noteOn $pitch 127
  printWhatz "ControlChange    0.0 1 " 65  0
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

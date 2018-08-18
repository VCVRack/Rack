set pitch 64.0
set press 64.0
set cont1 0.0
set cont2 64.0
set cont4 64.0
set cont7 128.0
set cont11 64.0
set cont44 24.0
set preset 0

# Configure main window
wm title . "STK Modal Bar Interface"
wm iconname . "modal"
. config -bg black

# Configure preset radio buttons
frame .radio1 -bg black
frame .radio2 -bg black

radiobutton .radio1.0 -text Marimba -bg grey66  \
    -variable preset -value 0 -command { patchChange $preset }
radiobutton .radio1.1 -text Vibraphone -bg grey66  \
    -variable preset -value 1 -command { patchChange $preset }
radiobutton .radio1.2 -text Agogo -bg grey66  \
    -variable preset -value 2 -command { patchChange $preset }
radiobutton .radio1.3 -text Wood1 -bg grey66  \
    -variable preset -value 3 -command { patchChange $preset }
radiobutton .radio2.4 -text Reso -bg grey66 \
    -variable preset -value 4 -command { patchChange $preset }
radiobutton .radio2.5 -text Wood2 -bg grey66 \
    -variable preset -value 5 -command { patchChange $preset }
radiobutton .radio2.6 -text Beats -bg grey66 \
    -variable preset -value 6 -command { patchChange $preset }
radiobutton .radio2.7 -text 2Fix -bg grey66 \
    -variable preset -value 7 -command { patchChange $preset }
radiobutton .radio2.8 -text Clump -bg grey66 \
    -variable preset -value 8 -command { patchChange $preset }

pack .radio1.0 -side left -padx 5
pack .radio1.1 -side left -padx 5 -pady 10
pack .radio1.2 -side left -padx 5 -pady 10
pack .radio1.3 -side left -padx 5 -pady 10
pack .radio1
pack .radio2.4 -side left -padx 5
pack .radio2.5 -side left -padx 5
pack .radio2.6 -side left -padx 5 -pady 10
pack .radio2.7 -side left -padx 5 -pady 10
pack .radio2.8 -side left -padx 5 -pady 10
pack .radio2

# Configure bitmap display
if {[file isdirectory bitmaps]} {
    set bitmappath bitmaps
} else {
    set bitmappath tcl/bitmaps
}

button .pretty -bitmap @$bitmappath/KModal.xbm \
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
    -orient horizontal -label "Strike Vigor" \
    -tickinterval 32 -showvalue true -bg grey66

scale .left.pitch -from 0 -to 128 -length 200 \
    -command {changePitch } -variable pitch \
    -orient horizontal -label "MIDI Note Number" \
    -tickinterval 32 -showvalue true -bg grey66

scale .left.cont2 -from 0 -to 128 -length 200 \
    -command {printWhatz "ControlChange    0.0  1 " 2} \
    -orient horizontal -label "Stick Hardness" \
    -tickinterval 32 -showvalue true -bg grey66  \
    -variable cont2

scale .right.reverb -from 0 -to 128 -length 200 \
    -command {printWhatz "ControlChange    0.0  1 " 44} \
    -orient horizontal -label "Reverb Mix" \
    -tickinterval 32 -showvalue true -bg grey66  \
    -variable cont44

scale .right.cont4 -from 0 -to 128 -length 200 \
    -command {printWhatz "ControlChange    0.0  1 " 4} \
    -orient horizontal -label "Stick Position" \
    -tickinterval 32 -showvalue true -bg grey66  \
    -variable cont4

scale .right.cont11 -from 0 -to 128 -length 200 \
    -command {printWhatz "ControlChange    0.0  1 " 11} \
    -orient horizontal -label "Disabled" \
    -tickinterval 32 -showvalue true -bg grey66  \
    -variable cont11 -state disabled

scale .right.cont1 -from 0 -to 128 -length 200 \
    -command {printWhatz "ControlChange    0.0  1 " 1} \
    -orient horizontal -label "Direct Stick Mix" \
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

#bind all <KeyPress> {
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
    puts [format "NoteOn           0.0  1  %3.2f  %3.2f" $pitchVal $pressVal]
    flush stdout
}

proc noteOff {pitchVal pressVal} {
    puts [format "NoteOff          0.0  1  %3.2f %3.2f" $pitchVal $pressVal]
    flush stdout
}

proc patchChange {value} {
    global preset
    if {$preset == 1} {
        .right.cont11 config -state normal -label "Vibrato Rate"
    } else {
        .right.cont11 config -state disabled -label "Disabled"
    }
    printWhatz "ControlChange    0.0 1 " 16  $preset
}

proc printWhatz {tag value1 value2 } {
    puts [format "%s %2i  %3.2f" $tag $value1 $value2]
    flush stdout
}

proc changePress {value} {
    puts [format "AfterTouch       0.0  1  %3.2f" $value]
    flush stdout
}

proc changePitch {value} {
    puts [format "PitchChange      0.0  1  %3.2f" $value]
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



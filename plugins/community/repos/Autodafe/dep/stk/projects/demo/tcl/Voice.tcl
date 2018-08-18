# Tcl/Tk Voice GUI for the Synthesis Toolkit (STK)

# Set initial control values
set pitch 64.0
set press 64.0
set cont1 20.0
set cont2 64.0
set cont4 64.0
set cont11 64.0
set patchnum 17

# Configure main window
wm title . "STK Voice Model Controller"
wm iconname . "voice"
. config -bg black

# Configure patch change buttons
frame .instChoice -bg black

radiobutton .instChoice.fm -text "FMVoice" -bg grey66  \
				-command { patchChange 17 } -value 17 -variable patchnum
radiobutton .instChoice.form -text "Formant" -bg grey66 \
				-command { patchChange 18 } -value 18 -variable patchnum

pack .instChoice.fm -side left -padx 5
pack .instChoice.form -side left -padx 5 -pady 10

pack .instChoice -side top

# Configure bitmap display
if {[file isdirectory bitmaps]} {
		set bitmappath bitmaps
} else {
		set bitmappath tcl/bitmaps
}
button .pretty -bitmap @$bitmappath/KVoiceFM.xbm \
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

scale .left.bPressure -from 0 -to 128 -length 200 \
    -command {changePress } -variable press \
    -orient horizontal -label "Loudness (Spectral Tilt)" \
    -tickinterval 32 -showvalue true -bg grey66

scale .left.pitch -from 0 -to 128 -length 200 \
    -command {changePitch } -variable pitch \
    -orient horizontal -label "MIDI Note Number" \
    -tickinterval 32 -showvalue true -bg grey66

scale .left.cont1 -from 0 -to 128 -length 200 \
    -command {printWhatz "ControlChange    -1.0 1 " 2} \
    -orient horizontal -label "Formant Q / Voiced/Un." \
    -tickinterval 32 -showvalue true -bg grey66 \
    -variable cont2

scale .right.cont2 -from 0 -to 128 -length 200 \
    -command {printWhatz "ControlChange    -1.0 1 " 4} \
    -orient horizontal -label "Vowel (Bass, Tenor, Alto, Sop.)" \
    -tickinterval 32 -showvalue true -bg grey66 \
    -variable cont4

scale .right.cont3 -from 0 -to 128 -length 200 \
    -command {printWhatz "ControlChange    -1.0 1 " 11} \
    -orient horizontal -label "Vibrato Rate" \
    -tickinterval 32 -showvalue true -bg grey66 \
    -variable cont11

scale .right.vibrato -from 0 -to 128 -length 200 \
    -command {printWhatz "ControlChange    -1.0 1 " 1} \
    -orient horizontal -label "Vibrato Amount" \
    -tickinterval 32 -showvalue true -bg grey66\
    -variable cont1

pack .left.bPressure -padx 10 -pady 10
pack .left.pitch -padx 10 -pady 10
pack .left.cont1 -padx 10 -pady 10
pack .right.cont2 -padx 10 -pady 10
pack .right.cont3 -padx 10 -pady 10
pack .right.vibrato -padx 10 -pady 10

pack .left -side left
pack .right -side right

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
		global bitmappath cont1 cont2 cont4 cont11
    puts [format "ProgramChange    0.0 1 %i" $value]
    if {$value==16}	{
				.pretty config -bitmap @$bitmappath/KVoiceFM.xbm
    }
    if {$value==17}	{
				.pretty config -bitmap @$bitmappath/KVoicForm.xbm
    }
    flush stdout
    set cont1 0.0
    set cont2 20.0
    set cont4 64.0
    set cont11 64.0
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
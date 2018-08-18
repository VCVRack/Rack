# A simple Tcl/Tk example script

# Set initial control values
set pitch 64.0
set press 64.0

# Configure main window
wm title . "A Simple GUI"
wm iconname . "simple"
. config -bg black

# Configure a "note-on" button
frame .noteOn -bg black

button .noteOn.on -text NoteOn -bg grey66 -command { noteOn $pitch $press }
pack .noteOn.on -side left -padx 5
pack .noteOn

# Configure sliders
frame .slider -bg black

scale .slider.pitch -from 0 -to 128 -length 200 \
-command {changePitch } -variable pitch \
-orient horizontal -label "MIDI Note Number" \
-tickinterval 32 -showvalue true -bg grey66

pack .slider.pitch -padx 10 -pady 10
pack .slider -side left

# Bind an X windows "close" event with the Exit routine
bind . <Destroy> +myExit

proc myExit {} {
    global pitch outID
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

proc changePitch {value} {
    puts [format "PitchChange      0.0 1 %.3f" $value]
    flush stdout
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

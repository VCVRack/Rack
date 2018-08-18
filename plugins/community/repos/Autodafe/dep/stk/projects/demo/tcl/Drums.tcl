# Tcl/Tk Drum GUI for the Synthesis Toolkit (STK)

# Set initial control values
set press 127

# Turn down the reverb
puts "ControlChange    0.0  1  44.0  0.0"

# Configure main window
wm title . "STK Drum Controller"
wm iconname . "drum"
. config -bg black

# Configure slider
scale .bPressure -from 0 -to 128 -length 100 \
-command {changePress } -variable press\
-orient horizontal -label "Velocity" \
-tickinterval 64 -showvalue true -bg grey66

pack .bPressure -pady 5 -padx 5

# Configure buttons
frame .buttons -bg black
frame .buttons.left -bg black
frame .buttons.right -bg black

button .buttons.left.bass -text Bass  -bg grey66 \
-command { playDrum 36 } -width 7
button .buttons.left.snare -text Snare  -bg grey66 \
-command { playDrum 38 } -width 7
button .buttons.left.tomlo -text LoTom  -bg grey66 \
-command { playDrum 41 } -width 7
button .buttons.left.tommid -text MidTom  -bg grey66 \
-command { playDrum 45 } -width 7
button .buttons.left.tomhi -text HiTom  -bg grey66 \
-command { playDrum 50 } -width 7
button .buttons.left.homer -text Homer -bg grey66 \
-command { playDrum 90 } -width 7
button .buttons.right.hat -text Hat  -bg grey66 \
-command { playDrum 42 } -width 7
button .buttons.right.ride -text Ride -bg grey66 \
-command { playDrum 46 } -width 7
button .buttons.right.crash -text Crash -bg grey66 \
-command { playDrum 49 } -width 7
button .buttons.right.cowbel -text CowBel -bg grey66 \
-command { playDrum 56 } -width 7
button .buttons.right.tamb -text Tamb -bg grey66 \
-command { playDrum 54 } -width 7
button .buttons.right.homer -text Homer -bg grey66 \
-command { playDrum 90 } -width 7

pack .buttons.left.bass -pady 5
pack .buttons.left.snare -pady 5
pack .buttons.left.tomlo -pady 5
pack .buttons.left.tommid -pady 5
pack .buttons.left.tomhi -pady 5
pack .buttons.left.homer -pady 5
pack .buttons.right.hat -pady 5
pack .buttons.right.ride -pady 5
pack .buttons.right.crash -pady 5
pack .buttons.right.cowbel -pady 5
pack .buttons.right.tamb -pady 5
pack .buttons.right.homer -pady 5

pack .buttons.left -side left -pady 5 -padx 5
pack .buttons.right -side right -pady 5 -padx 5
pack .buttons -pady 5 -padx 5

# Configure exit button
button .exit -text "Exit Program" -bg grey66 -command myExit
pack .exit -side bottom -pady 20

# Bind an X windows "close" event with the Exit routine
bind . <Destroy> +myExit

proc myExit {} {
    puts [format "ExitProgram"]
    flush stdout
    close stdout
    exit
}

proc playDrum {value}	{
    global press
    puts [format "NoteOn           0.0 1 %i %f" $value $press]
    flush stdout
}

proc changePress {value} {
    global press
    set press $value
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
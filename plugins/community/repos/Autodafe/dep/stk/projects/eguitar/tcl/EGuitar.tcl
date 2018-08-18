# Tcl/Tk Electric Guitar Model GUI for the Synthesis Toolkit (STK)
# by Gary P. Scavone, McGill University, 2012.

# Set lowest string note numbers and range
set stringRange 20
array set stringMin {
    1 40
    2 45
    3 50
    4 55
    5 59
    6 64
}
array set stringNote {
    1 40
    2 45
    3 50
    4 55
    5 59
    6 64
}
#array set stringAmp { 1 64 2 64 3 64 4 64 5 64 6 64 }
array set stringAmp {
    1 64
    2 64
    3 64
    4 64
    5 64
    6 64
}

# Set initial control values
set cont2 20.0
set cont7 100.0
set cont27 64.0
set cont28 0.0
set cont44 24.0
set cont72 64.0
set cont128 64.0
set velocity 64.0

# Configure main window
wm title . "STK Electric Guitar Model Controller"
wm iconname . "guitar"
. config -bg white

# Configure message box
label .message -font {Times 14 normal} -background white \
    -foreground darkred -relief raised \
    -wraplength 300 -width 60 \
    -text "Use the spacebar or button to strum all the strings.  Use the pulldown menu next to the velocity slider to control the velocity for individual strings."
pack .message -padx 5 -pady 10

# Configure "note on" buttons
frame .top
button .top.on -text Strum -bg grey66 -command strum
button .top.off -text "All Off" -bg grey66 -command allOff
button .top.exit -text "Quit" -bg grey66 -command quit
pack .top.on -side left -padx 5
pack .top.off -side left -padx 5 -pady 10
pack .top.exit -side left -padx 5 -pady 10
pack .top

frame .left -borderwidth 5 -relief groove -bg grey88

scale .left.volume -from 0 -to 128 -length 200 \
    -command {printWhatz "ControlChange    0.0  1 " 7} \
    -orient horizontal -label "Volume" \
    -tickinterval 32 -showvalue true \
    -variable cont7

scale .left.reverb -from 0 -to 128 -length 200 \
    -command {printWhatz "ControlChange    0.0  1 " 44} \
    -orient horizontal -label "Reverb Mix" \
    -tickinterval 32 -showvalue true \
    -variable cont44

scale .left.bridge -from 0 -to 128 -length 200 \
    -command {printWhatz "ControlChange    0.0  1 " 2} \
    -orient horizontal -label "Bridge Coupling Gain" \
    -tickinterval 32 -showvalue true -variable cont2

scale .left.fbGain -from 0 -to 128 -length 200 \
    -command {printWhatz "ControlChange    0.0 1 " 28} \
    -orient horizontal -label "Feedback Gain" \
    -tickinterval 32 -showvalue true -variable cont28

scale .left.fbDelay -from 0 -to 128 -length 200 \
    -command {printWhatz "ControlChange    0.0 1 " 27} \
    -orient horizontal -label "Feedback Delay" \
    -tickinterval 32 -showvalue true -variable cont27

scale .left.dmix -from 0 -to 128 -length 200 \
    -command {printWhatz "ControlChange    0.0 1 " 72} \
    -orient horizontal -label "Distortion Mix" \
    -tickinterval 32 -showvalue true -variable cont72

scale .left.pick -from 0 -to 128 -length 200 \
    -command {printWhatz "ControlChange    0.0 1 " 128} \
    -orient horizontal -label "Pick Hardness" \
    -tickinterval 32 -showvalue true -variable cont128

pack .left.volume -padx 10 -pady 5
pack .left.reverb -padx 10 -pady 5
pack .left.bridge -padx 10 -pady 5
pack .left.fbGain -padx 10 -pady 5
pack .left.fbDelay -padx 10 -pady 5
pack .left.dmix -padx 10 -pady 5
pack .left.pick -padx 10 -pady 10
pack .left -side left

proc quit {} {
    puts [format "ExitProgram"]
    flush stdout
    close stdout
    exit
}

proc strum {} {
    global stringNote stringAmp
    for {set n 1} {$n < 7} {incr n} {
        puts [format "NoteOn           %2.3f  %d  %3.2f  %3.2f" [expr rand()*0.04] $n $stringNote($n) $stringAmp($n)]
    }
    flush stdout
}

proc allOff {} {
    global stringNote stringAmp
    for {set n 1} {$n < 7} {incr n} {
      puts [format "NoteOff          0.0  %d  %3f %3f" $n $stringNote($n) $stringAmp($n)]
    }
    flush stdout
}

# Set bindings
bind . <KeyPress> { strum }
bind . <Destroy> +quit

proc printWhatz {tag value1 value2 } {
    puts [format "%s %2i  %3.2f" $tag $value1 $value2]
    flush stdout
}

proc pluckOne {value} {
    global stringNote stringAmp
    puts [format "NoteOn           0.0  %d  %3f  %3f" $value $stringNote($value) $stringAmp($value)]
    flush stdout
}

proc setNote {string value} {
    global stringNote
    set stringNote($string) $value
}

proc setStringAmp {value} {
    global stringAmp cbpath
    set n [$cbpath current]
    if { $n > 0 } {
        set stringAmp($n) $value
    } else {
        for {set i 1} {$i < 7} {incr i} {
            set stringAmp($i) $value
        }
    }
}

frame .strings -bg grey88 -borderwidth 5 -relief groove
scale .strings.s1 -from $stringMin(1) -to [expr $stringMin(1)+$stringRange] \
    -length 350 -orient horizontal -label "String 1: Note Number" \
    -tickinterval 5 -showvalue true -variable $stringNote(1) \
    -command {setNote 1}

scale .strings.s2 -from $stringMin(2) -to [expr $stringMin(2)+$stringRange] \
    -length 350 -orient horizontal -label "String 2: Note Number" \
    -tickinterval 5 -showvalue true -variable $stringNote(2) \
    -command {setNote 2}

scale .strings.s3 -from $stringMin(3) -to [expr $stringMin(3)+$stringRange] \
    -length 350 -orient horizontal -label "String 3: Note Number" \
    -tickinterval 5 -showvalue true -variable $stringNote(3) \
    -command {setNote 3}

scale .strings.s4 -from $stringMin(4) -to [expr $stringMin(4)+$stringRange] \
    -length 350 -orient horizontal -label "String 4: Note Number" \
    -tickinterval 5 -showvalue true -variable $stringNote(4) \
    -command {setNote 4}

scale .strings.s5 -from $stringMin(5) -to [expr $stringMin(5)+$stringRange] \
    -length 350 -orient horizontal -label "String 5: Note Number" \
    -tickinterval 5 -showvalue true -variable $stringNote(5) \
    -command {setNote 5}

scale .strings.s6 -from $stringMin(6) -to [expr $stringMin(6)+$stringRange] \
    -length 350 -orient horizontal -label "String 6: Note Number" \
    -tickinterval 5 -showvalue true -variable $stringNote(6) \
    -command {setNote 6}

button .strings.b1 -text Pluck -command { pluckOne 1 }
button .strings.b2 -text Pluck -command { pluckOne 2 }
button .strings.b3 -text Pluck -command { pluckOne 3 }
button .strings.b4 -text Pluck -command { pluckOne 4 }
button .strings.b5 -text Pluck -command { pluckOne 5 }
button .strings.b6 -text Pluck -command { pluckOne 6 }

grid .strings -column 0 -row 0
grid .strings.b1 -column 1 -row 0 -padx 5 -pady 5
grid .strings.b2 -column 1 -row 1
grid .strings.b3 -column 1 -row 2
grid .strings.b4 -column 1 -row 3
grid .strings.b5 -column 1 -row 4
grid .strings.b6 -column 1 -row 5

grid .strings.s1 -column 0 -row 0 -padx 5 -pady 5
grid .strings.s2 -column 0 -row 1 -padx 5 -pady 5
grid .strings.s3 -column 0 -row 2 -padx 5 -pady 5
grid .strings.s4 -column 0 -row 3 -padx 5 -pady 5
grid .strings.s5 -column 0 -row 4 -padx 5 -pady 5
grid .strings.s6 -column 0 -row 5 -padx 5 -pady 5

set stringSelect "All"
ttk::combobox .strings.combo \
    -values [ list "All" "String 1" "String 2" "String 3" "String 4" "String 5" "String 6" ] \
    -width 8 -textvariable stringSelect -justify center

scale .strings.velocity -from 0 -to 128 -length 350 \
    -orient horizontal -label "Note Velocity" \
    -tickinterval 32 -showvalue true -command setStringAmp -variable velocity

grid .strings.combo -column 1 -row 7
grid .strings.velocity -column 0 -row 7 -padx 5 -pady 10

pack .strings

set cbpath .strings.combo

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
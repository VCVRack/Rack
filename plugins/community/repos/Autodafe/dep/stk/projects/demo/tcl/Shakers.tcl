# Tcl/Tk Shakers GUI for the Synthesis Toolkit (STK)

# Set initial control values
set press 64.0
set cont1 64.0
set cont4 64.0
set cont11 64.0
set cont99 24.0
set patchnum 0

# Configure main window
wm title . "STK Shakers Controller"
wm iconname . "shakers"
. config -bg black

# Configure sliders
frame .right -bg black

scale .right.bPressure -from 0 -to 128 -length 300 \
    -command {changePress } -variable press\
    -orient horizontal -label "Shake Energy" \
    -tickinterval 32 -showvalue true -bg grey66

scale .right.cont2 -from 0 -to 128 -length 300 \
    -command {printWhatz "ControlChange    -1.0 1 " 11} \
    -orient horizontal -label "(<--High) System Damping (Low-->)" \
    -tickinterval 32 -showvalue true -bg grey66  \
    -variable cont4

scale .right.cont3 -from 0 -to 128 -length 300 \
    -command {printWhatz "ControlChange    -1.0 1 " 4} \
    -orient horizontal -label "Number of Objects" \
    -tickinterval 32 -showvalue true -bg grey66  \
    -variable cont11 

scale .right.vibrato -from 0 -to 128 -length 300 \
    -command {printWhatz "ControlChange    -1.0 1 " 1} \
    -orient horizontal -label "Resonance Center Freq." \
    -tickinterval 32 -showvalue true -bg grey66  \
    -variable cont1

scale .right.reverb -from 0 -to 128 -length 300 \
    -command {printWhatz "ControlChange    -1.0  1 " 44} \
    -orient horizontal -label "Reverb Mix" \
    -tickinterval 32 -showvalue true -bg grey66  \
    -variable cont99

pack .right.bPressure -padx 10 -pady 10
pack .right.vibrato 	-padx 10 -pady 10
pack .right.cont2 	-padx 10 -pady 10
pack .right.cont3 	-padx 10 -pady 10
pack .right.reverb 	-padx 10 -pady 10

pack .right -side right -padx 5 -pady 5

# Configure radio buttons
frame .buttons -bg black
frame .buttons.columns -bg black
frame .buttons.columns.left1 -bg black
frame .buttons.columns.left2 -bg black

radiobutton .buttons.columns.left1.maraca -text Maraca -bg grey66  \
				-command { patchChange 0 } -variable patchnum -width 15 \
				-justify left -value 0
radiobutton .buttons.columns.left1.cabasa -text Cabasa -bg grey66 \
				-command { patchChange 1 } -variable patchnum -width 15 \
				-justify left -value 1
radiobutton .buttons.columns.left1.sekere -text Sekere -bg grey66 \
				-command { patchChange 2 } -variable patchnum -width 15 \
				-justify left -value 2
radiobutton .buttons.columns.left1.tambourn -text Tambourine -bg grey66 \
				-command { patchChange 3 } -variable patchnum -width 15 \
				-justify left -value 3
radiobutton .buttons.columns.left1.sleighbl -text "Sleigh Bells" -bg grey66 \
				-command { patchChange 4 } -variable patchnum -width 15 \
				-justify left -value 4
radiobutton .buttons.columns.left1.bamboo -text Bamboo -bg grey66 \
				-command { patchChange 5 } -variable patchnum -width 15 \
				-justify left -value 5
radiobutton .buttons.columns.left1.sandpapr -text "Sand Paper" -bg grey66 \
				-command { patchChange 6 } -variable patchnum -width 15 \
				-justify left -value 6
radiobutton .buttons.columns.left1.cokecan -text "Coke Can" -bg grey66 \
				-command { patchChange 7 } -variable patchnum -width 15 \
				-justify left -value 7
radiobutton .buttons.columns.left1.stix1 -text Sticks -bg grey66 \
				-command { patchChange 8 } -variable patchnum -width 15 \
				-justify left -value 8
radiobutton .buttons.columns.left1.crunch1 -text Crunch -bg grey66 \
				-command { patchChange 9 } -variable patchnum -width 15 \
				-justify left -value 9
radiobutton .buttons.columns.left1.bigrocks -text "Big Rocks" -bg grey66 \
				-command { patchChange 10 } -variable patchnum -width 15 \
				-justify left -value 10
radiobutton .buttons.columns.left1.littlerocks -text "Little Rocks" -bg grey66 \
				-command { patchChange 11 } -variable patchnum -width 15 \
				-justify left -value 11
radiobutton .buttons.columns.left2.nextmug -text "NeXT Mug" -bg grey66 \
				-command { patchChange 12 } -variable patchnum -width 15 \
				-justify left -value 12
radiobutton .buttons.columns.left2.pennymug -text "Mug & Penny" -bg grey66 \
				-command { patchChange 13 } -variable patchnum -width 15 \
				-justify left -value 13
radiobutton .buttons.columns.left2.nicklemug -text "Mug & Nickle" -bg grey66 \
				-command { patchChange 14 } -variable patchnum -width 15 \
				-justify left -value 14
radiobutton .buttons.columns.left2.dimemug -text "Mug & Dime" -bg grey66 \
				-command { patchChange 15 } -variable patchnum -width 15 \
				-justify left -value 15
radiobutton .buttons.columns.left2.quartermug -text "Mug & Quarter" -bg grey66 \
				-command { patchChange 16 } -variable patchnum -width 15 \
				-justify left -value 16
radiobutton .buttons.columns.left2.francmug -text "Mug & Franc" -bg grey66 \
				-command { patchChange 17 } -variable patchnum -width 15 \
				-justify left -value 17
radiobutton .buttons.columns.left2.pesomug -text "Mug & Peso" -bg grey66 \
				-command { patchChange 18 } -variable patchnum -width 15 \
				-justify left -value 18
radiobutton .buttons.columns.left2.guiro -text Guiro -bg grey66 \
				-command { patchChange 19 } -variable patchnum -width 15 \
				-justify left -value 19
radiobutton .buttons.columns.left2.wrench -text Wrench -bg grey66 \
				-command { patchChange 20 } -variable patchnum -width 15 \
				-justify left -value 20
radiobutton .buttons.columns.left2.waterdrp -text "Water Drops" -bg grey66 \
				-command { patchChange 21 } -variable patchnum -width 15 \
				-justify left -value 21
radiobutton .buttons.columns.left2.tunedbamboo -text "Tuned Bamboo" -bg grey66 \
				-command { patchChange 22 } -variable patchnum -width 15 \
				-justify left -value 22

pack .buttons.columns.left1.maraca -pady 5
pack .buttons.columns.left1.cabasa -pady 5
pack .buttons.columns.left1.sekere -pady 5
pack .buttons.columns.left1.tambourn -pady 5
pack .buttons.columns.left1.sleighbl -pady 5
pack .buttons.columns.left1.bamboo -pady 5
pack .buttons.columns.left1.sandpapr -pady 5
pack .buttons.columns.left1.cokecan -pady 5
pack .buttons.columns.left1.stix1 -pady 5
pack .buttons.columns.left1.crunch1 -pady 5
pack .buttons.columns.left1.bigrocks -pady 5
pack .buttons.columns.left1.littlerocks -pady 5

pack .buttons.columns.left2.nextmug -pady 5
pack .buttons.columns.left2.pennymug -pady 5
pack .buttons.columns.left2.nicklemug -pady 5
pack .buttons.columns.left2.dimemug -pady 5
pack .buttons.columns.left2.quartermug -pady 5
pack .buttons.columns.left2.francmug -pady 5
pack .buttons.columns.left2.pesomug -pady 5
pack .buttons.columns.left2.guiro -pady 5
pack .buttons.columns.left2.wrench -pady 5
pack .buttons.columns.left2.waterdrp -pady 5
pack .buttons.columns.left2.tunedbamboo -pady 5

pack .buttons.columns.left1 -side left -padx 10
pack .buttons.columns.left2 -side left -padx 10
pack .buttons.columns -padx 10 -side top

# Configure exit button
button .buttons.exit -text "Exit Program" -bg grey66 -command myExit
pack .buttons.exit -pady 10 -side bottom
pack .buttons -pady 5

#bind all <KeyPress> {
bind . <KeyPress> {
    patchChange $patchnum
}

# Bind an X windows "close" event with the Exit routine
bind . <Destroy> +myExit

proc myExit {} {
    puts [format "ExitProgram"]
    flush stdout
    close stdout
    exit
}

proc patchChange {value} {
    global press
    puts [format "NoteOn    -1.0 1 %i $press" $value]
    flush stdout
}

proc printWhatz {tag value1 value2 } {
    puts [format "%s %i %f" $tag $value1 $value2]
    flush stdout
}

proc changePress {value} {
    puts [format "AfterTouch       -1.0 1 %f" $value]
    flush stdout
}

eval patchChange $patchnum

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

# Tcl/Tk GUI for the RagaMatic
# by Perry R. Cook

# Set initial control values
set cont1 10.0
set cont2 7.0
set cont4 0.0
set cont11 10.0
set cont7 3.0

# Configure main window
wm title . "STK RagaMatic Controller"
wm iconname . "raga"
. config -bg grey20

# Configure bitmap display
if {[file isdirectory bitmaps]} {
    set bitmappath bitmaps
} else {
    set bitmappath tcl/bitmaps
}

frame .banner -bg black

button .banner.top -bitmap @$bitmappath/ragamat2.xbm \
    -background white -foreground black

frame .banner.bottom -bg black
label .banner.bottom.ragamat -text " * * RagaMatic * *\n\n \
				by Perry R. Cook\n for Ken's Birthday\n \
				January, 1999\n\n (thanks also to\n \
				Brad for rough\n riff inspirations)" \
				-background black -foreground white -relief groove \
				-width 20 -height 10

# Bind an X windows "close" event with the Exit routine
bind . <Destroy> +myExit

proc myExit {} {
    puts [format "NoteOff          0.0 1 60 127"]
    flush stdout
    puts [format "ExitProgram"]
    flush stdout
    close stdout
    exit
}

proc mellow {} {
    global cont1 cont2 cont4 cont7 cont11
    set cont1 10.0
    set cont2 7.0
    set cont4 0.0
    set cont11 10.0
    set cont7 3.0
    printWhatz "ControlChange    0.0 1 " 1 $cont1
    printWhatz "ControlChange    0.0 1 " 2 $cont2
    printWhatz "ControlChange    0.0 1 " 4 $cont4
    printWhatz "ControlChange    0.0 1 " 7 $cont7
    printWhatz "ControlChange    0.0 1 " 11 $cont11
}

proc nicevibe {} {
    global cont1 cont2 cont4 cont7 cont11
    set cont1 6.0
    set cont2 72.0
    set cont4 21.0
    set cont11 50.0
    set cont7 60.0
    printWhatz "ControlChange    0.0 1 " 1 $cont1
    printWhatz "ControlChange    0.0 1 " 2 $cont2
    printWhatz "ControlChange    0.0 1 " 4 $cont4
    printWhatz "ControlChange    0.0 1 " 7 $cont7
    printWhatz "ControlChange    0.0 1 " 11 $cont11
}

proc voicSolo {} {
    global cont1 cont2 cont4 cont7 cont11
    set cont1 2.0
    set cont2 37.0
    set cont4 90.0
    set cont11 10.0
    set cont7 120.0
    printWhatz "ControlChange    0.0 1 " 1 $cont1
    printWhatz "ControlChange    0.0 1 " 2 $cont2
    printWhatz "ControlChange    0.0 1 " 4 $cont4
    printWhatz "ControlChange    0.0 1 " 7 $cont7
    printWhatz "ControlChange    0.0 1 " 11 $cont11
}

proc drumSolo {} {
    global cont1 cont2 cont4 cont7 cont11
    set cont1 3.0
    set cont2 37.0
    set cont4 0.0
    set cont11 100.0
    set cont7 120.0
    printWhatz "ControlChange    0.0 1 " 1 $cont1
    printWhatz "ControlChange    0.0 1 " 2 $cont2
    printWhatz "ControlChange    0.0 1 " 4 $cont4
    printWhatz "ControlChange    0.0 1 " 7 $cont7
    printWhatz "ControlChange    0.0 1 " 11 $cont11
}

proc rockOut {} {
    global cont1 cont2 cont4 cont7 cont11
    set cont1 1.0
    set cont2 97.0
    set cont4 52.0
    set cont11 120.0
    set cont7 123.0
    printWhatz "ControlChange    0.0 1 " 1 $cont1
    printWhatz "ControlChange    0.0 1 " 2 $cont2
    printWhatz "ControlChange    0.0 1 " 4 $cont4
    printWhatz "ControlChange    0.0 1 " 7 $cont7
    printWhatz "ControlChange    0.0 1 " 11 $cont11
}

proc raga {scale} {
    puts [format "ControlChange    0.0 1 64 %f" $scale]
    flush stdout
}

proc noteOn {pitchVal pressVal} {
    puts [format "NoteOn           0.0 1 %f %f" $pitchVal $pressVal]
    flush stdout
}

proc noteOff {pitchVal pressVal} {
    puts [format "NoteOff          0.0 1 %f %f" $pitchVal $pressVal]
    flush stdout
}

proc printWhatz {tag value1 value2 } {
    puts [format "%s %i %f" $tag $value1 $value2]
    flush stdout
}

frame .banner.butts -bg black

frame .banner.butts.ragas -bg black
button .banner.butts.ragas.raga0 -text "Raga1" \
	-bg grey66 -command {raga 0}
button .banner.butts.ragas.raga1 -text "Raga2" \
	-bg grey66 -command {raga 1}

frame .banner.butts.presets1 -bg black
button .banner.butts.presets1.warmup -text "Warmup" \
	-bg grey66 -command mellow
button .banner.butts.presets1.nicevibe -text "NiceVibe" \
	-bg grey66 -command nicevibe

frame .banner.butts.presets2 -bg black
button .banner.butts.presets2.voicsolo -text "VoiceSolo" \
	-bg grey66 -command voicSolo
button .banner.butts.presets2.drumsolo -text "DrumSolo" \
	-bg grey66 -command drumSolo

button .banner.butts.rockout -text "RockOut" \
	-bg grey66 -command rockOut

button .banner.butts.noteOn -text "Cease Meditations and Exit" \
	-bg grey66 -command myExit

frame .controls -bg black

scale .controls.cont1 -from 0 -to 128 -length 300 \
-command {printWhatz "ControlChange    0.0 1 " 1} \
-orient horizontal -label "Drone Probability" \
-tickinterval 32 -showvalue true -bg grey66  \
-variable cont1

scale .controls.cont2 -from 0 -to 128 -length 300 \
-command {printWhatz "ControlChange    0.0 1 " 2} \
-orient horizontal -label "Sitar Probability" \
-tickinterval 32 -showvalue true -bg grey66  \
-variable cont2

scale .controls.cont4 -from 0 -to 128 -length 300 \
-command {printWhatz "ControlChange    0.0 1 " 4} \
-orient horizontal -label "Voice Drum  Probability" \
-tickinterval 32 -showvalue true -bg grey66  \
-variable cont4 

scale .controls.cont11 -from 0 -to 128 -length 300 \
-command {printWhatz "ControlChange    0.0 1 " 11} \
-orient horizontal -label "Tabla  Probability" \
-tickinterval 32 -showvalue true -bg grey66  \
-variable cont11

scale .controls.cont7 -from 0 -to 128 -length 300 \
-command {printWhatz "ControlChange    0.0 1 " 7} \
-orient horizontal -label "Tempo" \
-tickinterval 32 -showvalue true -bg grey66  \
-variable cont7

pack .banner.top -pady 10 -padx 10
pack .banner.bottom.ragamat -padx 5 -pady 5
pack .banner.bottom -pady 10

pack .banner.butts.ragas.raga0 -side left
pack .banner.butts.ragas.raga1 -side left
pack .banner.butts.ragas
pack .banner.butts.presets1.warmup -side left
pack .banner.butts.presets1.nicevibe -side left
pack .banner.butts.presets1
pack .banner.butts.presets2.voicsolo -side left
pack .banner.butts.presets2.drumsolo -side left
pack .banner.butts.presets2
pack .banner.butts.rockout
pack .banner.butts.noteOn
pack .banner.butts -side left -padx 5 -pady 10
pack .banner -side left

pack .controls.cont1 	-padx 10 -pady 10
pack .controls.cont2 	-padx 10 -pady 10
pack .controls.cont4 	-padx 10 -pady 10
pack .controls.cont11	-padx 10 -pady 10
pack .controls.cont7 	-padx 10 -pady 10

pack .controls -side left -padx 10 -pady 10

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
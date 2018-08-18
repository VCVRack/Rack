set mixlevel 64.0
set effect1 64.0
set effect2 64.0
set effect 0
set outID "stdout"
set commtype "stdout"

# Configure main window
wm title . "STK Effects Controller"
wm iconname . "Effects"
. config -bg black

# Configure "communications" menu
menu .menu -tearoff 0
menu .menu.communication -tearoff 0 
.menu add cascade -label "Communication" -menu .menu.communication \
				-underline 0
.menu.communication add radio -label "Console" -variable commtype \
				-value "stdout" -command { setComm }
.menu.communication add radio -label "Socket" -variable commtype \
				-value "socket" -command { setComm }
. configure -menu .menu

# Configure title display
label .title -text "STK Effects Controller" \
				-font {Times 14 bold} -background white \
				-foreground darkred -relief raised

label .title2 -text "by Gary P. Scavone\n Music Technology, McGill University" \
				-font {Times 12 bold} -background white \
				-foreground darkred -relief raised

pack .title -padx 5 -pady 10
pack .title2 -padx 5 -pady 10

# Configure "note-on" buttons
frame .noteOn -bg black

button .noteOn.on -text NoteOn -bg grey66 -command { noteOn 64.0 64.0 }
button .noteOn.off -text NoteOff -bg grey66 -command { noteOff 64.0 127.0 }
button .noteOn.exit -text "Exit Program" -bg grey66 -command myExit
pack .noteOn.on -side left -padx 5
pack .noteOn.off -side left -padx 5 -pady 10
pack .noteOn.exit -side left -padx 5 -pady 10

pack .noteOn

# Configure sliders
frame .left -bg black

scale .left.effectsmix -from 0 -to 127 -length 400 \
-command {printWhatz "ControlChange    0.0 1 " 44} \
-orient horizontal -label "Effects Mix (0% effect - 100% effect)" \
-tickinterval 32 -showvalue true -bg grey66 \
-variable mixlevel

scale .left.effect1 -from 0 -to 127 -length 400 \
-command {printWhatz "ControlChange    0.0 1 " 22} \
-orient horizontal -label "Echo Delay" \
-tickinterval 32 -showvalue true -bg grey66 \
-variable effect1

scale .left.effect2 -from 0 -to 127 -length 400 \
-command {printWhatz "ControlChange    0.0 1 " 23} \
-orient horizontal -label "Disabled" \
-tickinterval 32 -showvalue true -bg grey66 \
-variable effect2

pack .left.effectsmix -padx 10 -pady 3
pack .left.effect1 -padx 10 -pady 3
pack .left.effect2 -padx 10 -pady 3

pack .left -side left

# Configure effect select buttons
frame .effectSelect -bg black
pack .effectSelect -side right -padx 5 -pady 5

radiobutton .effectSelect.echo -text "Echo" -variable effect -relief flat \
    -value 0 -command {changeEffect "ControlChange    0.0 1 " 20 $effect}
radiobutton .effectSelect.shifter -text "Pitch Shift" -variable effect -relief flat \
    -value 1 -command {changeEffect "ControlChange    0.0 1 " 20 $effect} 
radiobutton .effectSelect.lshifter -text "Lent Pitch Shift" -variable effect -relief flat \
    -value 2 -command {changeEffect "ControlChange    0.0 1 " 20 $effect} 
radiobutton .effectSelect.chorus -text "Chorus" -variable effect -relief flat \
    -value 3 -command {changeEffect "ControlChange    0.0 1 " 20 $effect}
radiobutton .effectSelect.prcrev -text "PRC Reverb" -variable effect -relief flat \
    -value 4 -command {changeEffect "ControlChange    0.0 1 " 20 $effect}
radiobutton .effectSelect.jcrev -text "JC Reverb" -variable effect -relief flat \
    -value 5 -command {changeEffect "ControlChange    0.0 1 " 20 $effect}
radiobutton .effectSelect.nrev -text "NRev Reverb" -variable effect -relief flat \
    -value 6 -command {changeEffect "ControlChange    0.0 1 " 20 $effect}
radiobutton .effectSelect.freerev -text "FreeVerb" -variable effect -relief flat \
    -value 7 -command {changeEffect "ControlChange    0.0 1 " 20 $effect}

pack .effectSelect.echo -pady 2 -padx 5 -side top -anchor w -fill x
pack .effectSelect.shifter -pady 2 -padx 5 -side top -anchor w -fill x
pack .effectSelect.lshifter -pady 2 -padx 5 -side top -anchor w -fill x
pack .effectSelect.chorus -pady 2 -padx 5 -side top -anchor w -fill x
pack .effectSelect.prcrev -pady 2 -padx 5 -side top -anchor w -fill x
pack .effectSelect.jcrev -pady 2 -padx 5 -side top -anchor w -fill x
pack .effectSelect.nrev -pady 2 -padx 5 -side top -anchor w -fill x
pack .effectSelect.freerev -pady 2 -padx 5 -side top -anchor w -fill x


proc myExit {} {
    global outID
    puts $outID [format "NoteOff          0.0 1 64 127" ]
    flush $outID
    puts $outID [format "ExitProgram"]
    flush $outID
    close $outID
    exit
}

proc noteOn {pitchVal pressVal} {
    global outID
    puts $outID [format "NoteOn           0.0 1 %f %f" $pitchVal $pressVal]
    flush $outID
}

proc noteOff {pitchVal pressVal} {
    global outID
    puts $outID [format "NoteOff          0.0 1 %f %f" $pitchVal $pressVal]
    flush $outID
}

proc printWhatz {tag value1 value2 } {
    global outID
    puts $outID [format "%s %i %f" $tag $value1 $value2]
    flush $outID
}

proc changeEffect {tag value1 value2 } {
    global outID
    if ($value2==0) {
        .left.effect1 config -state normal -label "Echo Delay"
        .left.effect2 config -state disabled -label "Disabled"
    }
    if {$value2>=1 && $value2<=2} {
        .left.effect1 config -state normal -label "Pitch Shift Amount (center = no shift)"
        .left.effect2 config -state disabled -label "Disabled"
    }
    if ($value2==3) {
        .left.effect1 config -state normal -label "Chorus Modulation Frequency"
        .left.effect2 config -state normal -label "Chorus Modulation Depth"
    }
    if {$value2>=4 && $value2<=6} {
        .left.effect1 config -state normal -label "T60 Decay Time ( 0 - 10 seconds)"
        .left.effect2 config -state disabled -label "Disabled"
    }
    if ($value2==7) {
        .left.effect1 config -state normal -label "Damping (low to high)"
        .left.effect2 config -state normal -label "Room Size (comb feedback gain)"
    }
    puts $outID [format "%s %i %f" $tag $value1 $value2]
    flush $outID
}

# Bind an X windows "close" event with the Exit routine
bind . <Destroy> +myExit

# Socket connection procedure
set d .socketdialog

proc setComm {} {
		global outID
		global commtype
		global d
		if {$commtype == "stdout"} {
				if { [string compare "stdout" $outID] } {
						set i [tk_dialog .dialog "Break Socket Connection?" {You are about to break an existing socket connection ... is this what you want to do?} "" 0 Cancel OK]
						switch $i {
								0 {set commtype "socket"}
								1 {close $outID
								   set outID "stdout"}
						}
				}
		} elseif { ![string compare "stdout" $outID] } {
				set sockport 2001
        set sockhost localhost
				toplevel $d
				wm title $d "STK Client Socket Connection"
				wm resizable $d 0 0
				grab $d
				label $d.message -text "Specify a socket host and port number below (if different than the STK defaults shown) and then click the \"Connect\" button to invoke a socket-client connection attempt to the STK socket server." \
								-background white -font {Helvetica 10 bold} \
								-wraplength 3i -justify left
				frame $d.sockhost
				entry $d.sockhost.entry -width 15
				label $d.sockhost.text -text "Socket Host:" \
								-font {Helvetica 10 bold}
				frame $d.sockport
				entry $d.sockport.entry -width 15
				label $d.sockport.text -text "Socket Port:" \
								-font {Helvetica 10 bold}
				pack $d.message -side top -padx 5 -pady 10
				pack $d.sockhost.text -side left -padx 1 -pady 2
				pack $d.sockhost.entry -side right -padx 5 -pady 2
				pack $d.sockhost -side top -padx 5 -pady 2
				pack $d.sockport.text -side left -padx 1 -pady 2
				pack $d.sockport.entry -side right -padx 5 -pady 2
				pack $d.sockport -side top -padx 5 -pady 2
				$d.sockhost.entry insert 0 $sockhost
				$d.sockport.entry insert 0 $sockport
				frame $d.buttons
				button $d.buttons.cancel -text "Cancel" -bg grey66 \
								-command { set commtype "stdout"
				                   set outID "stdout"
				                   destroy $d }
				button $d.buttons.connect -text "Connect" -bg grey66 \
								-command {
						set sockhost [$d.sockhost.entry get]
						set sockport [$d.sockport.entry get]
					  set err [catch {socket $sockhost $sockport} outID]

						if {$err == 0} {
								destroy $d
						} else {
								tk_dialog $d.error "Socket Error" {Error: Unable to make socket connection.  Make sure the STK socket server is first running and that the port number is correct.} "" 0 OK 
				}   }
				pack $d.buttons.cancel -side left -padx 5 -pady 10
				pack $d.buttons.connect -side right -padx 5 -pady 10
				pack $d.buttons -side bottom -padx 5 -pady 10
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



![alt text](/screens/cf064.png)
![alt text](/screens/cf064b.png)
 
 
**trSEQ : tr style 16 steps SEQ with trig input per step**

  main functions : cf. Fundamental/SEQ3
  
  steps :   each step can be turned on/off manually by clicking it or can be automated
  
              and/or externally controlled thru its own trig input
            
              you can use an external keyboard, pad controller, or third party app to use your computer keyboard
            
              as a midi keyboard, via Core/midi-trigger-to-CV or Core/midi-to-CV
            
              or any CV/trigger source/modifier like Audible/Bernouilli's gate for randomness
            
  tap in :  input notes by clicking on the pad or sending trigs or sustained gates to the corresponding input
  
              (cf. steps)
            
  clear :   same as "tap in" but for deleting notes
  
![alt text](/screens/trseq1.png)

![alt text](/screens/trseq2.png)

![alt text](/screens/trseq3.png)





**STEPS : variable quantiser**

    originally conceived as a companion for PLAYER,
  
    used between a LFO or a SEQ CV out and PLAYER's start input
  
    it will quantize 0 to 10 Volts CV to regular steps,
  
    allowing for regular slicing of breakbeat loops or anything else
  
![alt text](/screens/steps1.png)

![alt text](/screens/steps2.png)





**PLAYER : sampler [mod from Bidoo's OUAIve]**
  
  use right click menu to select a .wav or .aif sample (some hi-res or compressed ones won't open)
  
  gate :      the sample will be played while the gate is open (i.e. input > 0 V.)
  
                try retrigger mode (right click menu) with most seq's
              
                or use an enveloppe or ML gate delay to have a correct gate for your needs
              
  start :     use knob to choose start point and/or attenuverted CV input 
  
                (don't forget to "open" the attenuverter)
              
  speed :   same as start but for speed, very low setting will play reverse (from start point)
  
  trick :     use a square fast LFO on gate input and a saw LFO on start input to get dirty time stretch
  
![alt text](/screens/player1.png)

![alt text](/screens/player2.png)





**PEAK : Peak limiter**

  threshold :   turn it down from 10 V. to 0 V. until it lights up as the limiter acts
  
  make up :     turn it up to adjust gain, lights up when signal gets over 10 V.
  
  
  trick :     if input is unplugged, PEAK will output screen's voltage
  
                it can also be use as an attenu/amp.
              
![alt text](/screens/peak1.png)

![alt text](/screens/peak2.png)

  
  
  

**CUBE : a blank panel turned weird module**

  inputs :  each input receives CV to controle the rotation speed on a different axis
  
              modulate with a LFO, or any other signal
            
  output :  is Z coordinate of one of the points of the cube
  
              found it quite unpredictable, especially when inputs are modulated
            
              Pretty and experimental :)
            
![alt text](/screens/cube1.png)

     

**FOUR : 4 trigged solos/mutes**

  each signal "line" can be turned on/off or soloed, manually and/or triggered thru its own trig input
  
              you can use an external keyboard, pad controller, or third party app to use your computer keyboard
            
              as a midi keyboard, via Core/midi-trigger-to-CV or Core/midi-to-CV
            
              or any CV/trigger source/modifier like Audible/Bernouilli's gate for randomness
            
![alt text](/screens/four1.png)

![alt text](/screens/four2.png)

![alt text](/screens/four3.png)





**MONO STEREO MASTER : modular mixer**

  you can use them in combination to make a mixer 
  
  with any number of mono and/or stereo channels, sub groups, etc.
  
  
  
  in :        gets your mono or stero signal in
  
  out :       on MONO, duplicates the signal post gain, post on and solo, 
  
                for redirection to an FX (reverb, delay, ...) or any other use
              
              
  gain :      multiplies the in signal by 0 to 2, 1 is at 12 o'clock/reset position
  
                beware of your levels, a red light will bright up over 10 V.
              
  gain input : over-rides gain knob and turns 0 to 10 V. input 
  
                into 0 to 2 multiplier (1 being 5 V.) 
              
                for automation and/or external control
              
              
  on :        switches the signal of this channel on/off
  
  on input :  inverts on state on trig
  
                for automation and/or external control
              
              
              
  solo :      turns off un-soloed channels in a solo-group, cf. solo-link
  
  solo input : inverts solo state on trig
  
                for automation and/or external control
              
              
              
  pan :       attenuates one side of the stereo signal
  
  pan input : over-rides pan knob and turns 0 to 10 V. input into left to right (5 being center)
  
                for automation and/or external control
              
              
  
  stereo bus : stereo signal gets in 
  
                adds gained & pan'ed input signal
              
                stereo signal gets out to next channel or master
              
              
  S-L :       solo link, make a solo group so that solo'ing a channel or many will turn off others 
  
                by linking them so they can communicate, 
              
                don't forget to link the last to the first.
  
  ![alt text](/screens/mixer.png)
  
  
  
  
  

**METRO : master clock and metronome**

  BPM :        sets the beat per minute tempo
  
  BPM input :   over-rides BPM knob
  
                 turns 0 to 10 V. into 0 to 300 bpm
               
  on :         turns METRO on/off, starts on first beat (most of the times)

  on input :    turns METRO on/off on trig
  
  reset :      sets METRO back to 1st beat without stoping it
  
                 use to reset dividers and seqs on the fly
               
  mes. light : will light up on 1st beat out of 4
  
  beat light : will light up on every beat
  
  head speaker : outputs audio metronome

                 TOC-toc-toc-toc-TOC-toc-toc-toc-TOC-toc...
                 
  start :      will output a trig on start or reset
  
  x 12 :      will output 12 trigs per beat
  




**EACH : clock divider**

  left circuit 'start' will receive and duplicate
  
               start/reset trigs to its ouputs
  
  right circuit 'x 12' will receive and duplicate
  
               trigs stream ('x 12' from METRO) to its output
  
               and 1 out of DIV. (knob/screen) trig to the middle output
               
               allow sync'ing of seqs
               
               3 will give you 1/4 of beat, 4 triolets, 6 half beat and so on
               
  DIV. input : over-rides DIV. knob
  
               turns 0 to 10 V. to 1 to 12 divs
               
  trick :    chain EACHs to trig events every mesure, 4 mesures or 16 mesures
  

  ![alt text](/screens/clock.png)




**PATCH : patch bay**

  mainly here to take cables out of the view
  
  signals follow the lines between inputs and outputs




**DAVE : blank panel**


please, [DONATE](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=3CSNFE349G99Q) so i can buy other devs'modules, and thanks to those who already did :)

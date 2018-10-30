# Das Wohltemperierte Klavier - Unequal Temperament Quantizer
#### WK-101 Das Wohltemperierte Klavier

![View of the Das Wohltemperierte Klavier](WK-101.png "Das Wohltemperierte Klavier")

## Basic Operation

The WK-101 takes a note CV and quantizes it to a 12-ET chromatic scale. It then adjusts the quantized signal up or down by up to 50 cents according to which pitch it has been quantized to. The each of the 12 pitches can have a different adjustment specified by using the control knobs. The cent adjustment is displayed to the side of each knob.

The currently recognized pitch is indicated by a small blue led in the centre of the 12 adjustment knobs.

The context menu offers presets which you can configure. See below for more details

#### WK-205 Das Wohltemperierte Klavier Nano

![View of the Das Wohltemperierte Klavier nano](WK-205.png "Das Wohltemperierte Klavier nano")

## Basic Operation

The WK-205 has 5 input and output pairs. It takes each note CV input, quantizes it to a 12-ET chromatic scale, and then adjusts the quantized signal up or down by up to 50 cents. There are no control knobs on the WK-205 so the adjustment settings must be selected either from presets on the context menu, or by synching the device from a WK-101.

## Synch Ports

The WK-101 can output its settings to another WK-101 or to a WK-205. Connect a patch lead from the Sync-out port on the master device to the Sync-in port on the slave devige. Settings are automatically transmitted whenever they change on the master device.

## Presets

Presets are loaded from two different sources. 

#### Scala files

The SubmarineFree plugin directory contains a Scala subdirectory. Scala format .scl files are read from here. Note that the Scala application has far greater capabilities than the WK devices are intended to support, so not all Scala files will contain information the the WK devices can use. A maximum of 12 tunings will be read from the file, any more will be ignored. The WK devices always assume that the tonic in the Scala file is C; since the Scala format takes all tunings relative to the tonic, it does not include a tuning for the tonic itself. If a 12th ratio is present in the file, it will be used to provide a tuning for C. The fill will be rejected if any ratio is more than 50 cents from the 12-ET tuning.

#### WK_Custom.tunings

Presets can also be defined in the WK_Custom.tunings file. This is not included in the plugin release, but you can copy the WK_Custom.tunings.template file to use as a starting point. 

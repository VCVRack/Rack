# Vult Modules for Rack

A set of modules for [VCVRack](https://vcvrack.com) written in [Vult](http://modlfo.github.io/vult/). Read more about these modules here: http://modlfo.github.io/projects/vult-modules/

I provide precompiled binaries for: [Linux, Mac and Windows](https://github.com/modlfo/VultModules/releases).

![Rescomb](/images/Rescomb-render.png?raw=true "Rescomb")
![Stabile](/images/Stabile-render.png?raw=true "Stabile")
![Lateralus](/images/Lateralus-render.png?raw=true "Lateralus")
![Debriatus](/images/Debriatus-render.png?raw=true "Debriatus")
![Splie](/images/Splie-render.png?raw=true "Splie")
![Tohe](/images/Tohe-render.png?raw=true "Tohe")
![Trummor](/images/Trummor-render.png?raw=true "Trummor")
![Tangents](/images/Tangents-render.png?raw=true "Tangents")

*This repository builds against the latest released version of Rack (the latest tag). It may take a day or two for me to catch up when a new version of Rack is released. Let me know if I haven't fixed that.*

## Donate

I’m glad to publish these modules for free so anyone can use them. Behind every module there are many hours of work (and many litters of coffee). If you enjoy these modules you can support the development by making a donation. As a suggestion, you can send me the amount to buy 1 kg of roasted coffee beans in your country. That will make me very happy. Here's the link: [DONATE](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=RRMY2QPYEZT2S)

## Use

Download from the [releases page](https://github.com/modlfo/VultModules/releases) the file corresponding to the VCVRack version you have. Place the `VultModules` directory in the plugins folder.


## Build

First you need to build the latest release version of [Rack](https://github.com/VCVRack/Rack) following the instructions in its [README](https://github.com/VCVRack/Rack/blob/master/README.md).

Then clone this repository inside the `plugins` folder of Rack and use `make`.

```
$ cd plugins
$ git clone https://github.com/modlfo/VultModules.git
$ make
```

## Modify

To change the DSP code you need to have Vult installed. Vult can be installed by downloading it from the [Releases](https://github.com/modlfo/vult/releases) page or installed with [npm](https://www.npmjs.com/package/vult).

Once you have Vult installed you can regenerate the code with the following line:
```
$ vultc src/VultEngine.vult -ccode -o src/VultEngine
```

## License

The license is a BSD 3-Clause with the addition that any commercial use requires explicit permission of the author. That applies for the source code.

For the image resources (all SVG files), any use requires explicit permission of the author.

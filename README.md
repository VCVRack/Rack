# Rack

*Rack* is the engine for the VCV open-source virtual modular synthesizer.

![Rack screenshot](https://vcvrack.com/images/screenshot.png)

This README includes instructions for building Rack from source. For information about the software, go to https://vcvrack.com/.

## The [Issue Tracker](https://github.com/VCVRack/Rack/issues?q=is%3Aissue+is%3Aopen+sort%3Aupdated-desc) *is* the official developer's forum

Bug reports, feature requests, and even *questions/discussions* are welcome on the GitHub Issue Tracker for all VCVRack repos.
However, please search before posting to avoid duplicates, and limit to one issue per post.

Please vote on feature requests by using the Thumbs Up/Down reaction on the first post.

I rarely accept code contributions to Rack itself, so please notify me in advance if you wish to send a pull request.

## Setting up your development environment

Before building Rack, you must install build dependencies provided by your system's package manager.
Rack's own dependencies (GLEW, glfw, etc) do not need to be installed on your system, since specific versions are compiled locally during the build process.
However, you need proper tools to build Rack and these dependencies.

### Mac

Install [Xcode](https://developer.apple.com/xcode/).
Using [Homebrew](https://brew.sh/), install the build dependencies.
```
brew install git wget cmake autoconf automake libtool
```

### Windows

Install [MSYS2](http://www.msys2.org/) and launch the MinGW 64-bit shell (not the default MSYS shell).
```
pacman -S git wget make tar unzip zip mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake autoconf automake mingw-w64-x86_64-libtool
```

### Linux

On Arch Linux:
```
pacman -S git wget gcc make cmake tar unzip zip curl
```

On Ubuntu 16.04:
```
sudo apt install git curl cmake libx11-dev libglu1-mesa-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev zlib1g-dev libasound2-dev libgtk2.0-dev libjack-jackd2-dev
```

## Building

*If the build fails for you, please report the issue with a detailed error message to help the portability of Rack.*

Clone this repository with `git clone https://github.com/VCVRack/Rack.git` and `cd Rack`.
Make sure there are no spaces in your path, as this breaks many build systems.

Clone submodules.

	git submodule update --init --recursive

Build dependencies locally.
You may use make's `-j$(nproc)` flag to parallelize builds across all your CPU cores.

	make dep

You may use `make dep RTAUDIO_ALL_APIS=1` to attempt to build with all audio driver APIs enabled for your operating system, although this is unsupported.

You should see a message that all dependencies built successfully.

Build Rack.

	make

Run Rack.

	make run

## Building plugins

Be sure to check out and build the version of Rack you wish to build your plugins against.

You must clone the plugin in Rack's `plugins/` directory, e.g.

	cd plugins
	git clone https://github.com/VCVRack/Fundamental.git

Clone submodules.

	cd Fundamental
	git submodule update --init --recursive

Build plugin.

	make dep
	make

## Licenses

All **source code** in this repository is licensed under [BSD-3-Clause](LICENSE.txt) by [Andrew Belt](https://andrewbelt.name/).

**Component Library graphics** in `res/ComponentLibrary` are licensed under [CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/) by [Grayscale](http://grayscale.info/). Commercial plugins must request a commercial license to use Component Library graphics by emailing contact@vcvrack.com.

**Core** panel graphics in `res/Core` are copyright © 2017 Grayscale. You may not create derivative works of Core panels.

The **VCV logo and icon** are copyright © 2017 Andrew Belt and may not be used in derivative works.

The **"VCV" name** is trademarked and may not be used for unofficial products. However, it is acceptable to use the phrase "for VCV Rack" for promotion of your plugin. For all other purposes, email contact@vcvrack.com.

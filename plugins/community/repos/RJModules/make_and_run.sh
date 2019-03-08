#! /bin/bash

trash dist
trash plugins
trash /Applications/Rack.app/Contents/Resources/plugins/RJModules
trash ~/Documents/Rack/plugins/RJModules 
set -e
RACK_DIR=~/Downloads/Rack-SDK/ make dist
mkdir -p /Applications/Rack.app/Contents/Resources/plugins
cp -r dist/RJModules /Applications/Rack.app/Contents/Resources/plugins/
mkdir -p ~/Documents/Rack/plugins/RJModules
cp -r dist/RJModules ~/Documents/Rack/plugins/
mkdir -p plugins
cp -r dist/RJModules plugins/
/Applications/Rack.app/Contents/MacOS/Rack -d

# !/bin/sh

mkdir -p dist #debug
cp Rack.exe libRack.dll dist
# cp Rack.exe libRack.dll debug
strip -s dist/*
# cp dist/* 'c:/Program Files/VCV/Rack2 Pro'

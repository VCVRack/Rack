#!/bin/bash

for f in build/res/*.svg
do
	b=$(basename "$f")
	r=${b%.*}
#	"/c/Program Files/Inkscape/Inkscape.exe" $f --export-png=res/${r}.png
	"/c/Program Files/Inkscape/Inkscape.exe" $f --export-plain-svg=res/${r}.svg --export-text-to-path

#	if [[ "$OSTYPE" == "msys" ]]; then
#		unix2dos res/${r}.svg
#	fi
done

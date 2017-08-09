
for f in "$@"; do
	f2=${f/.png/@2x.png}
	mv $f $f2
	convert $f2 -resize 50% $f
done

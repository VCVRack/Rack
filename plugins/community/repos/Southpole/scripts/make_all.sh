!/bin/sh
make clean
make -j8 dist
rm -rf ../Southpole
cp -r dist/Southpole ..
make clean

!/bin/sh
make clean
make -j8 dist
rm -rf ../Southpole-parasites
cp -r dist/Southpole-parasites ..
make clean

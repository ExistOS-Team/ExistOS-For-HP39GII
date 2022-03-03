#! /bin/bash
set > log
sleep 5
rm -f xcasunst.zip
wget 'http://www-fourier.ujf-grenoble.fr/~parisse/giac/xcasunst.zip' 
mkdir unpack 
cd unpack 
../unzip -o ../xcasunst.zip >> log
cd ..
cp -R unpack/* . 
rm -rf unpack 
rm -f xcasunst.zip

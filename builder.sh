#!/bin/sh -e
source $stdenv/setup

mkdir -p $out

cd $src
cp -R $src $TMP/ExistOS
chmod -R 755 $TMP/ExistOS
cd $TMP/ExistOS

echo '#!/bin/sh -e
sysigner $@' >> ./tools/sysigner
chmod +x ./tools/sysigner

echo '#!/bin/sh -e
elftosb $@' >> ./tools/sbtools/elftosb
chmod +x ./tools/sbtools/elftosb

cmake .
make
cp $TMP/ExistOS/OSLoader/OSLoader.sb $out/
cp $TMP/ExistOS/System/ExistOS.sys $out/

#!/bin/sh -e
source $stdenv/setup
mkdir -p $out/lib $TMP/src

cp -R $src $TMP/src/mpy
chmod 755 -R $TMP/src/mpy
cd $TMP/src/mpy/ports/eoslib && make
cp $TMP/libmpy.libc $out/lib/

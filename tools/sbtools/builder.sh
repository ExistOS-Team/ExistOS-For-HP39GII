#!/bin/sh -e
source $stdenv/setup
mkdir -p $out/bin

cp -R $src $TMP/sbtools
chmod 755 -R $TMP/sbtools
cd $TMP/sbtools && make
cp sbloader $out/bin/
cp elftosb $out/bin/

#!/bin/sh -e
source $stdenv/setup
mkdir -p $out/bin

cd $src
mkdir -p $TMP/sys_signer/build
cmake -B $TMP/sys_signer/build/
cmake --build $TMP/sys_signer/build/
cp $TMP/sys_signer/build/sysigner $out/bin/sysigner

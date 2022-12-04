{ pkgs ? import <nixpkgs> {} }:
with pkgs;
stdenv.mkDerivation {
    name = "micropython-eos";
    src = ./.;
    builder = ./builder.sh;
    buildInputs = [
        gcc-arm-embedded
        python3
    ];
}
{ pkgs ? import <nixpkgs> {} }:
with pkgs;
stdenv.mkDerivation {
    name = "sbtools";
    src = ./.;
    builder = ./builder.sh;
    buildInputs = [
        libusb
        cryptopp
        udev
        pkg-config
    ];
}
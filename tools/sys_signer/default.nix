{ pkgs ? import <nixpkgs> {} }:
with pkgs;
stdenv.mkDerivation {
    name = "sys_signer";
    src = ./.;
    builder = ./builder.sh;
    buildInputs = [
        cmake
        glibc.static
    ];
}
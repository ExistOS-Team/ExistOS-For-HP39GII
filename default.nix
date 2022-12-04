{ pkgs ? import <nixpkgs> {} }:
with pkgs;
stdenv.mkDerivation {
    name = "ExistOS";
    src = ./.;
    builder = ./builder.sh;
    buildInputs = [
        gcc-arm-embedded
        newlib-nano
        cmake
        (import ./tools/sys_signer/default.nix {})
        (import ./tools/sbtools/default.nix {})
    ];
}
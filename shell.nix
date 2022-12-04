let pkgs = import <nixpkgs> {};
in with pkgs;
pkgs.mkShell {
    buildInputs = [
        gcc-arm-embedded
        newlib-nano
        cmake
        (import ./tools/sys_signer/default.nix {})
        (import ./tools/sbtools/default.nix {})
    ];
}
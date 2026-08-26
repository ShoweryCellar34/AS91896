{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
    let
      systems = [
        "x86_64-linux"
        "aarch64-linux"
      ];
      forAllSystems = nixpkgs.lib.genAttrs systems;
    in {
      devShells = forAllSystems (system:
        let
          pkgs = import nixpkgs { inherit system; };
        in {
          default = (pkgs.mkShell.override { stdenv = pkgs.clangStdenv; }) {
            hardeningDisable = [ "all" ];
            nativeBuildInputs = with pkgs; [
              git
              llvmPackages.llvm
              llvmPackages.lld
              llvmPackages.lldb
              llvmPackages.clang-tools
              gcc
              cmake
              ninja
              pkg-config
              include-what-you-use
              ccache
              cppcheck
              lcov
              valgrind
              doxygen
            ];
          };
        });
    };
}

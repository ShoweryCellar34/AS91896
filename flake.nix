{
  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
    in
    {
      devShells.${system}.default = pkgs.mkShell.override {} {
        hardeningDisable = [ "all" ];
        packages = with pkgs; [
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
          libX11
          libXrandr
          libXinerama
          libXcursor
          libXi
          libGL
        ];
      };
    };
}
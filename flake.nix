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
          pkgs     = import nixpkgs { inherit system; };
          llvmPkgs = pkgs.llvmPackages;
        in {
          default = pkgs.mkShell {
            nativeBuildInputs = with pkgs; [
              git
              llvmPkgs.clang
              llvmPkgs.lld
              llvmPkgs.lldb
              llvmPkgs.clang-tools
              cmake
              ninja
              pkg-config
              include-what-you-use # -> CMAKE_C_INCLUDE_WHAT_TO_USE
              ccache               # -> CMAKE_C_COMPILER_LAUNCHER
              cppcheck             # -> CMAKE_C_CPPCHECK
              valgrind

              wayland-scanner
              shader-slang
            ];

            # buildInputs = with pkgs; [
            # ];

            # shellHook = ''
            #   export LD_LIBRARY_PATH="${pkgs.lib.makeLibraryPath [

            #   ]}:$LD_LIBRARY_PATH"
            # '';

            env = {
              CC = "clang";
            };
          };
        });
    };
}

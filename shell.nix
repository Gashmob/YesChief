{ pkgs ? (import ./tools/nix/pin-nixpkgs.nix) {} }:

let
  currentDir = builtins.toString ./.;
  pnpm = pkgs.pnpm.override { 
    version = "10.4.1";
    hash = "sha256-S3Aoh5hplZM9QwCDawTW0CpDvHK1Lk9+k6TKYIuVkZc=";
    nodejs = pkgs.nodejs_20;
  };
in

pkgs.mkShellNoCC {
  name = "yeschief-dev-env";

  packages = [
    pkgs.git
    pkgs.cmake
    pkgs.ninja
    pkgs.clang
    pkgs.lcov
    pkgs.dpkg
    pkgs.rpm
    pkgs.nodejs_20
    pnpm
    pkgs.doxygen
  ];

  shellHook = ''
    export ROOT_DIR="${currentDir}"
    export PATH="$PATH:${currentDir}/tools/bin"
    pnpm install;
    echo -e "\n\033[34mWelcome to \033[1mYesChief!\033[0m\033[34m dev environment\033[0m\n"
  '';
}

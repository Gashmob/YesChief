{
  description = "Yeschief! dev environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/25.05";

    # <https://github.com/nix-systems/nix-systems>
    systems.url = "github:nix-systems/default-linux";
  };

  outputs = { nixpkgs, systems, ... }:
    let
      eachSystem = nixpkgs.lib.genAttrs (import systems);
      pkgs = eachSystem (system: import nixpkgs { inherit system; });
      pnpm = eachSystem (system:
        pkgs.${system}.pnpm.override {
          version = "10.12.4";
          hash = "sha256-yt/Z5sn8wst2/nwHeaUlC2MomK6l9T2DOnNpDHeneNk=";
          nodejs = pkgs.${system}.nodejs_20;
        });
      yeschief-package =
        eachSystem (system: pkgs.${system}.callPackage ./yeschief.nix { });
    in {
      packages = eachSystem (system: {
        yeschief = yeschief-package.${system};
        default = yeschief-package.${system};
      });

      devShells = eachSystem (system: {
        default = pkgs.${system}.mkShellNoCC {
          name = "yeschief-dev-env";

          packages = with pkgs.${system};
            [ git cmake ninja clang lcov dpkg rpm nodejs_20 doxygen ]
            ++ [ pnpm.${system} ];

          shellHook = ''
            export ROOT_DIR=$(git rev-parse --show-toplevel)
            export PATH="$PATH:$ROOT_DIR/tools/bin"
            pnpm install;
            echo -e "\n\033[34mWelcome to \033[1mYesChief!\033[0m\033[34m dev environment\033[0m\n"
          '';
        };
      });
    };
}

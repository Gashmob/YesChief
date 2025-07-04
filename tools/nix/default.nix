let
  pkgs = (import ./pin-nixpkgs.nix) {};
in {
  yeschief = pkgs.callPackage ./yeschief.nix { };
}

{ stdenv, cmake, ninja }:

stdenv.mkDerivation {
  pname = "yeschief";
  version = builtins.readFile ./VERSION;

  src = ./.;

  nativeBuildInputs = [ cmake ninja ];
}

{ stdenvNoCC
, fetchFromGitHub
, cmake
, ninja
, llvmPackages_19
}:

stdenvNoCC.mkDerivation rec {
  pname = "yeschief";
  version = "1.0.0";

  src = fetchFromGitHub {
    owner = "Gashmob";
    repo = "YesChief";
    rev = "v${version}";
    hash = "sha256-tfZhdoguvIif66Knr39z0KIcQ3vKK7jZtbODLFkrRgU=";
  };

  nativeBuildInputs = [ cmake ninja llvmPackages_19.libcxxClang ];

  patches = [ ./ranges.patch ];
}

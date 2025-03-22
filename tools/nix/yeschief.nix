{ stdenv
, fetchFromGitHub
, cmake
, ninja
}:

stdenv.mkDerivation rec {
  pname = "yeschief";
  version = "1.1.0";

  src = fetchFromGitHub {
    owner = "Gashmob";
    repo = "YesChief";
    rev = "v${version}";
    hash = "sha256-PRUpKIn6nGewY1MC0DLyHu/+FBK6cVtUcbjvRBLm4nM=";
  };

  nativeBuildInputs = [ cmake ninja ];
}

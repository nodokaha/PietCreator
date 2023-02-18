{pkgs ? import <nixpkgs> {} }:
with pkgs;
pkgs.mkShell {
  name = "pietcreator";
  buildInputs = [ qt5Full gd pkg-config cmake giflib libpng ];
}

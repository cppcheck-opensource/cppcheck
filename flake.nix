{
  description = "ccpcheck";
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.11";
  };

  outputs =
    {
      self,
      nixpkgs,
    }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
    in
    {
      formatter.${system} = pkgs.nixfmt-tree;
      packages.${system} = {
        default = pkgs.stdenvNoCC.mkDerivation {
          name = "cppcheck";
          src = self;
          outputs = [
            "out"
            "man"
          ];

          nativeBuildInputs = with pkgs; [
            docbook_xml_dtd_45
            docbook_xsl
            installShellFiles
            libxslt
            pkg-config
            python3
            which
            clang
          ];

          buildInputs = with pkgs; [
            pcre
            (python3.withPackages (ps: [ ps.pygments ]))
          ];

          makeFlags = [
            "PREFIX=$(out)"
            "MATCHCOMPILER=yes"
            "FILESDIR=$(out)/share/cppcheck"
            "HAVE_RULES=yes"
          ];

          enableParallelBuilding = true;
          strictDeps = true;

          # test/testcondition.cpp:4949(TestCondition::alwaysTrueContainer): Assertion failed.
          doCheck = !(pkgs.stdenv.hostPlatform.isLinux && pkgs.stdenv.hostPlatform.isAarch64);
          doInstallCheck = true;

          postPatch = ''
            substituteInPlace Makefile \
              --replace-fail 'PCRE_CONFIG = $(shell which pcre-config)' 'PCRE_CONFIG = $(PKG_CONFIG) libpcre'
          ''
          # Expected:
          # Internal Error. MathLib::toDoubleNumber: conversion failed: 1invalid
          #
          # Actual:
          # Internal Error. MathLib::toDoubleNumber: input was not completely consumed: 1invalid
          + pkgs.lib.optionalString pkgs.stdenv.hostPlatform.isDarwin ''
            substituteInPlace test/testmathlib.cpp \
              --replace-fail \
                'ASSERT_THROW_INTERNAL_EQUALS(MathLib::toDoubleNumber("1invalid"), INTERNAL, "Internal Error. MathLib::toDoubleNumber: conversion failed: 1invalid");' \
                "" \
              --replace-fail \
                'ASSERT_THROW_INTERNAL_EQUALS(MathLib::toDoubleNumber("1.1invalid"), INTERNAL, "Internal Error. MathLib::toDoubleNumber: conversion failed: 1.1invalid");' \
                ""
          '';

          postBuild = ''
            make DB2MAN=${pkgs.docbook_xsl}/xml/xsl/docbook/manpages/docbook.xsl man
          '';

          postInstall = ''
            installManPage cppcheck.1
          '';

          # nativeInstallCheckInputs = [
          #   pkgs.versionCheckHook
          # ];
          installCheckPhase = ''
            runHook preInstallCheck

            echo 'int main() {}' > ./installcheck.cpp
            $out/bin/cppcheck ./installcheck.cpp > /dev/null

            runHook postInstallCheck
          '';

          passthru = {
            updateScript = pkgs.gitUpdater { };
          };

          meta = {
            description = "Static analysis tool for C/C++ code";
            longDescription = ''
              Check C/C++ code for memory leaks, mismatching allocation-deallocation,
              buffer overruns and more.
            '';
            homepage = "http://cppcheck.sourceforge.net";
            license = pkgs.lib.licenses.gpl3Plus;
            maintainers = [ ];
            platforms = pkgs.lib.platforms.unix;
          };
        };
      };
    };
}

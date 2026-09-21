#!/bin/sh

set -ex

######################################################

if [ "$GITHUB_ACTIONS" != true ] ; then
    printf 'this script only run via GitHub Actions.\n' >&2
    exit 1
fi

if [ -z "$1" ] || [ -z "$2" ] ; then
    printf 'Usage: .github/build-core.sh <TARGET-PLATFORM-SPEC>\n' >&2
    exit 1
fi

######################################################

tee ndk-pkg-core.yml <<EOF
summary: core tools for ndk-pkg
web-url: https://github.com/leleliu008/ndk-pkg
src-url: dir://$PWD/core
binbstd: 1
install: |
    for f in *.c
    do
        run "\$CC" "\$CFLAGS" "\$LDFLAGS" -o "_/\${f%.c}" "\$f"
        install_bins _/*
    done
EOF

######################################################

case $2 in
    macos-*)
        curl -L -o core/elf.h https://raw.githubusercontent.com/leleliu008/patches/refs/heads/master/elf.h

        curl -LO https://raw.githubusercontent.com/leleliu008/ppkg/master/ppkg
        chmod a+x ppkg

        ./ppkg about
        ./ppkg setup
        ./ppkg update

        ./ppkg install $2/uppm@0.15.4
        ./ppkg bundle  $2/uppm@0.15.4 .tar.xz

        ./ppkg install $2/elftool
        ./ppkg bundle  $2/elftool .tar.xz

        ./ppkg install $2/ndk-pkg-core -I .
        ./ppkg bundle  $2/ndk-pkg-core .tar.xz
        ;;
    linux-*)
        curl -LO https://raw.githubusercontent.com/leleliu008/ppkg/master/ppkg
        chmod a+x ppkg

        ./ppkg about
        ./ppkg setup
        ./ppkg update

        ./ppkg install uppm@0.15.4 --static
        ./ppkg bundle  uppm@0.15.4 .tar.xz

        ./ppkg install elftool --static
        ./ppkg bundle  elftool .tar.xz

        ./ppkg install ndk-pkg-core --static -I .
        ./ppkg bundle  ndk-pkg-core .tar.xz
        ;;
    android-*)
        ./ndk-pkg about
        ./ndk-pkg setup
        ./ndk-pkg update

        ./ndk-pkg install $2/elftool
        ./ndk-pkg bundle  $2/elftool .tar.xz

        ./ndk-pkg install $2/uppm@0.15.4
        ./ndk-pkg bundle  $2/uppm@0.15.4 .tar.xz

        ./ndk-pkg install $2/ndk-pkg-core -I .
        ./ndk-pkg bundle  $2/ndk-pkg-core .tar.xz
esac

######################################################

for f in *.tar.xz
do
    tar vxf "$f" --strip-components=1
    rm "$f"
done

######################################################

curl -LO https://raw.githubusercontent.com/adobe-fonts/source-code-pro/release/OTF/SourceCodePro-Light.otf

######################################################

install -d bundle.d

mv bin/* *.otf core/fonts.conf bundle.d/

DIRNAME="ndk-pkg-core-$1-$2"
mv bundle.d "$DIRNAME"
tar cJvf "$DIRNAME.tar.xz" "$DIRNAME"

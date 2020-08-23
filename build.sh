#!/bin/sh

version=$(bin/ndk-pkg --version | awk '{print $2}' | head -n 1)
filename=ndk-pkg-$version.tar.gz

Color_Red='\033[0;31m'          # Red
Color_Green='\033[0;32m'        # Green
Color_Purple='\033[0;35m'       # Purple
Color_Off='\033[0m'             # Reset

msg() {
    printf "%b\n" "$1"
}

warn() {
    msg "${Color_Purple}🔥 $@${Color_Off}"
}

die() {
    msg "${Color_Red}[✘] $@${Color_Off}"
    exit 1
}

sed_in_place() {
    if command -v gsed > /dev/null ; then
        gsed -i "$1" "$2"
    elif command -v sed  > /dev/null ; then
        sed -i    "$1" "$2" 2> /dev/null ||
        sed -i "" "$1" "$2"
    else
        die "please install sed utility."
    fi
}

sha256sum() {
    if command -v openssl > /dev/null ; then
        openssl sha256 "$1" | awk '{print $2}'
    elif command -v sha256sum > /dev/null ; then
        sha256sum "$1" | awk '{print $1}'
    else
        die "please install openssl or GNU CoreUtils."
    fi
}

build() {
    clean || exit
    
    tar zvcf $filename bin/ndk-pkg zsh-completion/_ndk-pkg || exit 1
    
    SHA256SUM=$(sha256sum $filename) || exit 1
    
    sed_in_place "s|sha256sums=(.*)|sha256sums=(\'$SHA256SUM\')|" PKGBUILD
    sed_in_place "s|pkgver=.*|pkgver=\'$version\'|" PKGBUILD
    
    sed_in_place "s|VERSION=\'0.1.0\'|VERSION=\'$version\'|" install.sh
    
    if command -v makepkg > /dev/null ; then
        makepkg
    else
        warn "makepkg isn\'t installed."
    fi
}

clean() {
    rm -f  ndk-pkg-$version*.tar.gz
    rm -rf src
    rm -rf pkg
}

main() {
    if [ -z "$1" ] ; then
        build
        exit
    fi

    case $1 in
        build) build;;
        clean) clean;;
        *)     die "$1 : don\'t recognized command."
    esac
}

main $@

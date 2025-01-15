#!/bin/sh

set -e

__install_gcc_via_syspm_on_debian() {
    apt-get -y update
    apt-get -y install g++
}

__install_gcc_via_syspm_on_ubuntu() {
    apt-get -y update
    apt-get -y install g++
}

__install_gcc_via_syspm_on_linuxmint() {
    apt-get -y update
    apt-get -y install g++
}

__install_gcc_via_syspm_on_rocky() {
    dnf -y update
    dnf -y install gcc-c++
}

__install_gcc_via_syspm_on_almalinux() {
    dnf -y update
    dnf -y install gcc-c++
}

__install_gcc_via_syspm_on_centos() {
    dnf -y update
    dnf -y install gcc-c++
}

__install_gcc_via_syspm_on_fedora() {
    dnf -y update
    dnf -y install curl g++
}

__install_gcc_via_syspm_on_rhel() {
:
}

__install_gcc_via_syspm_on_opensuse_leap() {
    zypper update  -y
    zypper install -y gcc-c++
}

__install_gcc_via_syspm_on_gentoo() {
    emerge sys-devel/gcc
}

__install_gcc_via_syspm_on_manjaro() {
    pacman -Syyuu --noconfirm
    pacman -S     --noconfirm gcc
}

__install_gcc_via_syspm_on_arch() {
    pacman -Syyuu --noconfirm
    pacman -S     --noconfirm gcc
}

__install_gcc_via_syspm_on_void() {
    xbps-install -Syu xbps
    xbps-install -S
    xbps-install -Syu gcc
}

__install_gcc_via_syspm_on_alpine() {
    apk update
    apk add gcompat g++ linux-headers
}

__install_gcc_via_syspm_on_Linux() {
    if [ -f /etc/os-release ] ; then
        .   /etc/os-release

        if [ "$ID" = 'opensuse-leap' ] ; then
            ID='opensuse_leap'
        fi

        __install_gcc_via_syspm_on_$ID
    fi
}

NATIVE_OS_KIND="$(uname -s)"

__install_gcc_via_syspm_on_$NATIVE_OS_KIND

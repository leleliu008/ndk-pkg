# Maintainer: fpliu <leleliu008@gmail.com>

pkgname=('ndk-pkg')
pkgver=0.1.0
pkgrel=1
pkgdesc="A package manager for Android NDK"
arch=('any')
license=('custom')
url="https://github.com/leleliu008/ndk-pkg"
makedepends=()
source=(https://github.com/leleliu008/ndk-pkg/releases/download/v0.1.0/${pkgname}-${pkgver}.tar.gz)
sha256sums=('d96670179a4305493f623f71d2295e8998f08da303e123a07c1e726f686924ff')

build() {
    true
}

check() {
    cd "${srcdir}"
    bin/ndk-pkg -V
}

package() {
    depends=('curl' 'tar' 'gzip' 'bzip2' 'xz' 'make' 'cmake' 'vim' 'tree')
    
    mkdir -p ${pkgdir}/usr/bin
    mkdir -p ${pkgdir}/usr/share/zsh/site-functions
    
    cp -f ${srcdir}/bin/ndk-pkg ${pkgdir}/usr/bin/
    cp -f ${srcdir}/zsh-completion/_ndk-pkg ${pkgdir}/usr/share/zsh/site-functions/
}

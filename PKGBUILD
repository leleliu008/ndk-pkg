# Maintainer: fpliu <leleliu008@gmail.com>

pkgname=('ndk-pkg')
pkgver='0.1.0'
pkgrel=1
pkgdesc="A package manager for Android NDK"
arch=('any')
license=('custom')
url="https://github.com/leleliu008/ndk-pkg"
makedepends=()
source=("https://github.com/leleliu008/ndk-pkg/releases/download/v${pkgver}/${pkgname}-${pkgver}.tar.gz")
sha256sums=('5a90bb1beb6a42e4a43a5094215e99ddadcd82cf0b77680c6ea509ad377be0d3')

build() {
    true
}

check() {
    cd "${srcdir}"
    bin/ndk-pkg -V
}

package() {
    depends=('curl' 'tar' 'gzip' 'bzip2' 'xz' 'make' 'cmake' 'vim' 'tree')
    
    mkdir -p ${pkgdir}/usr/local/bin
    mkdir -p ${pkgdir}/usr/local/share/zsh/site-functions
    
    cp -f ${srcdir}/bin/ndk-pkg ${pkgdir}/usr/local/bin/
    cp -f ${srcdir}/zsh-completion/_ndk-pkg ${pkgdir}/usr/local/share/zsh/site-functions/
}

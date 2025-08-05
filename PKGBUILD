# Maintainer: Dein Name <deine@email.tld>
pkgname=qtdmm
pkgver=1.0.0.alpha.1
pkgrel=1
pkgdesc="A DMM readout software including a configurable recorder "
arch=('x86_64')
url="https://github.com/tuxmaster/QtDMM"
license=('GPL3')
provides=('qtdmm')
conflicts=('qtdmm' 'qtdmm-qt5')
depends=('qt6-base' 'qt6-translations' 'qt6-serialport' 'hidapi')
makedepends=('qt6-tools' 'cmake' 'git')
source=("$pkgname::git+https://github.com/tuxmaster/QtDMM.git")
sha256sums=('SKIP')


pkgver() {
  cd $pkgname
  echo "$(git tag --list --sort=-authordate --merged | head -n1)_$(git rev-parse --short HEAD)" | sed 's/\([^-]*-g\)/r\1/;s/-/_/g'
}

build() {
  cd "$pkgname"
  ./compile.sh
}

package() {
  cd "$pkgname"
  echo "---- $pkgdir"
  DESTDIR="$pkgdir" cmake --install build
}

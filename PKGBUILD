# Maintainer: Dein Name <deine@email.tld>
pkgname=qtdmm
pkgver=0.0.0 # placeholder, overwritten by pkgver() below on every build
pkgrel=1
pkgdesc="A DMM readout software including a configurable recorder "
arch=('x86_64')
url="https://github.com/tuxmaster/QtDMM"
license=('GPL3')
provides=('qtdmm')
conflicts=('qtdmm' 'qtdmm-qt5')
depends=('qt6-base' 'qt6-translations' 'qt6-serialport' 'qt6-charts' 'qt6-svg' 'hidapi')
makedepends=('qt6-tools' 'cmake' 'git')
source=("$pkgname::git+https://github.com/tuxmaster/QtDMM.git")
sha256sums=('SKIP')


pkgver() {
  cd $pkgname
  # CalVer from HEAD's commit date, same scheme as cmake/git_version.cmake,
  # plus Arch's usual .rN.gHASH VCS-package suffix so same-day rebuilds still
  # sort correctly. No tags needed, no manual bumping.
  echo "$(git log -1 --format=%cd --date=format:%Y.%m.%d).r$(git rev-list --count HEAD).g$(git rev-parse --short HEAD)"
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

# Maintainer: eNV25 <env252525@gmail.com>
# Contributor: Josip Ponjavic <josipponjavic at gmail dot com>

pkgname=clipsim-git
pkgver=0.0.1
pkgrel=1
pkgdesc="clipsim is a simple X11 clipboard manager written in C."
arch=(x86_64)
url='https://github.com/lucas-mior/clipsim'
license=(AGPL)
depends=(xclip xsel)
optdepends=()
makedepends=(git)
provides=(clipsim)
conflicts=(clipsim)
source=("git+${url}.git")
md5sums=('SKIP')

pkgver() {
    echo "$pkgver"
}

build() {
	cd clipsim
	make
}

package() {
	cd clipsim
	make DESTDIR="${pkgdir}" PREFIX=/usr install
	# make DESTDIR="${pkgdir}" PREFIX=/usr install-desktop

	install -Dm644 completions/fish/clipsim.fish "${pkgdir}/usr/share/fish/vendor_completions.d/clipsim.fish"
	install -Dm644 completions/bash/clipsim-completion.bash "${pkgdir}/usr/share/bash-completion/completions/clipsim"
	install -Dm644 completions/zsh/_clipsim "${pkgdir}/usr/share/zsh/site-functions/_clipsim"

	# install -Dm644 -t "${pkgdir}/usr/share/nnn/quitcd/" misc/quitcd/*

	# cp -a plugins "${pkgdir}/usr/share/nnn/plugins/"

	install -Dm644 -t "${pkgdir}/usr/share/licenses/${pkgname}/" LICENSE
}

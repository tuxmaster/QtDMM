#/bin/sh

usage()
{
cat << EOF

usage: compile.sh <install|clean|qt6>

 builds QtDMM. Additional options:

   appimg : creates appImage
   clean  : remove build files before build
   ctest  : build and run ctest
   install: install system wide
   pack   : create packages (DEB and source)
   run    : run qtdmm after successfull build

EOF
	exit 0
}

RUN=false
INSTALL=false
PACK=false
CTEST=false
APPIMG=false

for arg in $*
do
	arg=$(echo "$arg" | tr '[:upper:]' '[:lower:]')
	[ "$arg" = "clean"   ] && rm -rf build
	[ "$arg" = "ctest"   ] && CTEST=true
	[ "$arg" = "install" ] && INSTALL=true
	[ "$arg" = "run"     ] && RUN=true
	[ "$arg" = "pack"    ] && PACK=true
	[ "$arg" = "appimg"  ] && APPIMG=true
	[ "$arg" = "help"    ] && usage
done

if [ "$(uname)" = "Linux" ] >/dev/null
then
	JOBS=$(nproc)
	CMAKE_PARAMS="-DCMAKE_INSTALL_PREFIX=/usr"
elif [ "$(uname)" = "FreeBSD" ] >/dev/null
then
	JOBS=$(sysctl -n hw.ncpu)
else
	JOBS=$(sysctl -n hw.ncpu)
	CMAKE_PARAMS="-DCMAKE_PREFIX_PATH=$(brew --prefix qt@6)"
fi

cmake ${CMAKE_PARAMS} -DBUILD_TESTING=$(${CTEST} && echo "ON" || echo "OFF") -B build
cmake --build build --parallel ${JOBS} || exit 1

cd build

if ${PACK}
then
	rm -rf ../packages/
	make package_source

	if [ -f /etc/debian_version ]
	then
		make package
	elif [ -f /etc/os-release ] && grep -qiE 'rhel|fedora|suse' /etc/os-release
	then
		cp ../packages/qtdmm*.tar.bz2 ~/rpmbuild/SOURCES/
		( cd ..; rpmbuild -ba QtDMM.spec )
	elif [ "$(uname)" = "Darwin" ]
	then
		echo "mac osx packages not supported yet"
	else
		echo "unsupported OS. no package creation"
	fi
	rm -rf ../packages/_CPack_Packages
fi

if ${CTEST}
then
	echo
	ctest --test-dir . --output-on-failure
	echo
fi

QTDMM_EXE="qtdmm"
[ "$(uname)" = "Darwin" ] && QTDMM_EXE="qtdmm.app/Contents/MacOS/qtdmm"

if [ ! -x ${QTDMM_EXE} ]
then
	echo "build/qtdmm not found"
	exit 1
fi

if [ "$(uname)" = "Linux" ] && ${APPIMG}
then
	rm -rf AppDir appimagetool-x86_64.AppImage ../packages/QtDMM.AppImage
	mkdir -p AppDir/usr/share/metainfo
	wget -q https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage
	chmod +x appimagetool-x86_64.AppImage
	DESTDIR=AppDir make install
	cp -v ../assets/qtdmm.desktop ../qtdmm.png AppDir
	cp -v ../assets/appimage/qtdmm.appdata.xml AppDir/usr/share/metainfo
	echo '#!/bin/sh' > AppDir/AppRun
	echo '$APPDIR/usr/bin/qtdmm' >> AppDir/AppRun
	chmod +x AppDir/AppRun

	for lib in $(ldd -r AppDir/usr/bin/qtdmm | awk '{ print $3 }' | grep -v '^$')
	do
		d="$(dirname "${lib}")"
		mkdir -p "AppDir/$d"
		cp -v "${lib}" "AppDir/$d"
	done

	ARCH=x86_64 ./appimagetool-x86_64.AppImage -n AppDir QtDMM.AppImage
	rm -rf AppDir appimagetool-x86_64.AppImage
	mkdir -p ../packages
	mv QtDMM.AppImage ../packages
fi


mkdir -p ../bin
cp ${QTDMM_EXE} qtdmm*.qm ../bin

if [ "$(uname)" != "Darwin" ] && ${INSTALL}
then
	echo
	echo "-- install QtDMM system wide --"
	sudo make install || exit 1
	${RUN} && ${QTDMM_EXE}
elif ${RUN}
then
	./${QTDMM_EXE} --debug
fi

exit 0

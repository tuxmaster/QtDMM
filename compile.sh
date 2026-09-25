#!/bin/sh

usage()
{
cat << EOF

usage: compile.sh <install|clean|qt6>

 builds QtDMM. Additional options:

   appimage: creates appImage
   clean   : remove build files before build
   ctest   : build and run ctest
   doxygen : generate the developer documentation (build/doxygen/html)
   install : install system wide
   pack    : create packages (DEB and source)
   run     : run qtdmm after successfull build

EOF
	exit 0
}

RUN=false
INSTALL=false
PACK=false
CTEST=false
APPIMG=false
DOXYGEN=false

for arg in $*
do
	arg=$(echo "$arg" | tr '[:upper:]' '[:lower:]')
	[ "$arg" = "clean"    ] && rm -rf build
	[ "$arg" = "ctest"    ] && CTEST=true
	[ "$arg" = "install"  ] && INSTALL=true
	[ "$arg" = "run"      ] && RUN=true
	[ "$arg" = "pack"     ] && PACK=true
	[ "$arg" = "appimage" ] && APPIMG=true
	[ "$arg" = "doxygen"  ] && DOXYGEN=true
	[ "$arg" = "help"     ] && usage
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
	cpack --config CPackSourceConfig.cmake

	if [ -f /etc/debian_version ]
	then
		cpack --config CPackConfig.cmake
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

if ${DOXYGEN}
then
	if command -v doxygen >/dev/null
	then
		cmake --build . --target doxygen || exit 1
		echo "developer documentation: build/doxygen/html/index.html"
	else
		echo "doxygen not installed, skipping developer documentation"
	fi
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
	DESTDIR=AppDir cmake --install .
	# appimagetool looks the metadata up by the desktop file's name
	rm -f AppDir/usr/share/metainfo/io.github.qtdmm.qtdmm.metainfo.xml
	cp -v ../assets/qtdmm.desktop ../qtdmm.png AppDir
	cp -v ../assets/appimage/qtdmm.appdata.xml AppDir/usr/share/metainfo
	# pass the command line on (--version, --config-id, ...); quoted, as the
	# AppImage may be mounted below a path with spaces
	echo '#!/bin/sh' > AppDir/AppRun
	echo 'exec "$APPDIR/usr/bin/qtdmm" "$@"' >> AppDir/AppRun
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
	SUDO=""
	[ "$(id -u)" -ne 0 ] && SUDO="$(command -v sudo || command -v doas)"
	${SUDO} cmake --install . || exit 1
	${RUN} && ${QTDMM_EXE}
elif ${RUN}
then
	./${QTDMM_EXE} #--debug
fi

exit 0

#/bin/sh

usage()
{
cat << EOF

usage: compile.sh <install|clean|qt6>

 builds QtDMM. Additional options:

   install: install system wide
   clean  : remove build files
   qt6    : allow build with qt6
            if qt6 is not available qt5 is used

EOF
	exit 0
}

INSTALL=false
FORCE_QT5="-DFORCE_QT5=ON"

for arg in $*
do
	arg=$(echo "$arg" | tr '[:upper:]' '[:lower:]')
	[ "$arg" = "clean" ] && rm -rf build
	[ "$arg" = "install" ] && INSTALL=true
	[ "$arg" = "qt6" ] && FORCE_QT5=""
	[ "$arg" = "help" -o "$arg" = "h" ] && usage
done

mkdir -p build

cd build
cmake ${FORCE_QT5} ..
make -j $(nproc)

if [ -e src/qtdmm ]
then
	mkdir -p ../bin
	cp qtdmm ../bin
	if ${INSTALL}
	then
		echo
		echo "-- install QtDMM system wide --"
		sudo make install
	fi
fi

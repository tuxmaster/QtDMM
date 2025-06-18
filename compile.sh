#/bin/sh

INSTALL=false
for arg in $*
do
	[ "$arg" = "clean" ] && rm -rf build
	[ "$arg" = "install" ] && INSTALL=true
done

mkdir -p build

cd build
cmake ..
make -j $(nproc)

if [ -e src/qtdmm ]
then
	mkdir -p ../bin
	cp src/qtdmm ../bin
	if ${INSTALL}
	then
		echo
		echo "-- install QtDMM system wide --"
		sudo make install
	fi
fi

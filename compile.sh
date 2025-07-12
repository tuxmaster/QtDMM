#/bin/sh

usage()
{
cat << EOF

usage: compile.sh <install|clean|qt6>

 builds QtDMM. Additional options:

   install: install system wide
   clean  : remove build files before build
   run    : run qtdmm after successfull build
   package: create packages (DEB and source)

EOF
	exit 0
}

RUN=false
INSTALL=false
PACK=false

for arg in $*
do
	arg=$(echo "$arg" | tr '[:upper:]' '[:lower:]')
	[ "$arg" = "clean"   ] && rm -rf build
	[ "$arg" = "install" ] && INSTALL=true
	[ "$arg" = "run"     ] && RUN=true
	[ "$arg" = "pack"    ] && PACK=true
	[ "$arg" = "help"    ] && usage
done

cmake -B build 
cmake --build build --parallel $(nproc) || exit 1

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
	fi
	rm -rf ../packages/_CPack_Packages
fi

if [ -x qtdmm ]
then
	mkdir -p ../bin
	cp qtdmm qtdmm*.qm ../bin
	if ${INSTALL}
	then
		echo
		echo "-- install QtDMM system wide --"
		sudo make install || exit 1
		${RUN} && qtdmm
	elif ${RUN}
	then
		./qtdmm --console
	fi
fi

include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_DIRECTORY "${CMAKE_SOURCE_DIR}/packages")

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}" )
set(CPACK_PACKAGE_VERSION "${QTDMM_PACKAGE_VERSION}")
set(CPACK_PACKAGE_CONTACT "hello@qtdmm.de")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "DMM Readout Software Including a Configurable Recorder.")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_VERBATIM_VARIABLES YES)

if(WIN32)
	## portable zip plus an Inno Setup installer, both from the same install tree.
	## windeployqt (via the generated deploy script) adds the Qt DLLs and plugins.
	set(CPACK_GENERATOR "ZIP;INNOSETUP")
	## file names carry the version as shown (26.1-rc1), '+' of a dev build as '-'
	string(REPLACE "+" "-" _file_version "${QTDMM_VERSION}")
	set(CPACK_PACKAGE_FILE_NAME "${APP_NAME}-${_file_version}-windows-x64")
	set(CPACK_PACKAGE_VERSION "${QTDMM_VERSION}")
	## the Windows file version must be numeric
	set(CPACK_INNOSETUP_SETUP_VersionInfoVersion "${PROJECT_VERSION}")
	set(CPACK_PACKAGE_INSTALL_DIRECTORY "${APP_NAME}")
	set(CPACK_PACKAGE_VENDOR "${APP_ORGANIZATION}")
	set(CPACK_PACKAGE_EXECUTABLES "${PROJECT_NAME};${APP_NAME}")
	set(CPACK_CREATE_DESKTOP_LINKS "${PROJECT_NAME}")
	set(CPACK_INNOSETUP_ARCHITECTURE "x64")
	set(CPACK_INNOSETUP_ICON_FILE "${CMAKE_SOURCE_DIR}/assets/windows/qtdmm.ico")
	set(CPACK_INNOSETUP_INSTALL_ROOT "{autopf}")
	set(CPACK_INNOSETUP_ALLOW_CUSTOM_DIRECTORY ON)

	qt_generate_deploy_app_script(
		TARGET ${PROJECT_NAME}
		OUTPUT_SCRIPT deploy_script
		NO_UNSUPPORTED_PLATFORM_ERROR
	)
	install(SCRIPT ${deploy_script})
elseif(UNIX AND NOT APPLE)
	set(CPACK_GENERATOR "DEB")
	set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "QtDMM is a graphical multimeter reader and logger based on Qt. It reads more than 150 digital multimeters over serial and USB cables, Bluetooth LE and the network.")
	set(CPACK_DEBIAN_PACKAGE_MAINTAINER "QtDMM team <hello@qtdmm.de>")
	set(CPACK_DEBIAN_PACKAGE_SECTION "electronics")
	set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "${PROJECT_HOMEPAGE_URL}")
	## dependencies from the libraries the binary really links (Qt modules,
	## hidapi, Bluetooth when built in), with the package names of the build host
	## - Ubuntu 24.04 renamed the Qt libraries (t64), a fixed list would be wrong
	## somewhere. Needs dpkg-shlibdeps, i.e. a Debian-based build host.
	set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
	set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "qt6-translations-l10n")
	set(CPACK_DEBIAN_PACKAGE_SUGGESTS "sigrok-cli")

	set(CPACK_SOURCE_GENERATOR "TBZ2")
	set(CPACK_SOURCE_IGNORE_FILES \\.git/ bin/ packages/ build/ tmp/ site/ __pycache__/ Testing/ ".*~$")
	set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${QTDMM_PACKAGE_VERSION}")
	set(CPACK_SOURCE_TOPLEVEL_DIRECTORY "${CMAKE_PROJECT_NAME}-${QTDMM_PACKAGE_VERSION}")
	## the tarball has no .git: it carries the version in .tarball-version
	## (read by cmake/git_version.cmake). CPack runs this script after staging
	## the files of every package; only the source package (the one with
	## CPACK_INSTALLED_DIRECTORIES) gets the file. CPACK_PRE_BUILD_SCRIPTS
	## needs CMake 3.19; older ones build the tarball without it.
	file(WRITE "${CMAKE_BINARY_DIR}/tarball-version.txt" "${QTDMM_TARBALL_VERSION}")
	file(WRITE "${CMAKE_BINARY_DIR}/tarball-version.cmake"
		"if (CPACK_INSTALLED_DIRECTORIES)\n"
		"  configure_file(\"${CMAKE_BINARY_DIR}/tarball-version.txt\" \"\${CPACK_TEMPORARY_DIRECTORY}/.tarball-version\" COPYONLY)\n"
		"endif()\n")
	set(CPACK_PRE_BUILD_SCRIPTS "${CMAKE_BINARY_DIR}/tarball-version.cmake")

	## RPM wants "* Www Mmm DD YYYY Name <mail> - version-release" entries; the
	## CHANGELOG file (shipped as %doc) has its own format, so the spec gets one
	## entry for this version, dated with the commit date
	set(_y ${QTDMM_COMMIT_YEAR})
	set(_m ${QTDMM_COMMIT_MONTH})
	set(_d ${QTDMM_COMMIT_DAY})
	# day of the week (Zeller's congruence, 0 = Saturday)
	if (_m LESS 3)
		math(EXPR _zm "${_m} + 12")
		math(EXPR _zy "${_y} - 1")
	else()
		set(_zm ${_m})
		set(_zy ${_y})
	endif()
	math(EXPR _wd "(${_d} + (13 * (${_zm} + 1)) / 5 + ${_zy} % 100 + (${_zy} % 100) / 4 + (${_zy} / 100) / 4 + 5 * (${_zy} / 100)) % 7")
	set(_days Sat Sun Mon Tue Wed Thu Fri)
	set(_months Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec)
	list(GET _days ${_wd} _wday)
	math(EXPR _mi "${_m} - 1")
	list(GET _months ${_mi} _mon)
	if (_d LESS 10)
		set(_d "0${_d}")
	endif()
	set(RPM_CHANGELOG "* ${_wday} ${_mon} ${_d} ${_y} QtDMM team <hello@qtdmm.de> - ${QTDMM_PACKAGE_VERSION}-1\n- QtDMM ${QTDMM_VERSION}; the changes are listed in CHANGELOG")
#	configure_file(${CMAKE_SOURCE_DIR}/QtDMM.spec.in ${CMAKE_SOURCE_DIR}/QtDMM.spec @ONLY)
endif()

include(CPack)

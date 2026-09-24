include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_DIRECTORY "${CMAKE_SOURCE_DIR}/packages")

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}" )
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}" ) 
set(CPACK_PACKAGE_CONTACT "hello@qtdmm.de")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "DMM Readout Software Including a Configurable Recorder.")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_VERBATIM_VARIABLES YES)

if(WIN32)
	## portable zip plus an Inno Setup installer, both from the same install tree.
	## windeployqt (via the generated deploy script) adds the Qt DLLs and plugins.
	set(CPACK_GENERATOR "ZIP;INNOSETUP")
	set(CPACK_PACKAGE_FILE_NAME "${APP_NAME}-${PROJECT_VERSION}-windows-x64")
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
	set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${PROJECT_VERSION}")
	set(CPACK_SOURCE_TOPLEVEL_DIRECTORY "${CMAKE_PROJECT_NAME}-${PROJECT_VERSION}")

	## RPM wants "* Www Mmm DD YYYY Name <mail> - version-release" entries; the
	## CHANGELOG file (shipped as %doc) has its own format, so the spec gets one
	## entry for this version, dated like the version (the commit date)
	set(_y ${PROJECT_VERSION_MAJOR})
	set(_m ${PROJECT_VERSION_MINOR})
	set(_d ${PROJECT_VERSION_PATCH})
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
	set(RPM_CHANGELOG "* ${_wday} ${_mon} ${_d} ${_y} QtDMM team <hello@qtdmm.de> - ${PROJECT_VERSION}-1\n- Build of ${PROJECT_VERSION_FULL}; the changes are listed in CHANGELOG")
	configure_file(${CMAKE_SOURCE_DIR}/QtDMM.spec.in ${CMAKE_SOURCE_DIR}/QtDMM.spec @ONLY)
endif()

include(CPack)

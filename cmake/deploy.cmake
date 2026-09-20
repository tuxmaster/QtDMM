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
else()
	set(CPACK_GENERATOR "DEB")
	set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "QtDMM is a graphical multimeter reader and logger based on Qt. It supports various serial devices.")
	set(CPACK_DEBIAN_PACKAGE_MAINTAINER "redPanther <redpanther@spooky-onlinde.de>")
	set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6core6 (>= 6.4), libqt6gui6 (>= 6.4), libqt6widgets6 (>= 6.4), libqt6serialport6 (>= 6.4), libhidapi-hidraw0 (>= 0.10)")

	set(CPACK_SOURCE_GENERATOR "TBZ2")
	set(CPACK_SOURCE_IGNORE_FILES \\.git/ bin/ packages/  build/ ".*~$")
	set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${PROJECT_VERSION}")
	set(CPACK_SOURCE_TOPLEVEL_DIRECTORY "${CMAKE_PROJECT_NAME}-${PROJECT_VERSION}")

	file(READ "${CMAKE_SOURCE_DIR}/CHANGELOG" RPM_CHANGELOG)
	configure_file(${CMAKE_SOURCE_DIR}/QtDMM.spec.in ${CMAKE_SOURCE_DIR}/QtDMM.spec @ONLY)
endif()

include(CPack)

include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_DIRECTORY "${CMAKE_SOURCE_DIR}/packages")

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}" )
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}" ) 
set(CPACK_PACKAGE_CONTACT "${PROJECT_HOMEPAGE_URL}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "DMM Readout Software Including a Configurable Recorder.")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "QtDMM is a graphical multimeter reader and logger based on Qt. It supports various serial devices.")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "redPanther <redpanther@spooky-onlinde.de>")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6core6 (>= 6.4), libqt6gui6 (>= 6.4), libqt6widgets6 (>= 6.4), libqt6serialport6 (>= 6.4), libhidapi-hidraw0 (>= 0.10)")

set(CPACK_SOURCE_GENERATOR "TBZ2")
set(CPACK_SOURCE_IGNORE_FILES \\.git/ bin/ packages/  build/ ".*~$")
set(CPACK_VERBATIM_VARIABLES YES)
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${PROJECT_VERSION}")
set(CPACK_SOURCE_TOPLEVEL_DIRECTORY "${CMAKE_PROJECT_NAME}-${PROJECT_VERSION}")

file(READ "${CMAKE_SOURCE_DIR}/CHANGELOG" RPM_CHANGELOG)
configure_file(${CMAKE_SOURCE_DIR}/QtDMM.spec.in ${CMAKE_SOURCE_DIR}/QtDMM.spec @ONLY)

include(CPack)

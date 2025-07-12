include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_DIRECTORY "${CMAKE_SOURCE_DIR}/packages")

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}" )
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}" ) 
set(CPACK_PACKAGE_CONTACT "https://github.com/tuxmaster/QtDMM")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "QtDMM is a DMM readout and recording software")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "redPanther <redpanther@spooky-onlinde.de>")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6core6 (>= 6.4), libqt6gui6 (>= 6.4), libqt6widgets6 (>= 6.4), libqt6serialport6 (>= 6.4)")

set(CPACK_SOURCE_GENERATOR "TBZ2")
set(CPACK_SOURCE_IGNORE_FILES \\.git/ bin/ packages/  build/ ".*~$")
set(CPACK_VERBATIM_VARIABLES YES)

include(CPack)

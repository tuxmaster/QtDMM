
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "qtdmm")
set(CPACK_PACKAGE_VERSION "0.9.0") # oder aus Projektversion ziehen
set(CPACK_PACKAGE_CONTACT "Dein Name <deine@mail.tld>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Qt-basierter Digitalmultimeter-Reader")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README")

# Setze das Format-bezogene Verhalten
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Dein Name") # für deb
set(CPACK_RPM_PACKAGE_LICENSE "GPLv2")           # für rpm

include(CPack)

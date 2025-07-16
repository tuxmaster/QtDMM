cmake_minimum_required(VERSION 3.11)
project(qtdmm VERSION "1.0.0" HOMEPAGE_URL "https://github.com/tuxmaster/QtDMM/" LANGUAGES CXX)

## main settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(PROJECT_LICENSE "GPL-3.0")
set(TRANSLATIONS "assets/translations/${PROJECT_NAME}_de.ts")
set(RESOURCES "src/resources.qrc")
set(QT_COMPONENTS Widgets SerialPort PrintSupport LinguistTools)

## get files
file(GLOB DRV_HEADER_FILES CONFIGURE_DEPENDS src/decoders/*.h )
file(GLOB UI_FILES CONFIGURE_DEPENDS src/ui/*.ui)
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp src/*.h )
file(GLOB XPM_FILES CONFIGURE_DEPENDS assets/icons/*.xpm )

## generate resource.qrc so you can't forget to add the icons there
list(SORT XPM_FILES)
set(RES_CONTENT "<!-- ATTENTION this file is auto generated -->\n<RCC>\n    <qresource prefix=\"/Symbols\">\n")
foreach(file_path IN LISTS XPM_FILES)
	get_filename_component(base_name "${file_path}" NAME)
	set(RES_CONTENT "${RES_CONTENT}        <file alias=\"${base_name}\">../assets/icons/${base_name}</file>\n")
endforeach()
file(WRITE "src/resources.qrc" "${RES_CONTENT}    </qresource>\n</RCC>\n")

## generate header file for drivers, to make adding a driver easier
set(DRIVER_HEADER_CONTENT "#pragma once\n\n")
foreach(file_path IN LISTS DRV_HEADER_FILES)
	get_filename_component(base_name "${file_path}" NAME)
	set(DRIVER_HEADER_CONTENT "${DRIVER_HEADER_CONTENT}#include \"decoders/${base_name}\"\n")
endforeach()
file(WRITE "${CMAKE_BINARY_DIR}/decoders.h" "${DRIVER_HEADER_CONTENT}")

## compile the application
add_definitions(-DAPP_VERSION="${PROJECT_VERSION}" -DAPP_NAME="QtDMM" -DAPP_ORGANIZATION="QtDMM" )
include_directories(src ${CMAKE_BINARY_DIR})

find_package(Qt6 REQUIRED COMPONENTS ${QT_COMPONENTS})
set(QT_NAMESPACE Qt6)

qt_wrap_ui(UI_HEADERS ${UI_FILES})
qt_add_resources(RES_SOURCES ${RESOURCES})
qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR}/src ${TRANSLATIONS})

add_executable(${PROJECT_NAME}
	${SRC_FILES}
	${UI_HEADERS}
	${RES_SOURCES}
	${QM_FILES}
)

target_link_libraries(${PROJECT_NAME}
	${QT_NAMESPACE}::Widgets
	${QT_NAMESPACE}::SerialPort
	${QT_NAMESPACE}::PrintSupport
)

## install files
install(TARGETS ${PROJECT_NAME} 
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib
)

install(FILES ${CMAKE_BINARY_DIR}/qtdmm_de.qm DESTINATION "share/qtdmm/translations" )
install(FILES assets/qtdmm.desktop DESTINATION share/applications)
install(FILES assets/icons/qtdmm_32.png DESTINATION share/icons/hicolor/32x32/apps RENAME qtdmm.png)
install(FILES assets/icons/qtdmm_64.png DESTINATION share/icons/hicolor/64x64/apps RENAME qtdmm.png)
install(FILES assets/icons/qtdmm_128.png DESTINATION share/icons/hicolor/128x128/apps RENAME qtdmm.png)

## include deployment 
include(deploy.cmake)

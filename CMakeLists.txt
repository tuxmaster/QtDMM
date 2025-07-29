cmake_minimum_required(VERSION 3.11)
project(qtdmm VERSION "1.0.0" HOMEPAGE_URL "https://github.com/tuxmaster/QtDMM/" LANGUAGES CXX)

## main settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(PROJECT_LICENSE "GPL-3.0")
set(RESOURCES "src/resources.qrc")
set(APP_NAME "QtDMM")
set(APP_ORGANIZATION "QtDMM")
set(QT_COMPONENTS Widgets SerialPort PrintSupport Test LinguistTools Network)


## get files
file(GLOB TRANSLATIONS CONFIGURE_DEPENDS assets/translations/${PROJECT_NAME}_*.ts)
file(GLOB DECODER_HEADER_FILES CONFIGURE_DEPENDS src/decoders/*.h )
file(GLOB UI_FILES CONFIGURE_DEPENDS src/ui/*.ui)
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp src/*.h )
file(GLOB XPM_FILES CONFIGURE_DEPENDS assets/icons/*.xpm )


## generate resource.qrc so you can't forget to add the icons and translations
list(SORT XPM_FILES)
set(RES_CONTENT "    <qresource prefix=\"/Symbols\">\n")
foreach(file_path IN LISTS XPM_FILES)
	get_filename_component(base_name "${file_path}" NAME)
	set(RES_CONTENT "${RES_CONTENT}        <file alias=\"${base_name}\">../assets/icons/${base_name}</file>\n")
endforeach()
file(WRITE "src/resources.qrc" "${RES_CONTENT}    </qresource>\n")
set(RES_CONTENT "${RES_CONTENT}    </qresource>\n")

set(RES_CONTENT "${RES_CONTENT}    <qresource prefix=\"/Translations\">\n")
foreach(translation_file IN LISTS TRANSLATIONS)
	string(REGEX REPLACE "^.*/${PROJECT_NAME}_([^.]+)\\.ts$" "\\1" lang ${translation_file})
	file(RELATIVE_PATH translation_file ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/${PROJECT_NAME}_${lang}.qm)
	set(RES_CONTENT "${RES_CONTENT}        <file alias=\"${lang}\">../${translation_file}</file>\n")
endforeach()
set(RES_CONTENT "${RES_CONTENT}    </qresource>\n")

file(WRITE "src/resources.qrc" "<!-- ATTENTION this file is auto generated -->\n<RCC>\n${RES_CONTENT}</RCC>\n")

## generate header file for drivers, to make adding a driver easier
set(DRIVER_HEADER_CONTENT "#pragma once\n\n")
foreach(file_path IN LISTS DECODER_HEADER_FILES)
	get_filename_component(base_name "${file_path}" NAME)
	set(DRIVER_HEADER_CONTENT "${DRIVER_HEADER_CONTENT}#include \"decoders/${base_name}\"\n")
endforeach()
file(WRITE "${CMAKE_BINARY_DIR}/decoders.h" "${DRIVER_HEADER_CONTENT}")

## find external libs
find_package(Qt6 REQUIRED COMPONENTS ${QT_COMPONENTS})
find_package(PkgConfig REQUIRED)

qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR}/src ${TRANSLATIONS})
qt_wrap_ui(UI_HEADERS ${UI_FILES})
qt_add_resources(RES_SOURCES ${RESOURCES})

if(APPLE)
	pkg_check_modules(HIDAPI REQUIRED hidapi)
	set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.qtdmm.qtdmm")
	set(MACOSX_BUNDLE_BUNDLE_NAME "${APP_NAME}")
	set(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}")
	set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}")
	set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")
elseif(UNIX)
	pkg_check_modules(HIDAPI REQUIRED hidapi-hidraw)
endif()

## compile the application
add_definitions(-DAPP_VERSION="${PROJECT_VERSION}" -DAPP_NAME="${APP_NAME}" -DAPP_ORGANIZATION="${APP_ORGANIZATION}" )
include_directories(src ${CMAKE_BINARY_DIR} ${HIDAPI_INCLUDE_DIRS})

if (HIDAPI_FOUND)
	link_directories(${HIDAPI_LIBRARY_DIRS})
endif()

add_executable(${PROJECT_NAME}
	MACOSX_BUNDLE
	${SRC_FILES}
	${UI_HEADERS}
	${RES_SOURCES}
	${QM_FILES}
)

# paranoia options, force you to correct all compiler warnings
#if(MSVC)
#	target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
#else()
#	target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
#endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
	Qt6::Widgets
	Qt6::SerialPort
	Qt6::PrintSupport
	Qt6::Network
	${HIDAPI_LIBRARIES}
)

## install files
install(TARGETS ${PROJECT_NAME} 
	BUNDLE DESTINATION .
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib
)

install(FILES assets/qtdmm.desktop DESTINATION share/applications)
install(FILES assets/icons/qtdmm_32.png DESTINATION share/icons/hicolor/32x32/apps RENAME qtdmm.png)
install(FILES assets/icons/qtdmm_64.png DESTINATION share/icons/hicolor/64x64/apps RENAME qtdmm.png)
install(FILES assets/icons/qtdmm_128.png DESTINATION share/icons/hicolor/128x128/apps RENAME qtdmm.png)

## include test framework
include(cmake/ctest.cmake)

## include deployment 
include(cmake/deploy.cmake)

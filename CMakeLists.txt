cmake_minimum_required(VERSION 3.16)
project(qtdmm VERSION "0.0.0" HOMEPAGE_URL "https://github.com/tuxmaster/QtDMM/" LANGUAGES CXX)

## main settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(PROJECT_LICENSE "GPL-3.0")
set(RESOURCES "src/resources.qrc")
set(APP_NAME "QtDMM")
set(APP_ORGANIZATION "QtDMM")
set(QT_COMPONENTS Widgets SerialPort PrintSupport Test LinguistTools Network Charts)

include( cmake/git_version.cmake )

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

## in-app help: the user guide pages, aliased by their path below docs/user so
## relative Markdown links resolve the same way in QTextBrowser and in MkDocs
file(GLOB_RECURSE HELP_FILES CONFIGURE_DEPENDS docs/user/*.md docs/user/*.png docs/user/*.svg docs/user/*.jpg)
list(SORT HELP_FILES)
set(RES_CONTENT "${RES_CONTENT}    <qresource prefix=\"/Help\">\n")
foreach(file_path IN LISTS HELP_FILES)
	file(RELATIVE_PATH alias "${CMAKE_SOURCE_DIR}/docs/user" "${file_path}")
	file(RELATIVE_PATH rel "${CMAKE_SOURCE_DIR}/src" "${file_path}")
	set(RES_CONTENT "${RES_CONTENT}        <file alias=\"${alias}\">${rel}</file>\n")
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

qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR}/src ${TRANSLATIONS})
qt_wrap_ui(UI_HEADERS ${UI_FILES})
qt_add_resources(RES_SOURCES ${RESOURCES})

## hidapi: distro package on Unix (keeps DEB/RPM/AUR dependencies honest),
## built from source on Windows where there is no pkg-config and no package
## manager to rely on. Either way the result is one imported target.
if(WIN32)
	include(FetchContent)
	FetchContent_Declare(hidapi
		GIT_REPOSITORY https://github.com/libusb/hidapi.git
		GIT_TAG hidapi-0.14.0
		GIT_SHALLOW TRUE
	)
	set(HIDAPI_WITH_TESTS OFF CACHE BOOL "" FORCE)
	set(HIDAPI_BUILD_HIDTEST OFF CACHE BOOL "" FORCE)
	set(BUILD_SHARED_LIBS OFF)
	# hidapi 0.14 still declares cmake_minimum_required 3.1.3, which CMake >= 4
	# refuses outright; this lets it configure under the 3.5 policy set.
	set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
	FetchContent_MakeAvailable(hidapi)
	unset(CMAKE_POLICY_VERSION_MINIMUM)
	set(HIDAPI_TARGET hidapi::hidapi)
else()
	find_package(PkgConfig REQUIRED)
	if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
		pkg_check_modules(HIDAPI REQUIRED IMPORTED_TARGET hidapi-hidraw)
	else()
		pkg_check_modules(HIDAPI REQUIRED IMPORTED_TARGET hidapi)
	endif()
	set(HIDAPI_TARGET PkgConfig::HIDAPI)
endif()

if(APPLE)
	set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.qtdmm.qtdmm")
	set(MACOSX_BUNDLE_BUNDLE_NAME "${APP_NAME}")
	set(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}")
	set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}")
	set(CMAKE_OSX_ARCHITECTURES "arm64") #x86_64;
endif()

## compile the application
add_definitions(-DAPP_VERSION="${PROJECT_VERSION_FULL}" -DAPP_NAME="${APP_NAME}" -DAPP_ORGANIZATION="${APP_ORGANIZATION}" )
include_directories(src ${CMAKE_BINARY_DIR})

if(WIN32)
	enable_language(RC)
	set(WIN_RESOURCES assets/windows/qtdmm.rc) # executable icon
endif()

add_executable(${PROJECT_NAME}
	MACOSX_BUNDLE
	WIN32
	${SRC_FILES}
	${UI_HEADERS}
	${RES_SOURCES}
	${QM_FILES}
	${WIN_RESOURCES}
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
	Qt6::Charts
	${HIDAPI_TARGET}
)

## install files
install(TARGETS ${PROJECT_NAME}
	BUNDLE DESTINATION .
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib
)

if (NOT WIN32)
	file( COPY ${CMAKE_SOURCE_DIR}/docs/qtdmm.1 DESTINATION ${CMAKE_BINARY_DIR} )
	execute_process( COMMAND gzip -f ${CMAKE_BINARY_DIR}/qtdmm.1 )

	install(FILES assets/qtdmm.desktop DESTINATION share/applications)
	install(FILES assets/icons/qtdmm_32.png DESTINATION share/icons/hicolor/32x32/apps RENAME qtdmm.png)
	install(FILES assets/icons/qtdmm_64.png DESTINATION share/icons/hicolor/64x64/apps RENAME qtdmm.png)
	install(FILES assets/icons/qtdmm_128.png DESTINATION share/icons/hicolor/128x128/apps RENAME qtdmm.png)
	install(FILES ${CMAKE_BINARY_DIR}/qtdmm.1.gz DESTINATION share/man/man1)
endif()

## include test framework
include(cmake/ctest.cmake)

## include deployment
include(cmake/deploy.cmake)

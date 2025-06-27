cmake_minimum_required(VERSION 3.11)
project(qtdmm VERSION "1.0.0" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(QT5_MIN_VERSION "5.14")

set(TRANSLATIONS "assets/translations/qtdmm_de.ts")
set(RESOURCES "src/resources.qrc")
set(QT_COMPONENTS Widgets SerialPort PrintSupport LinguistTools)

add_definitions(-DAPP_VERSION="${PROJECT_VERSION}" -DAPP_NAME="QtDMM" -DAPP_ORGANIZATION="QtDMM" )
include_directories(src ${CMAKE_BINARY_DIR})

file(GLOB UI_FILES CONFIGURE_DEPENDS src/ui/*.ui)
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp src/*.h )
file(GLOB DRV_HEADER_FILES CONFIGURE_DEPENDS src/drivers/*.h )

set(DRIVER_HEADER_CONTENT "#pragma once\n\n")
foreach(file_path IN LISTS DRV_HEADER_FILES)
    get_filename_component(base_name "${file_path}" NAME)
    set(DRIVER_HEADER_CONTENT "${DRIVER_HEADER_CONTENT}#include \"drivers/${base_name}\"\n")
endforeach()
file(WRITE "${CMAKE_BINARY_DIR}/drivers.h" "${DRIVER_HEADER_CONTENT}")

find_package(Qt6 QUIET)
if(Qt6_FOUND AND NOT FORCE_QT5)
	message(STATUS "Building with Qt6")
	find_package(Qt6 REQUIRED COMPONENTS ${QT_COMPONENTS})
	set(QT_NAMESPACE Qt6)

	include_directories(${Qt6Core5Compat_INCLUDE_DIRS})
	qt_wrap_ui(UI_HEADERS ${UI_FILES})
	qt_add_resources(RES_SOURCES ${RESOURCES})
	qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TRANSLATIONS})
else()
	message(STATUS "Building with Qt5")
	find_package(Qt5 ${QT5_MIN_VERSION} REQUIRED COMPONENTS ${QT_COMPONENTS})
	set(QT_NAMESPACE Qt5)

	qt5_wrap_ui(UI_HEADERS ${UI_FILES})
	qt5_add_resources(RES_SOURCES ${RESOURCES})
	qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TRANSLATIONS})
endif()

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

install(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib
)

include(deploy.cmake)


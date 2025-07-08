cmake_minimum_required(VERSION 3.11)
project(qtdmm VERSION "1.0.0" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(TRANSLATIONS "assets/translations/qtdmm_de.ts")
set(RESOURCES "src/resources.qrc")
set(QT_COMPONENTS Widgets SerialPort PrintSupport LinguistTools)

add_definitions(-DAPP_VERSION="${PROJECT_VERSION}" -DAPP_NAME="QtDMM" -DAPP_ORGANIZATION="QtDMM" )
include_directories(src ${CMAKE_BINARY_DIR})

file(GLOB UI_FILES CONFIGURE_DEPENDS src/ui/*.ui)
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp src/*.h )

find_package(Qt6 REQUIRED COMPONENTS ${QT_COMPONENTS})
set(QT_NAMESPACE Qt6)

qt_wrap_ui(UI_HEADERS ${UI_FILES})
qt_add_resources(RES_SOURCES ${RESOURCES})
qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TRANSLATIONS})

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


cmake_minimum_required(VERSION 3.11)
project(qtdmm)

set (CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(QT_MIN_VERSION "5.14")

set(TRANSLATIONS "src/translations/qtdmm_de.ts")
set(RESOURCES "src/Resources.qrc")
set(QT_COMPONENTS Widgets SerialPort PrintSupport LinguistTools)

include_directories(src/sources/)

file(GLOB_RECURSE UI_FILES CONFIGURE_DEPENDS src/forms/*.ui)
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/sources/*.cpp src/sources/*.h)

if(FORCE_QT5)
	message(STATUS "Building with Qt5")
	find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS ${QT_COMPONENTS})
	set(QT_NAMESPACE Qt5)

	qt5_wrap_ui(UI_HEADERS ${UI_FILES})
	qt5_add_resources(RES_SOURCES ${RESOURCES})
	qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TRANSLATIONS})
else()
	message(STATUS "Building with Qt6")
	find_package(Qt6 REQUIRED COMPONENTS ${QT_COMPONENTS} Core5Compat)
	set(QT_NAMESPACE Qt6)

	include_directories(${Qt6Core5Compat_INCLUDE_DIRS})
	qt_wrap_ui(UI_HEADERS ${UI_FILES})
	qt_add_resources(RES_SOURCES ${RESOURCES})
	qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TRANSLATIONS})
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

if(Qt6_FOUND)
	target_link_libraries(${PROJECT_NAME} ${QT_NAMESPACE}::Core5Compat)
endif()

install(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib
)


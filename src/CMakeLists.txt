find_package(Qt5 COMPONENTS Widgets SerialPort PrintSupport LinguistTools REQUIRED)
set (CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(
	${Qt5Widgets_INCLUDE_DIRS}
	${Qt5SerialPort_INCLUDE_DIRS}
	${Qt5PrintSupport_INCLUDE_DIRS}
	${Qt5LinguistTools_INCLUDE_DIRS}
	${CMAKE_SOURCE_DIR}/src/sources/
)

set(CMAKE_AUTOMOC ON)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.11.0")
    file(GLOB_RECURSE UI_FILES CONFIGURE_DEPENDS forms/*.ui)
else()
    file(GLOB UI_FILES forms/*.ui )
endif()

qt5_wrap_ui(UI_GENERATED_HEADERS ${UI_FILES})
qt5_add_resources(RES_SOURCES Resources.qrc)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.11.0")
    file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS sources/*.cpp sources/*.h)
else()
    file(GLOB SRC_FILES sources/*.cpp sources/*.h )
endif()

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} translations/qtdmm_de.ts)

add_definitions(${Qt5Widgets_DEFINITIONS})

set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS} ${CMAKE_CXX_FLAGS}")

add_executable(${PROJECT_NAME}
	${SRC_FILES}
	${UI_GENERATED_HEADERS}
	${RES_SOURCES}
	${QM_FILES}
)

target_link_libraries(${PROJECT_NAME}
	${Qt5Widgets_LIBRARIES} 
	${Qt5SerialPort_LIBRARIES} 
	${Qt5PrintSupport_LIBRARIES}
	${Qt5LinguistTools_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib
)

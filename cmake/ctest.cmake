if (BUILD_TESTING)
	set( TEST_DECODER test_decoder)
	enable_testing()

	file(GLOB DECODER_FILES CONFIGURE_DEPENDS src/decoders/*.h  src/decoders/*.cpp )
	add_executable(${TEST_DECODER} MACOSX_BUNDLE tests/test_decoder.cpp src/dmmdecoder.cpp src/siprefix.cpp ${DECODER_FILES})
	target_link_libraries(${TEST_DECODER} PRIVATE Qt::Core Qt::Test)

	file(GLOB TEST_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/tests/data/decoder/*.json")
	foreach(test_file ${TEST_FILES})
		get_filename_component(name ${test_file} NAME_WE)
		add_test(NAME ${name} COMMAND ${TEST_DECODER} ${test_file})
	endforeach()

	set( TEST_GRAPH test_graph)
	add_executable(${TEST_GRAPH} MACOSX_BUNDLE tests/test_graph.cpp src/dmmgraph.cpp src/settings.cpp src/siprefix.cpp src/engnumbervalidator.cpp)
	target_link_libraries(${TEST_GRAPH} PRIVATE Qt6::Widgets Qt6::PrintSupport Qt6::Charts Qt::Core Qt::Test)
	add_test(NAME dmmgraph COMMAND ${TEST_GRAPH} "${CMAKE_SOURCE_DIR}/tests/data/graph")

	## the handbook: same compiled resources as the application, so the test
	## sees exactly the pages the user gets under :/Help/
	set( TEST_HELP test_help)
	add_executable(${TEST_HELP} MACOSX_BUNDLE tests/test_help.cpp src/helpdlg.cpp src/settings.cpp ${RES_SOURCES})
	target_include_directories(${TEST_HELP} PRIVATE src)
	target_link_libraries(${TEST_HELP} PRIVATE Qt6::Widgets Qt::Core)
	add_test(NAME handbook COMMAND ${TEST_HELP})

	## generated documents must match their sources (device table from the
	## decoders, README from docs/)
	find_package(Python3 COMPONENTS Interpreter)
	if (Python3_Interpreter_FOUND)
		add_test(NAME docs_generated COMMAND ${Python3_EXECUTABLE} "${CMAKE_SOURCE_DIR}/tests/generate_docs.py" --check)
	endif()
endif()

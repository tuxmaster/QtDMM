if (BUILD_TESTING)
	set( TEST_DECODER test_decoder)
	enable_testing()

	file(GLOB DECODER_FILES CONFIGURE_DEPENDS src/decoders/*.h  src/decoders/*.cpp )
	add_executable(${TEST_DECODER} MACOSX_BUNDLE tests/test_decoder.cpp src/dmmdecoder.cpp ${DECODER_FILES})
	target_link_libraries(${TEST_DECODER} PRIVATE Qt::Core Qt::Test)

	file(GLOB TEST_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/tests/data/decoder/*.json")
	foreach(test_file ${TEST_FILES})
		get_filename_component(name ${test_file} NAME_WE)
		add_test(NAME ${name} COMMAND ${TEST_DECODER} ${test_file})
	endforeach()

	set( TEST_GRAPH test_graph)
	add_executable(${TEST_GRAPH} MACOSX_BUNDLE tests/test_graph.cpp src/dmmgraph.cpp src/settings.cpp)
	target_link_libraries(${TEST_GRAPH} PRIVATE Qt6::Widgets Qt6::PrintSupport Qt::Core Qt::Test)
	add_test(NAME dmmgraph COMMAND ${TEST_GRAPH} "${CMAKE_SOURCE_DIR}/tests/data/graph")
endif()

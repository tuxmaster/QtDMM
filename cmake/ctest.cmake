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
	## links the same compiled resources as the application (qtdmm_resources
	## also carries the generated ui header helpdlg.cpp needs)
	add_executable(${TEST_HELP} MACOSX_BUNDLE tests/test_help.cpp src/helpdlg.cpp src/settings.cpp)
	target_include_directories(${TEST_HELP} PRIVATE src)
	target_link_libraries(${TEST_HELP} PRIVATE qtdmm_resources Qt6::Widgets Qt::Core)
	add_test(NAME handbook COMMAND ${TEST_HELP})

	## the analog meter: angle mapping, full-scale derivation, ballistics and
	## a headless render check
	set( TEST_METER test_meter)
	add_executable(${TEST_METER} MACOSX_BUNDLE tests/test_meter.cpp src/meterwid.cpp src/panelframe.cpp)
	target_include_directories(${TEST_METER} PRIVATE src)
	target_link_libraries(${TEST_METER} PRIVATE Qt6::Widgets Qt::Core Qt::Test)
	add_test(NAME analog_meter COMMAND ${TEST_METER})

	## the digital display: glyph table and a headless render check
	set( TEST_DISPLAY test_display)
	add_executable(${TEST_DISPLAY} MACOSX_BUNDLE tests/test_display.cpp src/displaywid.cpp src/panelframe.cpp src/siprefix.cpp)
	target_include_directories(${TEST_DISPLAY} PRIVATE src)
	target_link_libraries(${TEST_DISPLAY} PRIVATE Qt6::Widgets Qt::Core)
	add_test(NAME digital_display COMMAND ${TEST_DISPLAY})

	## instance coordination over shared memory: registration, state channel
	## and published readings
	set( TEST_SHAREDSTATE test_sharedstate)
	add_executable(${TEST_SHAREDSTATE} MACOSX_BUNDLE tests/test_sharedstate.cpp src/sharedstatemanager.cpp)
	target_include_directories(${TEST_SHAREDSTATE} PRIVATE src)
	target_link_libraries(${TEST_SHAREDSTATE} PRIVATE Qt::Core)
	add_test(NAME shared_state COMMAND ${TEST_SHAREDSTATE})
	## the formula evaluator of calculated instances
	set( TEST_CALC test_calc)
	add_executable(${TEST_CALC} MACOSX_BUNDLE tests/test_calc.cpp src/calcexpr.cpp src/siprefix.cpp)
	target_include_directories(${TEST_CALC} PRIVATE src)
	target_link_libraries(${TEST_CALC} PRIVATE Qt::Core)
	add_test(NAME calc_expression COMMAND ${TEST_CALC})

	## the calculated-value source: formula over the other instances' readings,
	## checked through the real ASCII decoder
	set( TEST_CALC_DEVICE test_calc_device)
	add_executable(${TEST_CALC_DEVICE} MACOSX_BUNDLE tests/test_calc_device.cpp src/portdevices/calc.cpp src/calcexpr.cpp
		src/sharedstatemanager.cpp src/dmmdecoder.cpp src/siprefix.cpp ${DECODER_FILES})
	target_include_directories(${TEST_CALC_DEVICE} PRIVATE src)
	target_link_libraries(${TEST_CALC_DEVICE} PRIVATE Qt::Core)
	add_test(NAME calc_device COMMAND ${TEST_CALC_DEVICE})

	## HID cable chips: report layouts and chip detection, no hardware needed
	set( TEST_HID test_hid)
	add_executable(${TEST_HID} MACOSX_BUNDLE tests/test_hid.cpp src/portdevices/hidserial.cpp src/dmmdecoder.cpp src/siprefix.cpp ${DECODER_FILES})
	target_include_directories(${TEST_HID} PRIVATE src)
	target_link_libraries(${TEST_HID} PRIVATE Qt::Core ${HIDAPI_TARGET})
	add_test(NAME hid_cable COMMAND ${TEST_HID})

	## RFC 2217 client against a fake server: negotiation, telnet filtering, IAC escaping
	set( TEST_RFC2217 test_rfc2217)
	add_executable(${TEST_RFC2217} MACOSX_BUNDLE tests/test_rfc2217.cpp src/portdevices/rfc2217serial.cpp src/dmmdecoder.cpp src/siprefix.cpp ${DECODER_FILES})
	target_include_directories(${TEST_RFC2217} PRIVATE src)
	target_link_libraries(${TEST_RFC2217} PRIVATE Qt::Core Qt::Network)
	add_test(NAME rfc2217_client COMMAND ${TEST_RFC2217})

	## generated documents must match their sources (device table from the
	## decoders, README from docs/)
	find_package(Python3 COMPONENTS Interpreter)
	if (Python3_Interpreter_FOUND)
		add_test(NAME docs_generated COMMAND ${Python3_EXECUTABLE} "${CMAKE_SOURCE_DIR}/tests/generate_docs.py" --check)
		## the network bridge (tools/qtdmm-bridge) has its own unittest suite, no pyserial needed
		if (Python3_VERSION VERSION_GREATER_EQUAL 3.11)
			add_test(NAME qtdmm_bridge COMMAND ${Python3_EXECUTABLE} -m unittest discover -s tests WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/tools/qtdmm-bridge")
		endif()
	endif()

	## the tests report through qWarning()/qInfo(); on Windows Qt sends those
	## to the debugger instead of stderr unless told otherwise, and ctest's
	## --output-on-failure would show nothing
	get_property(ALL_TESTS DIRECTORY PROPERTY TESTS)
	set_tests_properties(${ALL_TESTS} PROPERTIES ENVIRONMENT "QT_FORCE_STDERR_LOGGING=1;QT_LOGGING_TO_CONSOLE=1")
endif()

if (BUILD_TESTING)
	set( TEST_DECODER test_decoder)
	enable_testing()

	file(GLOB DECODER_FILES CONFIGURE_DEPENDS src/decoders/*.h  src/decoders/*.cpp )
	## the Victron decoder reads bit fields through src/victronble.cpp (which also carries the AES)
	list(APPEND DECODER_FILES src/victronble.cpp src/3rdparty/tiny-aes/aes.c)
	add_executable(${TEST_DECODER} MACOSX_BUNDLE tests/test_decoder.cpp src/dmmdecoder.cpp src/protocols.cpp src/siprefix.cpp ${DECODER_FILES})
	target_link_libraries(${TEST_DECODER} PRIVATE Qt::Core Qt::Test)
	add_test(NAME protocol_table COMMAND ${TEST_DECODER} --table)

	file(GLOB TEST_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/tests/data/decoder/*.json")
	foreach(test_file ${TEST_FILES})
		get_filename_component(name ${test_file} NAME_WE)
		add_test(NAME ${name} COMMAND ${TEST_DECODER} ${test_file})
	endforeach()

	set( TEST_GRAPH test_graph)
	add_executable(${TEST_GRAPH} MACOSX_BUNDLE tests/test_graph.cpp src/dmmgraph.cpp src/recordingfile.cpp src/settings.cpp src/siprefix.cpp src/engnumbervalidator.cpp)
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
	add_executable(${TEST_METER} MACOSX_BUNDLE tests/test_meter.cpp src/meterwid.cpp src/panelframe.cpp src/siprefix.cpp)
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

	## instances dialog: list from config files, delete mode, calculated instance
	set( TEST_INSTANCES test_instances)
	add_executable(${TEST_INSTANCES} MACOSX_BUNDLE tests/test_instances.cpp src/instancesdlg.cpp src/settings.cpp src/protocols.cpp src/dmmdecoder.cpp src/siprefix.cpp ${DECODER_FILES}
		src/sharedstatemanager.cpp src/calcexpr.cpp src/siprefix.cpp src/ui/uiinstancesdlg.ui)
	target_include_directories(${TEST_INSTANCES} PRIVATE src)
	target_link_libraries(${TEST_INSTANCES} PRIVATE Qt6::Widgets Qt::Core)
	add_test(NAME instances_dialog COMMAND ${TEST_INSTANCES})
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
		src/sharedstatemanager.cpp src/dmmdecoder.cpp src/protocols.cpp src/siprefix.cpp ${DECODER_FILES})
	target_include_directories(${TEST_CALC_DEVICE} PRIVATE src)
	target_link_libraries(${TEST_CALC_DEVICE} PRIVATE Qt::Core)
	add_test(NAME calc_device COMMAND ${TEST_CALC_DEVICE})

	## HID cable chips: report layouts and chip detection, no hardware needed
	set( TEST_HID test_hid)
	add_executable(${TEST_HID} MACOSX_BUNDLE tests/test_hid.cpp src/portdevices/hidserial.cpp src/dmmdecoder.cpp src/protocols.cpp src/siprefix.cpp ${DECODER_FILES})
	target_include_directories(${TEST_HID} PRIVATE src)
	target_link_libraries(${TEST_HID} PRIVATE Qt::Core ${HIDAPI_TARGET})
	add_test(NAME hid_cable COMMAND ${TEST_HID} "${CMAKE_SOURCE_DIR}/tests/data/hid_cables.json")

	## DMM connection state machine (Connecting/Connected/Timeout/Error/reconnect)
	## against a fake RFC 2217 server; needs the whole port stack
	set( TEST_DMM test_dmm)
	add_executable(${TEST_DMM} MACOSX_BUNDLE tests/test_dmm.cpp src/dmm.cpp src/readerthread.cpp src/porthandler.cpp
		src/portdevices/serial.cpp src/portdevices/hidserial.cpp src/portdevices/rfc2217serial.cpp src/portdevices/sigrok.cpp
		src/portdevices/calc.cpp src/calcexpr.cpp src/sharedstatemanager.cpp src/dmmdecoder.cpp src/protocols.cpp src/siprefix.cpp ${DECODER_FILES})
	target_include_directories(${TEST_DMM} PRIVATE src)
	target_link_libraries(${TEST_DMM} PRIVATE Qt6::Widgets Qt6::SerialPort Qt::Network Qt::Core ${HIDAPI_TARGET})
	add_test(NAME dmm_link_state COMMAND ${TEST_DMM})

	## RFC 2217 client against a fake server: negotiation, telnet filtering, IAC escaping
	set( TEST_RFC2217 test_rfc2217)
	add_executable(${TEST_RFC2217} MACOSX_BUNDLE tests/test_rfc2217.cpp src/portdevices/rfc2217serial.cpp src/dmmdecoder.cpp src/protocols.cpp src/siprefix.cpp ${DECODER_FILES})
	target_include_directories(${TEST_RFC2217} PRIVATE src)
	target_link_libraries(${TEST_RFC2217} PRIVATE Qt::Core Qt::Network)
	add_test(NAME rfc2217_client COMMAND ${TEST_RFC2217})

	## the recorder's CSV formats, without widgets
	set( TEST_RECORDING test_recording)
	add_executable(${TEST_RECORDING} MACOSX_BUNDLE tests/test_recording.cpp src/recordingfile.cpp src/siprefix.cpp)
	target_include_directories(${TEST_RECORDING} PRIVATE src)
	target_link_libraries(${TEST_RECORDING} PRIVATE Qt::Core)
	add_test(NAME recording_file COMMAND ${TEST_RECORDING} "${CMAKE_SOURCE_DIR}/tests/data/graph")

	## the readings table model, without its widget
	set( TEST_READINGLOG test_readinglog)
	add_executable(${TEST_READINGLOG} MACOSX_BUNDLE tests/test_readinglog.cpp src/readinglog.cpp src/siprefix.cpp)
	target_include_directories(${TEST_READINGLOG} PRIVATE src)
	target_link_libraries(${TEST_READINGLOG} PRIVATE Qt::Core Qt::Test)
	add_test(NAME reading_log COMMAND ${TEST_READINGLOG})

	## Victron Instant Readout: advertisement parsing, AES-CTR, the decoder
	set( TEST_VICTRON test_victronble)
	add_executable(${TEST_VICTRON} MACOSX_BUNDLE tests/test_victronble.cpp src/dmmdecoder.cpp src/protocols.cpp src/siprefix.cpp ${DECODER_FILES})
	target_include_directories(${TEST_VICTRON} PRIVATE src ${CMAKE_BINARY_DIR})
	target_link_libraries(${TEST_VICTRON} PRIVATE Qt::Core Qt::Test)
	add_test(NAME victron_instant_readout COMMAND ${TEST_VICTRON})

	## mDNS browsing for qtdmm-bridge announcements
	set( TEST_MDNS test_mdns)
	add_executable(${TEST_MDNS} MACOSX_BUNDLE tests/test_mdns.cpp src/mdnsbrowser.cpp)
	target_include_directories(${TEST_MDNS} PRIVATE src)
	target_link_libraries(${TEST_MDNS} PRIVATE Qt::Core Qt6::Network Qt::Test)
	add_test(NAME mdns_browse COMMAND ${TEST_MDNS} "${CMAKE_SOURCE_DIR}/tests/data/mdns" "${CMAKE_SOURCE_DIR}/tools/qtdmm-bridge")

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
	set_tests_properties(${ALL_TESTS} PROPERTIES ENVIRONMENT "QT_FORCE_STDERR_LOGGING=1;QT_LOGGING_TO_CONSOLE=1;PYTHONDONTWRITEBYTECODE=1")
endif()

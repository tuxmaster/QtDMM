# Version scheme YY.N[.P][-rcK] (cmake/git_version.cmake): what a tag, or a
# commit after it, turns into. Run: cmake -P tests/test_version.cmake
set(QTDMM_VERSION_TEST ON)
include("${CMAKE_CURRENT_LIST_DIR}/../cmake/git_version.cmake")

set(failed 0)
# describe  commit-YY  count  hash  -> display | package | numeric
function(check describe yy count hash display package numeric)
	qtdmm_parse_describe("${describe}" "${yy}" "${count}" "${hash}" V)
	set(num "${V_MAJOR}.${V_MINOR}.${V_PATCH}")
	if (NOT V_DISPLAY STREQUAL display OR NOT V_PACKAGE STREQUAL package OR NOT num STREQUAL numeric)
		message(SEND_ERROR "'${describe}' (${yy}): got ${V_DISPLAY} | ${V_PACKAGE} | ${num}, "
		                   "expected ${display} | ${package} | ${numeric}")
	endif()
endfunction()

# releases on their tag (describe --long and the short form of git archive)
check("26.1-0-gabc1234"       26 0 x "26.1"          "26.1"          "26.1.0")
check("26.1"                  26 0 x "26.1"          "26.1"          "26.1.0")
check("26.1.1-0-gabc1234"     26 0 x "26.1.1"        "26.1.1"        "26.1.1")
check("26.1-rc1-0-gabc1234"   26 0 x "26.1-rc1"      "26.1~rc1"      "26.1.0")
check("26.2-beta2"            26 0 x "26.2-beta2"    "26.2~beta2"    "26.2.0")
# after a release: the next one; a new year starts at .1
check("26.1-14-gabc1234"      26 0 x "26.2-dev.14+gabc1234"   "26.2~dev.14.gabc1234"   "26.2.0")
check("26.1.1-3-gabc1234"     26 0 x "26.2-dev.3+gabc1234"    "26.2~dev.3.gabc1234"    "26.2.0")
check("26.3-5-gabc1234"       27 0 x "27.1-dev.5+gabc1234"    "27.1~dev.5.gabc1234"    "27.1.0")
# after a release candidate: still heading for that release
check("26.1-rc1-3-gabc1234"   26 0 x "26.1-rc1.3+gabc1234"    "26.1~rc1.3.gabc1234"    "26.1.0")
# no version tag at all
check(""                      26 594 55ee3c3 "26.1-dev.594+g55ee3c3" "26.1~dev.594.g55ee3c3" "26.1.0")
# not a version tag
check("v1.0-2-gabc1234"       26 0 x "0.0-unknown"   "0.0~unknown"   "0.0.0")
check("26.0"                  26 0 x "0.0-unknown"   "0.0~unknown"   "0.0.0")

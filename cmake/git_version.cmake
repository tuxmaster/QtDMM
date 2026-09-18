# get version
#
# CalVer, derived from HEAD's commit date - not from git tags, not from the
# build date. This needs no tag management, is reproducible (rebuilding the
# same commit always yields the same version), and works on a shallow clone
# (unlike `git describe --tags`, which needs tag refs to be fetched).
#
# PROJECT_VERSION (MAJOR.MINOR.PATCH = YEAR.MONTH.DAY) is the clean form fed
# to CPack/RPM/Debian packaging, where a raw commit hash isn't valid syntax.
# PROJECT_VERSION_FULL additionally carries the short commit hash and is used
# for the APP_VERSION shown in the UI/--version, for build traceability.
function(get_version_from_git)
	set(GIT_COMMIT_SHORT_HASH "unknown")
	set(HAVE_GIT_DATE FALSE)

	find_package(Git QUIET)
	if(Git_FOUND)
		execute_process(
			COMMAND ${GIT_EXECUTABLE} log -1 --format=%cd --date=format:%Y.%m.%d
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			OUTPUT_VARIABLE GIT_COMMIT_DATE
			OUTPUT_STRIP_TRAILING_WHITESPACE
			RESULT_VARIABLE GIT_DATE_RESULT
		)

		if(GIT_DATE_RESULT EQUAL 0 AND GIT_COMMIT_DATE MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$")
			set(HAVE_GIT_DATE TRUE)
		else()
			message(WARNING "git log did not return a usable commit date, falling back to today's date")
		endif()

		execute_process(
			COMMAND ${GIT_EXECUTABLE} rev-parse --short=7 HEAD
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			OUTPUT_VARIABLE GIT_COMMIT_SHORT_HASH
			OUTPUT_STRIP_TRAILING_WHITESPACE
			RESULT_VARIABLE GIT_HASH_RESULT
		)
		if(NOT GIT_HASH_RESULT EQUAL 0)
			set(GIT_COMMIT_SHORT_HASH "unknown")
		endif()
	else()
		message(WARNING "Git not found, falling back to today's date for the version")
	endif()

	if(HAVE_GIT_DATE)
		set(VERSION_YEAR  "${CMAKE_MATCH_1}")
		set(VERSION_MONTH "${CMAKE_MATCH_2}")
		set(VERSION_DAY   "${CMAKE_MATCH_3}")
	else()
		# No git (e.g. a tarball build without .git) or the command failed:
		# CMake's own current-date timestamp needs no external tool at all.
		string(TIMESTAMP VERSION_YEAR  "%Y")
		string(TIMESTAMP VERSION_MONTH "%m")
		string(TIMESTAMP VERSION_DAY   "%d")
	endif()

	# strip leading zeros (CMake/CPack/RPM want plain integers, e.g. "9" not "09")
	string(REGEX REPLACE "^0+([0-9])" "\\1" VERSION_MONTH "${VERSION_MONTH}")
	string(REGEX REPLACE "^0+([0-9])" "\\1" VERSION_DAY   "${VERSION_DAY}")

	set(PROJECT_VERSION_MAJOR ${VERSION_YEAR}  PARENT_SCOPE)
	set(PROJECT_VERSION_MINOR ${VERSION_MONTH} PARENT_SCOPE)
	set(PROJECT_VERSION_PATCH ${VERSION_DAY}   PARENT_SCOPE)

	set(CLEAN_VERSION "${VERSION_YEAR}.${VERSION_MONTH}.${VERSION_DAY}")
	set(PROJECT_VERSION "${CLEAN_VERSION}" PARENT_SCOPE)
	set(PROJECT_VERSION_FULL "${CLEAN_VERSION}+g${GIT_COMMIT_SHORT_HASH}" PARENT_SCOPE)

	message(STATUS "QtDMM version: ${CLEAN_VERSION} (${CLEAN_VERSION}+g${GIT_COMMIT_SHORT_HASH})")
endfunction()

get_version_from_git()

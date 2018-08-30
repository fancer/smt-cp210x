# FindLibUUID.cmake
# Once done this will define
#
#  LIBUUID_FOUND - System has libuuid
#  LIBUUID_INCLUDE_DIR - The libuuid include directory
#  LIBUUID_LIBRARY - The libraries needed to use libuuid
#  LIBUUID_DEFINITIONS - Compiler switches required for using libuuid

# FreeBSD
if (${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
	find_path(LIBUUID_INCLUDE_DIR NAMES uuid.h HINTS "/usr/include" PATH_SUFFIXES uuid)
else ()
	find_path(LIBUUID_INCLUDE_DIR NAMES uuid.h HINTS "/usr;/usr/local;/opt" PATH_SUFFIXES uuid)
endif()

set(LIBUUID_NAMES "uuid;libuuid")

if (MSYS OR MINGW OR MSVC)
	find_library(LIBUUID_LIBRARY NAMES ${LIBUUID_NAMES})
else()
	find_library(LIBUUID_LIBRARY NAMES ${LIBUUID_NAMES} HINTS "/usr;/usr/local;/opt")
endif ()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Libuuid DEFAULT_MSG LIBUUID_LIBRARY LIBUUID_INCLUDE_DIR)

mark_as_advanced(LIBUUID_INCLUDE_DIR LIBUUID_LIBRARY)

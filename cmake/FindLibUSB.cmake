# FindLibUSB.cmake
# Once done this will define
#
#  LIBUSB_FOUND - System has libusb
#  LIBUSB_INCLUDE_DIR - The libusb include directory
#  LIBUSB_LIBRARY - The libraries needed to use libusb
#  LIBUSB_DEFINITIONS - Compiler switches required for using libusb

# FreeBSD
if (${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
	find_path(LIBUSB_INCLUDE_DIR NAMES libusb.h HINTS "/usr/include")
else ()
	find_path(LIBUSB_INCLUDE_DIR NAMES libusb.h HINTS "/usr;/usr/local;/opt" PATH_SUFFIXES libusb-1.0)
endif()

if (APPLE)
	set(LIBUSB_NAME libusb-1.0.a)
elseif(MSYS OR MINGW)
	set(LIBUSB_NAME usb-1.0)
elseif(MSVC)
	set(LIBUSB_NAME libusb-1.0.lib)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
	set(LIBUSB_NAME usb)
else()
	set(LIBUSB_NAME usb-1.0)
endif()

if (MSYS OR MINGW OR MSVC)
	find_library(LIBUSB_LIBRARY NAMES ${LIBUSB_NAME})
else()
	find_library(LIBUSB_LIBRARY NAMES ${LIBUSB_NAME} HINTS "/usr;/usr/local;/opt")
endif ()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Libusb DEFAULT_MSG LIBUSB_LIBRARY LIBUSB_INCLUDE_DIR)

mark_as_advanced(LIBUSB_INCLUDE_DIR LIBUSB_LIBRARY)

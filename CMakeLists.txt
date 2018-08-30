cmake_minimum_required(VERSION 3.1.0)
project(smt-cp210x
	VERSION "6.7.5"
	DESCRIPTION "Standalone management tools for SiliconLabs cp210x serial controllers"
	LANGUAGES "CXX")

# Common build cleanups
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# Include cmake modules so to have LibUSB/LibUUID searched
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/cmake")
find_package(LibUSB REQUIRED)
include_directories(${LIBUSB_INCLUDE_DIR})
find_package(LibUUID REQUIRED)
include_directories(${LIBUUID_INCLUDE_DIR})

# These are common header files used across the build process
include_directories(BEFORE "common/include")

# List of libcp210x source files
file(GLOB LIBCP210X_SOURCES "lib/src/*.cpp")
file(GLOB LIBCP210X_PRIVATE_HEADERS "lib/src/*.h")
file(GLOB LIBCP210X_PUBLIC_HEADERS "lib/include/*.h")

# List of smt-cp210x utility source files
file(GLOB SMTCP210X_SOURCES "smt/src/*.cpp")
file(GLOB SMTCP210X_PRIVATE_HEADERS "smt/src/*.h;smt/src/utf8/*.h")
file(GLOB SMTCP210X_CONFIGS "configs/*.configuration")
file(GLOB SMTCP210X_UDEV_RULES "udev/rules.d/*.rules")

# Include OsDep utility to smt-cp210x in case of UNIX-like OS
if(UNIX)
	list(APPEND SMTCP210X_SOURCES "common/unix/OsDep.cpp")
endif()

# Build libcp210x library
add_library(cp210x SHARED ${LIBCP210X_SOURCES} ${LIBCP210X_PRIVATE_HEADERS})
target_include_directories(cp210x BEFORE PUBLIC "lib/include")
target_link_libraries(cp210x PUBLIC ${LIBUSB_LIBRARY})
set_target_properties(cp210x
		      PROPERTIES
		        VERSION "${PROJECT_VERSION}"
		        SOVERSION "${PROJECT_VERSION_MAJOR}"
		        PUBLIC_HEADER "${LIBCP210X_PUBLIC_HEADERS}")

# Build smt-cp210x binary
add_executable(smt-cp210x ${SMTCP210X_SOURCES} ${SMTCP210X_PRIVATE_HEADERS})
set_target_properties(smt-cp210x
		      PROPERTIES
		        PRIVATE_HEADER "${SMTCP210X_PRIVATE_HEADERS}")
target_link_libraries(smt-cp210x PUBLIC cp210x ${LIBUUID_LIBRARY})

# libcp210x installation rules
install(TARGETS cp210x 
	LIBRARY
	  DESTINATION lib
	PUBLIC_HEADER
	  DESTINATION include)

# smt-cp210x installation rules
install(TARGETS smt-cp210x DESTINATION bin)
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
	install(FILES ${SMTCP210X_CONFIGS} DESTINATION "/etc/smt/cp210x")
	install(FILES ${SMTCP210X_UDEV_RULES} DESTINATION "/etc/udev/rules.d")
endif()

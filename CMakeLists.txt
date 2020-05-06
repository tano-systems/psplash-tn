cmake_minimum_required(VERSION 2.8.10)

INCLUDE(CheckLibraryExists)
INCLUDE(CheckFunctionExists)

SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

PROJECT(psplash C)

ADD_DEFINITIONS(-Wall -Wextra)
ADD_DEFINITIONS(-D_GNU_SOURCE)

SET(PSPLASH_SOURCES
	psplash.c
	psplash-console.c
	psplash-fb.c
	psplash-uci.c
)

SET(PSPLASH_WRITE_SOURCES
	psplash-write.c
)

FIND_PATH(uci_include_dir uci.h)
INCLUDE_DIRECTORIES(${uci_include_dir})

FIND_PATH(ubox_include_dir libubox/ulog.h)
INCLUDE_DIRECTORIES(${ubox_include_dir})

ADD_EXECUTABLE(psplash ${PSPLASH_SOURCES})
SET_TARGET_PROPERTIES(psplash PROPERTIES OUTPUT_NAME psplash)
TARGET_LINK_LIBRARIES(psplash uci ubox)

ADD_EXECUTABLE(psplash-write ${PSPLASH_WRITE_SOURCES})
SET_TARGET_PROPERTIES(psplash-write PROPERTIES OUTPUT_NAME psplash-write)

INSTALL(TARGETS psplash psplash-write
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin
)

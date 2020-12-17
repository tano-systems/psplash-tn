cmake_minimum_required(VERSION 2.8.10)

INCLUDE(CheckLibraryExists)
INCLUDE(CheckFunctionExists)

OPTION(ENABLE_PNG "Enable support for PNG loading" ON)
OPTION(ENABLE_JPEG "Enable support for JPEG loading" OFF)
OPTION(ENABLE_ALIVE_GIF "Enable alive GIF animation support" OFF)

SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

PROJECT(psplash C)

ADD_DEFINITIONS(-Wall -Wextra)
ADD_DEFINITIONS(-D_GNU_SOURCE)

SET(LIBS uci ubox)

SET(PSPLASH_SOURCES
	psplash.c
	psplash-console.c
	psplash-fb.c
	psplash-uci.c
	psplash-image.c
)

SET(PSPLASH_WRITE_SOURCES
	psplash-write.c
)

IF(ENABLE_ALIVE_GIF)
	SET(PSPLASH_SOURCES ${PSPLASH_SOURCES}
		psplash-alive.c
		3rdparty/gifdec/gifdec.c
	)
	INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/gifdec)
	ADD_DEFINITIONS(-DENABLE_ALIVE_GIF)
ENDIF()

IF(ENABLE_PNG)
	FIND_LIBRARY(png NAMES png)
	FIND_PATH(png_include_dir png.h)
	INCLUDE_DIRECTORIES(${png_include_dir})
	SET(LIBS ${LIBS} png)
	ADD_DEFINITIONS(-DENABLE_PNG)
ENDIF()

IF(ENABLE_JPEG)
	FIND_LIBRARY(jpeg NAMES jpeg)
	FIND_PATH(jpeg_include_dir jpeglib.h)
	INCLUDE_DIRECTORIES(${jpeg_include_dir})
	SET(LIBS ${LIBS} jpeg)
	ADD_DEFINITIONS(-DENABLE_JPEG)
ENDIF()

FIND_LIBRARY(uci NAMES uci)
FIND_PATH(uci_include_dir uci.h)
INCLUDE_DIRECTORIES(${uci_include_dir})

FIND_LIBRARY(ubox NAMES ubox)
FIND_PATH(ubox_include_dir libubox/ulog.h)
INCLUDE_DIRECTORIES(${ubox_include_dir})

ADD_EXECUTABLE(psplash ${PSPLASH_SOURCES})
SET_TARGET_PROPERTIES(psplash PROPERTIES OUTPUT_NAME psplash)
TARGET_LINK_LIBRARIES(psplash ${LIBS})

ADD_EXECUTABLE(psplash-write ${PSPLASH_WRITE_SOURCES})
SET_TARGET_PROPERTIES(psplash-write PROPERTIES OUTPUT_NAME psplash-write)

INSTALL(TARGETS psplash psplash-write
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin
)

cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(node_fontinfo)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
set(freetype2_INSTALL ${CMAKE_BINARY_DIR}/freetype2)

SET(NODEJS_URL "https://atom.io/download/atom-shell" CACHE STRING "Node.JS URL")
SET(NODEJS_NAME "iojs" CACHE STRING "Node.JS Name")
SET(NODEJS_VERSION "v9.3.1" CACHE STRING "Node.JS Version")

include(NodeJS)
include(ExternalProject)

nodejs_init()

ExternalProject_Add(
	freetype2
	SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/freetype2
	URL "https://download-mirror.savannah.gnu.org/releases/freetype/freetype-2.9.tar.gz"
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${freetype2_INSTALL}
)

set(freetype2_INCLUDE_DIRS ${freetype2_INSTALL}/include/freetype2)
IF(WIN32)
set(freetype2_DEBUG_LIBRARY
	${freetype2_INSTALL}/lib/freetyped${CMAKE_STATIC_LIBRARY_SUFFIX})

set(freetype2_RELEASE_LIBRARY
	${freetype2_INSTALL}/lib/freetype${CMAKE_STATIC_LIBRARY_SUFFIX})
ELSEIF(APPLE)
set(freetype2_DEBUG_LIBRARY
	${freetype2_INSTALL}/lib/libfreetype${CMAKE_STATIC_LIBRARY_SUFFIX})
set(freetype2_RELEASE_LIBRARY
	${freetype2_INSTALL}/lib/libfreetype${CMAKE_STATIC_LIBRARY_SUFFIX})
ENDIF()

set(fontinfo_SOURCES
	src/fontinfo/fontinfo.c)

set(fontinfo_HEADERS
	src/fontinfo/fontinfo.h
	src/fontinfo/endian.h)

add_library(fontinfo STATIC ${fontinfo_SOURCES})

target_include_directories(fontinfo PUBLIC ${freetype2_INCLUDE_DIRS})

target_link_libraries(fontinfo
	optimized ${freetype2_RELEASE_LIBRARY}
	debug ${freetype2_DEBUG_LIBRARY})

add_dependencies(fontinfo freetype2)

# Include N-API wrappers
execute_process(COMMAND node -p "require('node-addon-api').include"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE NODE_ADDON_API_DIR
        )
string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})

# Define NAPI_VERSION
add_definitions(-DNAPI_VERSION=4)

add_nodejs_module(node_fontinfo "src/module.cpp")

if (WIN32)
	target_link_libraries(node_fontinfo fontinfo ${NODEJS_LIBRARIES})
else ()
	target_link_libraries(node_fontinfo fontinfo ${NODEJS_LIBRARIES} ${freetype2_DEBUG_LIBRARY})
endif ()

target_include_directories(node_fontinfo PUBLIC ${NODEJS_INCLUDE_DIRS} ${NODE_ADDON_API_DIR})

if (WIN32)
	target_compile_definitions(node_fontinfo PRIVATE -DUNICODE -D_UNICODE)
endif ()

if(FONTINFO_BUILD_TESTS)
	add_executable(simple_test "test/simple_test.c")
	target_link_libraries(simple_test fontinfo)
	target_include_directories(simple_test PUBLIC "src")
endif()

IF(WIN32)
	install(FILES $<TARGET_PDB_FILE:node_fontinfo> DESTINATION ${CMAKE_INSTALL_PREFIX} OPTIONAL)
ENDIF()

install(FILES $<TARGET_FILE:node_fontinfo> DESTINATION ${CMAKE_INSTALL_PREFIX})

install(
	FILES
		${node_fontinfo_SOURCE_DIR}/package.json
		${node_fontinfo_SOURCE_DIR}/main.js
	DESTINATION
		${CMAKE_INSTALL_PREFIX})

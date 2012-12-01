project(vaporpp)
cmake_minimum_required(VERSION 2.8)

#we need C++11:
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )


#Get as many warnings as possible:
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	message("using g++")
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic" )
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	message("using clang++")
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -Wno-c++98-compat" )
else()
	message("unknown compiler")
endif()

if(CMAKE_BUILD_TYPE MATCHES "Debug")
	message("creating Debug-Version")
	add_definitions( -DDEBUG )
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g" )
elseif(CMAKE_BUILD_TYPE MATCHES "Release")
	message("creating Release-Version")
	add_definitions( -DNDEBUG )
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror" )
	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3"  )
	elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O4"  )
	endif()
endif()

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

find_package(Boost COMPONENTS REQUIRED system thread program_options)
find_package(Threads)

#find_package(readline)
#include_directories(${CMAKE_SOURCE_DIR}/src/lib)

add_subdirectory(src)

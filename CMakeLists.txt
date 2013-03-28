cmake_minimum_required(VERSION 2.6)
project(collins) 

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL Clang)
	message(FATAL_ERROR "Clang required!")
endif()

if(NOT CMAKE_BUILD_TYPE)
	message(STATUS "Build type: RelWithDebInfo (default)")
	set(CMAKE_BUILD_TYPE "RelWithDebInfo")
else()
	message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")

#set(Boost_USE_STATIC_LIBS        ON)
#set(Boost_USE_MULTITHREADED      ON)
#set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost REQUIRED COMPONENTS filesystem system program_options)

if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	#message(STATUS ${Boost_LIBRARIES})
endif()

# enable C++11 and libc++
set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")
#set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++ -O2 -g")
#set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++ -O2 -g -arch i386 -arch x86_64")

# find sparsehash headers
find_file(SPARSEHASH_HEADER sparsehash/sparse_hash_map)
find_file(DENSEHASH_HEADER sparsehash/dense_hash_map)

# select default map container
if(NOT DEFINED USE_MAP)
	if(DENSEHASH_HEADER)
		set(USE_MAP 1)
	else()
		set(USE_MAP 0)
	endif()
endif()

# map container info
if(USE_MAP EQUAL -1)
	message(STATUS "Will use std::map")
elseif(USE_MAP EQUAL 0) # default
	message(STATUS "Will use std::unordered_map")
elseif(USE_MAP EQUAL 1 AND DENSEHASH_HEADER)
	message(STATUS "Will use google::dense_hash_map")
elseif(USE_MAP EQUAL 2 AND SPARSEHASH_HEADER)
	message(STATUS "Will use google::sparse_hash_map")
else()
	if(USE_MAP EQUAL 1 AND NOT DENSEHASH_HEADER)
		message(STATUS "Headers for google::dense_hash_map not available, will use default std::unordered_map")
	elseif(USE_MAP EQUAL 2 AND NOT SPARSEHASH_HEADER)
		message(STATUS "Headers for google::sparse_hash_map not available, will use default std::unordered_map")
	else()
		message(STATUS "Unknown map container selection, will use default std::unordered_map")
	endif()
	set(USE_MAP 0)
endif()

add_definitions(-DUSE_MAP=${USE_MAP})


add_executable(collins main.cpp train.cpp features.cpp collins0.cpp utils.cpp)

target_link_libraries(collins ${Boost_LIBRARIES})

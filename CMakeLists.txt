cmake_minimum_required(VERSION 2.6)
project(collins) 

#set(Boost_USE_STATIC_LIBS        ON)
#set(Boost_USE_MULTITHREADED      ON)
#set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost REQUIRED COMPONENTS filesystem system program_options)

if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	#message(STATUS ${Boost_LIBRARIES})
endif()

# TODO: pārbaudīt vai ir clang, vai g++
set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++ -O2 -g")
#set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++ -O2 -g -arch i386 -arch x86_64")

#add_executable(collins main.cpp collins0.cpp train.cpp utils.cpp features.cpp MurmurHash3.cpp)
add_executable(collins main.cpp train.cpp features.cpp collins0.cpp utils.cpp)

target_link_libraries(collins ${Boost_LIBRARIES})

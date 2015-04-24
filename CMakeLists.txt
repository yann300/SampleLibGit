# Date: 2015
# Author: Ali Mashatan
# Email : ali.mashatan@gmail.com

cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

project(SampleLibGit)

find_package(Qt5Core REQUIRED )
find_package(Qt5Test REQUIRED )

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(SampleLibGit WIN32 test/main.cpp test/GitClientTest.h  src/Repository.cpp src/Repository.h)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/libgit/inc")
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/src")
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/test")

target_link_libraries(SampleLibGit Qt5::Test)

IF(WIN32) # Check if we are on Windows
	if ("${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
		target_link_libraries(SampleLibGit "${CMAKE_CURRENT_SOURCE_DIR}/libgit/lib/relase/git2.lib")
	else()
		target_link_libraries(SampleLibGit "${CMAKE_CURRENT_SOURCE_DIR}/libgit/lib/debug/git2.lib")
	endif()
	if(MSVC) # Check if we are using the Visual Studio compiler
		set_target_properties(SampleLibGit PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
		set_target_properties(SampleLibGit PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
	endif()
endif()

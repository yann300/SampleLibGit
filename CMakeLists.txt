# Date: 2015
# Author: Ali Mashatan
# Email : ali.mashatan@gmail.com

cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

project(SampleLibGit)

find_package(Qt5Core REQUIRED )
find_package(Qt5 COMPONENTS Widgets Test Quick Qml Declarative)

set(LIBRARIES "Qt5::Core")

#set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(QT_USE_QTDECLARATIVE TRUE)

qt5_add_resources(UI_RESOURCES Resource.qrc)

add_executable(SampleLibGit
		WIN32 sample/main.cpp
		test/GitClientTest.h
		src/StatusFile.h
		src/Repository.cpp
		src/Repository.h src/RepositoryProxy.cpp
		src/RepositoryProxy.h
		${UI_RESOURCES})

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/libgit/inc")
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/src")
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/test")


list(APPEND LIBRARIES ${Qt5Widgets_LIBRARIES})
list(APPEND LIBRARIES "Qt5::Gui")
list(APPEND LIBRARIES "Qt5::Widgets")
list(APPEND LIBRARIES "Qt5::Quick")
list(APPEND LIBRARIES "Qt5::Qml")
list(APPEND LIBRARIES "Qt5::Test")
target_link_libraries(SampleLibGit ${LIBRARIES})


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

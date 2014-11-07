cmake_minimum_required (VERSION 2.6)
project (XMLTest)
# The version number.
set (XMLTest_VERSION_MAJOR 1)
set (XMLTest_VERSION_MINOR 0)
  
include_directories("${PROJECT_BINARY_DIR}")

include_directories ("${PROJECT_SOURCE_DIR}/include")
add_subdirectory(include) 

# add the executable
add_executable(XMLTest 
	xml_test.cpp 
	${PROJECT_SOURCE_DIR}/source/xml.cpp
)

SET( CMAKE_CXX_FLAGS "-std=c++11 -fpermissive" )

install (TARGETS XMLTest DESTINATION "${PROJECT_SOURCE_DIR}")
# setup project metadata
cmake_minimum_required(VERSION 3.0)
project(corluma_tests)

# create a lib and bin in the build directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# include Catch
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE 
			   ${CMAKE_CURRENT_SOURCE_DIR}/catch
			   ${CMAKE_CURRENT_SOURCE_DIR}/../src/cor)


set(TEST_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_Dictionary.cpp
)

add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests Catch)



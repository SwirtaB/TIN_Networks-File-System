cmake_minimum_required(VERSION 3.16.0)
project(TIN_NFS VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB LIB_SRC "./src/*.cpp" )
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)
# Output directory for shared library target
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/lib)
# Output directory for static library target
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/lib)
set(CMAKE_EXPORT_COMPILE_COMMANDS)

include(CTest)
enable_testing()

set(LIB_NAME "tinnfslib")
add_library(${LIB_NAME} SHARED ${LIB_SRC})

target_include_directories(${LIB_NAME} PRIVATE ${CMAKE_HOME_DIRECTORY}/include)
if(CMAKE_BUILD_TYPE EQUAL "DEBUG")
    target_compile_options(${LIB_NAME} PRIVATE -Wall -Wextra -pedantic -O0)
else()
    target_compile_options(${LIB_NAME} PRIVATE -Ofast)
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

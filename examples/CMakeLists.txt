cmake_minimum_required(VERSION 3.16.0)
project(TIN_NFS VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_library(TINNFS tinnfs
        HINTS ${CMAKE_HOME_DIRECTORY}/lib/
        REQUIRED
)

# Add src folder for example and set example target name
file(GLOB USER_APP_SRC "./user_app/*.cpp" )
set(USER_APP_EXAMPLE user_app)

file(GLOB SEND_MESSAGE_SRC "./send_message/*.cpp" )
set(SEND_MESSAGE_EXAMPLE send_message)

file(GLOB NFS_SERVER_SRC "./nfs_server/*.cpp")
set(NFS_SERVER_EXAMPLE nfs_server)

file(GLOB COPY_FILE_TO_SERVER_SRC "./copy_file_to_server/*.cpp")
set(COPY_FILE_TO_SERVER_EXAMPLE copy_file_to_server)

# Add include folders
include_directories(${CMAKE_HOME_DIRECTORY}/include)

add_executable(${USER_APP_EXAMPLE} ${USER_APP_SRC})
target_link_libraries(${USER_APP_EXAMPLE} PRIVATE ${TINNFS})

add_executable(${SEND_MESSAGE_EXAMPLE} ${SEND_MESSAGE_SRC})
target_link_libraries(${SEND_MESSAGE_EXAMPLE} PRIVATE ${TINNFS})

add_executable(${NFS_SERVER_EXAMPLE} ${NFS_SERVER_SRC})
target_link_libraries(${NFS_SERVER_EXAMPLE} PRIVATE ${TINNFS})

add_executable(${COPY_FILE_TO_SERVER_EXAMPLE} ${COPY_FILE_TO_SERVER_SRC})
target_link_libraries(${COPY_FILE_TO_SERVER_EXAMPLE} PRIVATE ${TINNFS})

if(CMAKE_BUILD_TYPE EQUAL "DEBUG")
  target_compile_options(${LIB_NAME} PRIVATE -Wall -Wextra -pedantic -O0)
else()
  target_compile_options(${LIB_NAME} PRIVATE -O3)
endif()
cmake_minimum_required(VERSION 3.20)
project(FlexFlow LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (UNIX)
    add_compile_options(-Wall -Werror -Wextra -fvisibility=hidden)

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        add_compile_options(-Weffc++)
    endif ()
else () # Windows
    if (MINGW)
        add_compile_options(-Wall -Werror -Wextra -fvisibility=hidden)

        if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
            add_compile_options(-Weffc++)
        endif ()
    else () # msvc or icc
        add_compile_options(/W4 /utf-8)
    endif (MINGW)
endif (UNIX) # end if (UNIX)

# options
option(ENABLE_SERVER "enable server" on)
option(ENABLE_CAPI "enable model CAPI for ffi" on)
option(ENABLE_TEST "Enable model unit test" on)

if((NOT ENABLE_SERVER) AND (NOT ENABLE_CAPI))
    set(ENABLE_SERVER on)
endif((NOT ENABLE_SERVER) AND (NOT ENABLE_CAPI))

if(ENABLE_TEST)
    enable_testing()
    set(ENABLE_SERVER on)
endif(ENABLE_TEST)

# variables
set(FF_NAME FlexFlow)

if((CLIENT_TIMEOUT GREATER 61) OR (CLIENT_TIMEOUT LESS 1))
    set(CLIENT_TIMEOUT 31)
endif((CLIENT_TIMEOUT GREATER 61) OR (CLIENT_TIMEOUT LESS 1))

set(PROJECT_ROOT_DIR ${CMAKE_SOURCE_DIR})
if (WIN32)
    string(REPLACE "/" "\\\\" PROJECT_ROOT_DIR ${CMAKE_SOURCE_DIR})
ENDIF (WIN32)

configure_file(config.h.in config.h @ONLY)
include_directories(After SYSTEM
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/protos
)
include(GNUInstallDirs)

find_package(Protobuf REQUIRED)
find_library (UPB_LIBRARIES NAMES upb)
if (UPB_LIBRARIES)
    add_library(protobuf::libupb STATIC IMPORTED)
    add_executable(protobuf::protoc-gen-upb IMPORTED)
    add_executable(protobuf::protoc-gen-upbdefs IMPORTED)
    add_executable(protobuf::protoc-gen-upb_minitable IMPORTED)
endif()

find_package(gRPC REQUIRED)
find_package(cxxopts REQUIRED)
find_package(spdlog REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(GTest REQUIRED)

file(GLOB_RECURSE PROTO_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/protos/*.cc")
file(GLOB_RECURSE PROTO_HDRS "${CMAKE_CURRENT_SOURCE_DIR}/protos/*.h")

add_library(grpc_common STATIC
    ${PROTO_HDRS}
    ${PROTO_SRCS}
)

target_link_libraries(grpc_common
    PRIVATE
    protobuf::libprotobuf
    gRPC::grpc++
)

set_property(TARGET grpc_common PROPERTY POSITION_INDEPENDENT_CODE ON)

include(cmake/ffmodel.cmake)
include(cmake/ffmodel_ut.cmake)
include(cmake/flexflowserver.cmake)

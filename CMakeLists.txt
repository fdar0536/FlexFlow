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

    add_link_options(-Wl,--unresolved-symbols=report-all)
else () # Windows
    if (MINGW)
        add_compile_options(-Wall -Werror -Wextra -fvisibility=hidden)

        if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
            add_compile_options(-Weffc++)
        endif ()

        add_link_options(-Wl,--unresolved-symbols=report-all)
    else () # msvc or icc
        add_compile_options(/W4 /utf-8)
    endif (MINGW)
endif (UNIX) # end if (UNIX)

# options
option(ENABLE_SERVER "enable server" on)
option(ENABLE_C_MODEL "enable C model for ffi" on)

# variables
set(FF_VERSION r12)
set(FF_NAME FlexFlow)

if(NOT DEFINED CLIENT_TIMEOUT)
    set(CLIENT_TIMEOUT 31)
endif(NOT DEFINED CLIENT_TIMEOUT)

if((CLIENT_TIMEOUT GREATER 61) OR (CLIENT_TIMEOUT LESS 1))
    set(CLIENT_TIMEOUT 31)
endif((CLIENT_TIMEOUT GREATER 61) OR (CLIENT_TIMEOUT LESS 1))

include(cmake/getGitInfo.cmake)

set(FF_COMMIT "")
get_git_hash(FF_COMMIT)
message(STATUS "Git hash is ${FF_COMMIT}")

# git_BRANCH
set(FF_BRANCH "")
get_git_branch(FF_BRANCH)
message(STATUS "Git branch is ${FF_BRANCH}")

if(NOT DEFINED READ_BUFFER_SIZE)
    set(READ_BUFFER_SIZE 4096)
endif(NOT DEFINED READ_BUFFER_SIZE)

if(NOT DEFINED MAX_READ_QUEUE_SIZE)
    set(MAX_READ_QUEUE_SIZE 1024)
endif(NOT DEFINED MAX_READ_QUEUE_SIZE)

configure_file(config.h.in config.h @ONLY)
include_directories(After SYSTEM ${CMAKE_CURRENT_BINARY_DIR})
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

set(PROTOC_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/protos)
include(cmake/protoc.cmake)

set(PROTOS
    ${CMAKE_CURRENT_SOURCE_DIR}/protos/access.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/protos/queue.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/protos/queuelist.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/protos/types.proto
)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_BINARY_DIR} ${PROTOS})
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${CMAKE_CURRENT_BINARY_DIR} ${PROTOS})

add_library(grpc_common STATIC
    ${PROTO_HDRS}
    ${PROTO_SRCS}
    ${GRPC_HDRS}
    ${GRPC_SRCS}
)

target_link_libraries(grpc_common
    PRIVATE
    protobuf::libprotobuf
    gRPC::grpc++
)

include(cmake/ffmodel.cmake)
include(cmake/flexflowserver.cmake)

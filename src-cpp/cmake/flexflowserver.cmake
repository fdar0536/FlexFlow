if(ENABLE_SERVER)
    set(FF_SERVER_LIBS
        protobuf::libprotobuf
        gRPC::grpc++
        cxxopts::cxxopts

        grpc_common
        spdlog::spdlog
        SQLite::SQLite3
        yaml-cpp::yaml-cpp
    )

    set(SERVER_CONTROLLER_SRC

        #config
        controller/grpcserver/config.cpp
        controller/grpcserver/config.hpp
        controller/grpcserver/init.cpp
        controller/grpcserver/init.hpp

        # grpc
        controller/grpcserver/accessimpl.cpp
        controller/grpcserver/accessimpl.hpp
        controller/grpcserver/queueimpl.cpp
        controller/grpcserver/queueimpl.hpp
        controller/grpcserver/queuelistimpl.cpp
        controller/grpcserver/queuelistimpl.hpp
        controller/grpcserver/server.cpp
        controller/grpcserver/server.hpp
    )

    add_executable(FlexFlowServer
        ${SERVER_CONTROLLER_SRC}

        servermain.cpp)

    add_dependencies(FlexFlowServer grpc_common ffmodel)

    target_link_libraries(FlexFlowServer
        PRIVATE

        ${FF_SERVER_LIBS}
        ffmodel
    )

    if (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
        target_link_options(FlexFlowServer PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
        target_compile_options(FlexFlowServer PRIVATE $<$<CONFIG:Debug>:/ZI>)
    endif()
endif(ENABLE_SERVER)

if(ENABLE_CLI)
    set(FF_CLI_LIBS
        protobuf::libprotobuf
        gRPC::grpc++
        CLI11::CLI11
        spdlog::spdlog
        yaml-cpp::yaml-cpp

        grpc_common
    )

    set(CLI_SRC
        climain.cpp

        controller/cli/connect.cpp
        controller/cli/connect.hpp
        controller/cli/queue.cpp
        controller/cli/queue.hpp
        controller/cli/queuelist.cpp
        controller/cli/queuelist.hpp
    )
    

    add_executable(FlexFlowCLI
        ${CLI_SRC}
    )

    add_dependencies(FlexFlowCLI grpc_common ffmodel)

    target_link_libraries(FlexFlowCLI
        PRIVATE

        ${FF_CLI_LIBS}
        ffmodel
    )

    if (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
        target_link_options(FlexFlowCLI PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
        target_compile_options(FlexFlowCLI PRIVATE $<$<CONFIG:Debug>:/ZI>)
    endif()
endif(ENABLE_CLI)
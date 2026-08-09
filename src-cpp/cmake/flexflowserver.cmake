if(ENABLE_SERVER)
    set(FF_SERVER_LIBS
        protobuf::libprotobuf
        gRPC::grpc++
        CLI11::CLI11
        spdlog::spdlog
        SQLite::SQLite3
        yaml-cpp::yaml-cpp

        OpenSSL::Crypto
        OpenSSL::SSL

        grpc_common
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
        controller/grpcserver/utils.cpp
        controller/grpcserver/utils.hpp
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

    install(TARGETS FlexFlowServer DESTINATION .)
    install(FILES server-config.example.yaml DESTINATION .)

    if (NOT ("${BUNDLE_LIB_SEARCH_DIR}" STREQUAL ""))
        install(CODE "
            include(BundleUtilities)

            function(gp_item_default_embedded_path_override item default_embedded_path_var)
                set(\${default_embedded_path_var} \"@executable_path\" PARENT_SCOPE)
            endfunction()

            set(SEARCH_DIRS \"\")
            list(APPEND SEARCH_DIRS \"${BUNDLE_LIB_SEARCH_DIR}\")
            list(APPEND SEARCH_DIRS \"${CMAKE_BINARY_DIR}\")

            file(TO_CMAKE_PATH \"\$ENV{PATH}\" PATH_LIST)
            list(APPEND SEARCH_DIRS \${PATH_LIST})

            set(EXE_PATH \"\${CMAKE_INSTALL_PREFIX}/FlexFlowServer${CMAKE_EXECUTABLE_SUFFIX}\")

            fixup_bundle(
                \"\${EXE_PATH}\"
                \"\"
                \"\${SEARCH_DIRS}\"
            )
        " COMPONENT Runtime)
    endif (NOT ("${BUNDLE_LIB_SEARCH_DIR}" STREQUAL ""))

    if (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
        target_link_options(FlexFlowServer PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
        target_compile_options(FlexFlowServer PRIVATE $<$<CONFIG:Debug>:/ZI>)
    endif()
endif(ENABLE_SERVER)

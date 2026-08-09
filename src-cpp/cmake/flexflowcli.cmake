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

    install(TARGETS FlexFlowCLI DESTINATION .)
    install(FILES client-profile.example.yaml DESTINATION .)

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

            set(EXE_PATH \"\${CMAKE_INSTALL_PREFIX}/FlexFlowCLI${CMAKE_EXECUTABLE_SUFFIX}\")

            fixup_bundle(
                \"\${EXE_PATH}\"
                \"\"
                \"\${SEARCH_DIRS}\"
            )
        " COMPONENT Runtime)
    endif (NOT ("${BUNDLE_LIB_SEARCH_DIR}" STREQUAL ""))

    if (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
        target_link_options(FlexFlowCLI PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
        target_compile_options(FlexFlowCLI PRIVATE $<$<CONFIG:Debug>:/ZI>)
    endif()
endif(ENABLE_CLI)
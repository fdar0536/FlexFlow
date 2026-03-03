if (ENABLE_GUI)
    set(FF_GUI_LIBS
        protobuf::libprotobuf
        gRPC::grpc++

        grpc_common
        spdlog::spdlog
        SQLite::SQLite3
        yaml-cpp::yaml-cpp
        Qt6::Widgets
    )

    qt_add_executable(FlexFlowGUI
        guimain.cpp
        guimain.qrc

        # view
        view/about.ui
        view/mainwindow.ui
        view/output.ui
        view/settings.ui

        # controller
        controller/gui/about.cpp
        controller/gui/about.hpp
        controller/gui/config.cpp
        controller/gui/config.hpp
        controller/gui/global.cpp
        controller/gui/global.hpp
        controller/gui/mainwindow.cpp
        controller/gui/mainwindow.hpp
        controller/gui/settings.cpp
        controller/gui/settings.hpp
        controller/gui/settingsthread.cpp
        controller/gui/settingsthread.hpp
    )

    add_dependencies(FlexFlowGUI grpc_common ffmodel)

    target_link_libraries(FlexFlowGUI
        PRIVATE

        ${FF_GUI_LIBS}
        ffmodel
    )

    if (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
        target_link_options(FlexFlowGUI PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
        target_compile_options(FlexFlowGUI PRIVATE $<$<CONFIG:Debug>:/ZI>)
    endif()

endif (ENABLE_GUI)
# the program for testing

if(ENABLE_TEST)
    add_executable(testproc
        model/defines.h
        model/test/testproc.cpp
    )

    add_dependencies(testproc ffmodel)

    target_link_libraries(testproc
        PRIVATE

        cxxopts::cxxopts
        ffmodel
    )

    set(TEST_MODEL_SRC
        ffmodel_ut_main.cpp

        model/test/param.cpp
        model/test/param.hpp

        model/test/grpcmodel_ut.cpp
    )

    add_executable(TestModel
        ${TEST_MODEL_SRC}
    )

    add_dependencies(TestModel ffmodel)

    target_link_libraries(TestModel
        PRIVATE

        GTest::gtest
        GTest::gtest_main
        GTest::gmock
        GTest::gmock_main

        ffmodel
    )

    add_test(TestingModel TestModel)

    if (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
        target_link_options(TestModel PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
        target_compile_options(TestModel PRIVATE $<$<CONFIG:Debug>:/ZI>)
    endif()
endif(ENABLE_TEST)

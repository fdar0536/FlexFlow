# the program for testing

add_executable(testproc
    controller/global/defines.h
    model/test/testproc.cpp
)

add_dependencies(testproc ffmodel)

target_link_libraries(testproc
    PRIVATE

    cxxopts::cxxopts
    ffmodel
)

set(TEST_FFMODEL_SRC
    ffmodel_unittest_main.cpp

    model/test/param.cpp
    model/test/param.hpp

    model/test/grpcmodel_unittest.cpp
)

add_executable(TestFFModel
    ${TEST_FFMODEL_SRC}
)

add_dependencies(TestFFModel ffmodel)

target_link_libraries(TestFFModel
    PRIVATE

    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main

    ffmodel
)

add_test(TestingFFModel TestFFModel)

if (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
    target_link_options(TestFFModel PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
    target_compile_options(TestFFModel PRIVATE $<$<CONFIG:Debug>:/ZI>)
endif()

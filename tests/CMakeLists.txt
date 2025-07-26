cmake_minimum_required(VERSION 3.10)

add_executable(runTests CollisionProbabilityEstimatorTest.cpp ParseDoublesTest.cpp)
target_include_directories(runTests PRIVATE ${CMAKE_SOURCE_DIR}/cpe)
target_link_libraries(runTests cpe_lib cpe_utils_lib)
target_link_libraries(runTests GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(runTests)

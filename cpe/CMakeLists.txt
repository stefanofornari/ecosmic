add_library(cpe_lib CollisionProbabilityEstimator.cpp)
add_library(cpe_utils_lib utils.cpp)
add_executable(cpe main.cpp)
target_link_libraries(ecosmic cpe_lib cpe_utils_lib)

cmake_minimum_required(VERSION 3.10)
project(Ecosmic)

enable_testing()
add_subdirectory(third_party/googletest)

add_subdirectory(cpe)
add_subdirectory(tests)

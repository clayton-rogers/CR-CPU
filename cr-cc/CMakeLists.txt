cmake_minimum_required(VERSION 3.8)

project(cr-cc-top)

add_compile_options(-Wall -Wextra -Wpedantic -Wunused -Wcast-qual -Wcast-align=strict -Wduplicated-branches -Wduplicated-cond -Wnull-dereference -Wno-psabi)

#add_compile_options(-Wswitch-enum)
#add_compile_options(-Wconversion)

set (CMAKE_CXX_STANDARD 20)

add_subdirectory(cr-cc-lib)
add_subdirectory(cr-cc)
add_subdirectory(cr-cc-test)

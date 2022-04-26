cmake_minimum_required(VERSION 3.8)

project(cr-cc)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} cr-cc-lib)

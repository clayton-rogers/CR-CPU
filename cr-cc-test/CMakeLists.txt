cmake_minimum_required(VERSION 3.8)

project(cr-cc-test)

add_executable(${PROJECT_NAME}
    main.cpp
    TEST_assembler.cpp
    TEST_cast.cpp
    TEST_compiler.cpp
    TEST_linker.cpp
    TEST_object_code.cpp
)
target_link_libraries(${PROJECT_NAME} cr-cc-lib)

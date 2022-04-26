cmake_minimum_required(VERSION 3.8)

project(cr-cc-lib)

add_library(${PROJECT_NAME}
    assembler.cpp
    AST_code_gen.cpp
    AST_declaration.cpp
    AST_expressions.cpp
    AST_statements.cpp
    AST.cpp
    c_to_asm.cpp
    cmdline_parse.cpp
    compiler.cpp
    file_io.cpp
    label_maker.cpp
    linker.cpp
    object_code.cpp
    parser.cpp
    preprocessor.cpp
    simulator_bus.cpp
    simulator_core.cpp
    simulator_io.cpp
    simulator_ram.cpp
    simulator_timer.cpp
    simulator_uart.cpp
    simulator_vga.cpp
    simulator_spi.cpp
    simulator_spi_flash.cpp
    simulator.cpp
    tokenizer.cpp
    tokens.cpp
    type.cpp
    utilities.cpp
)
target_include_directories(${PROJECT_NAME} INTERFACE .)
target_link_libraries(${PROJECT_NAME} stdc++fs)

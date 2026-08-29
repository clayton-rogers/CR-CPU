---
name: cr-cc
---

# 📜 Project Rule: CR-CC Compiler & Runtime Testing Framework

## 🎯 Summary Overview
The `cr-cc/` directory constitutes the **Compiler Runtime and Testing Environment** for the entire embedded platform. It is a sophisticated, multi-layered project that does not merely contain code; it defines the entire software validation lifecycle. Its purpose is to ensure language compliance (C standards), verify low-level operating system primitives, and provide a mechanism to test compiler features rigorously before integration into the main hardware core (`fpga/`).

This folder serves as the heart of the platform's **Software Foundation Layer**.

## 🧩 Core Components Breakdown

The directory is organized into three primary functional areas:

### 1. The Build & Execution Environment (Root Level)
*   **`CMakeLists.txt`**: Acts as the root build orchestrator, setting up compilation standards (C++20, strict warnings) and linking together all sub-modules (`cr-cc-lib`, `cr-cc`, `cr-cc-test`).
*   **`setup_paths.sh`**: Initializes the development environment by exporting necessary paths (e.g., pointing to the built libraries and standard library source files), ensuring that compilation and linking use consistent, localized dependencies.
*   **`run_tests.sh`**: The primary entry point for testing. It sources the environment setup and executes a dedicated test harness (`./build/cr-cc-test/`), validating compiler output against defined test cases.

### 2. Compiler Toolchain Engine (`cr-cc-lib/`)
This subdirectory contains the actual implementation of the compiler components, defining how source code is processed:

*   **Parsing & Analysis:** Modules like `parser.*`, `tokenizer.*`, and `AST.*` handle lexical analysis, syntax checking, and building the Abstract Syntax Tree (AST).
*   **Code Generation:** Components such as `compiler.cpp` and modules handling object code (`object_code.*`) are responsible for traversing the AST and generating machine-executable instruction streams or intermediate representations.
*   **Simulation & Verification:** The `simulator_*` files (e.g., `simulator_uart`, `simulator_vga`, `simulator_ram`) provide functional emulations of hardware peripherals and system components, allowing the compiler's output to be tested in a virtual environment before synthesis.

### 3. Low-Level Runtime & Testing Primitives
This section governs the behavior of the embedded target device itself.

#### A. Standard Library (`stdlib/`)
These files define the minimum required operating system functions and hardware interactions that must exist on the target platform (often implemented in Assembly for speed). They simulate the bare metal OS environment:
*   **OS Services:** Functions like `os_puts`, `os_sleep`, and `timer.*` provide essential, low-level APIs (like printing to console or pausing execution) that are fundamental to any embedded application.
*   **Hardware Abstraction:** Files related to `vga.h`/`.s` and memory management define the interface between high-level code and physical display/memory resources.

#### B. Test Data (`test_data/`)
This is the exhaustive suite of test vectors, ensuring maximum coverage:
*   **`valid_c/`**: Contains compilable source files demonstrating correct usage of C language features (e.g., valid scope rules, bitwise operations, pointer arithmetic). Used for *positive testing*.
*   **`invalid_c/`**: Contains deliberately malformed or semantically incorrect code snippets. These are used to ensure the compiler correctly throws specific errors and warnings—a critical measure of compiler robustness.
*   **Categorical Tests (e.g., `asm/`, `echo/`, `relocation/`)**: Grouped files that target specific, narrow functionality (like assembly instruction sets or memory mapping logic), providing deep validation for isolated parts of the system architecture.

## 💡 Conclusion / Role in Development Flow
To modify or extend this project:
1.  **Language Change:** Update `cr-cc-lib/` to add new AST nodes, parsers, and code generators.
2.  **OS Primitives:** Add/modify files in `stdlib/` (often requiring assembly updates) if a new hardware interaction or OS service is needed.
3.  **Language Test:** Create/modify test cases within `test_data/valid_c/` (for positive validation) or `test_data/invalid_c/` (for error coverage).

This directory ensures that both the **language semantics** and the **low-level system services** are functionally correct, acting as a vital pre-hardware verification step.
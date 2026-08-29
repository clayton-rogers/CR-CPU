---
name: cr-cc2
---

# 📜 Project Rule Addendum: CR-CC Executable Driver Module

## 🎯 Summary Overview (Directory: `cr-cc/cr-cc/`)

The directory `cr-cc/cr-cc/` represents the **Final Integration and Execution Layer** for the compiler testing environment. Unlike `cr-cc-lib/` which contains the *definition* of the compiler components, this module provides the executable entry point (`main.cpp`) that initializes and runs the entire compilation test suite.

It is the critical component responsible for taking all compiled pieces (parser, simulator cores, standard library routines) and packaging them into a runnable binary for validation.

## 🧩 Component Analysis
*   **`CMakeLists.txt`**: This file dictates how the executable is built. It defines `cr-cc` as an executable that must link together its own source (`main.cpp`) with the entire library implementation defined in the parent directory's build module (`cr-cc-lib`).
*   **`main.cpp`**: This serves as the **Application Driver** or main test harness entry point. Its primary function is to instantiate and run the comprehensive tests that utilize the compiler functionality provided by `cr-cc-lib/`. It likely contains boilerplate code that sets up memory, passes initial parameters, and calls the core logic functions in sequence.

## 💡 Conclusion / Role in Development Flow
This module should be viewed as the **Test Orchestrator**. If a developer modifies the internal workings of the compiler (e.g., changing how AST nodes are handled), they must ensure that `main.cpp` and its corresponding `CMakeLists.txt` still correctly initialize and trigger these modified components to validate their functionality through execution.

**In short, if `cr-cc-lib/` is the engine, `cr-cc/cr-cc/` is the key that turns it on.**
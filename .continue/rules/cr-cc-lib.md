---
name: cr-cc-lib
---
# The CR-CC Compiler Core Library (cr-cc-lib)

This library forms the backend and core components of a compiler designed to translate a subset of C language code into low-level object code suitable for embedded systems (Targeting the "CR-CPU"). It implements classic compiler stages: Lexical Analysis $\\rightarrow$ Parsing $\\rightarrow$ Abstract Syntax Tree (AST) Construction/Semantic Analysis $\\rightarrow$ Code Generation.

## 📂 Core Functionality & Components

### 1. Tokenization (`tokenizer.*`)
The system first converts raw source code strings into a stream of meaningful tokens.
*   **`tokens.h`/`.cpp`**: Defines over 70 distinct token types, covering keywords (`key_if`, `key_while`), literals (`constant`, `string_literal`), operators (`<<=`, `&&`, `?:`), and structural elements (e.g., function names, identifiers).
*   **`tokenizer.h`/`.cpp`**: Implements the state machine logic to consume the input stream and categorize characters into `Token` objects. It handles complex token groupings like compound operators (`++`, `->`) and distinguishes between keywords/identifiers.

### 2. Parsing & AST Construction (`parser.*`, `AST*.h/cpp`, `declaration.*`)
The parser consumes the token list to build a structured representation of the source code—the Abstract Syntax Tree (AST).
*   **`parser.h`/`.cpp`**: Uses a context-free grammar approach defined by `C_GRAMMAR` rules to recursively validate and construct nodes (`ParseNode`). It ensures that sequences of tokens adhere to C language syntax rules for function definitions, declarations, statements (`if_statement`, `for_statement`, etc.), and expressions.
*   **AST Nodes:** The AST is represented by classes derived from a common base or specialized structures (e.g., `Assignment_Expression`, `Function_Call_Expression`).
    *   **`AST.h`/`.cpp`**: Contains the high-level components (`Environment`, `VarMap`, `Statement`, `Expression`) responsible for managing scope, variable lifecycles, and traversing/analyzing the code structure.
    *   **`AST_declaration.*`**: Specifically handles parsing declaration statements and function parameters, converting raw parsed nodes into structured `Declaration` objects, which define variables' types (via `Type`) and initial values.

### 3. Type System (`type.*`)
This module implements the type checking logic for basic C data types.
*   **`type.h`/`.cpp`**: Defines a `Type` class that encapsulates metadata like size and broad category (`INTEGRAL`, `POINTER`, `ARRAY`, etc.). It manages how declaration specifiers (e.g., `const`, `volatile`) affect the calculated type information, preventing simple type mismatches during analysis.

### 4. Semantic Analysis & Scope Management (`var_map.*`, `environment*`)
These classes perform the crucial semantic passes—checking if identifiers are used correctly and tracking variable locations.
*   **`VarMap`**: Manages local and global scope visibility. It calculates offsets for local variables on the stack (via `offset_map`), simulating compiler register allocation/stack frame management. It also tracks loop labels (`break`, `continue`).
*   **`Environment`**: Maintains symbol tables for both globally visible symbols (functions, static variables) and manages the overall context during compilation.

### 5. Object Code Emission (`object_code.*`)
This is the final stage, translating analyzed AST nodes into a structured binary format compatible with hardware loading.
*   **Data Structures:** `Object_Container` encapsulates the resulting machine code, external references (symbols defined elsewhere), and relocation information.
    *   The format includes a magic header (`CR-CPU`) and version markers.
*   **Serialization:** The system uses complex template functions in `object_code.cpp` to serialize high-level concepts like symbols, variable types, and machine instructions into raw sequences of `std::uint16_t`.
*   **Instruction Set:** It supports common embedded instruction formats (e.g., `add`, `mov`, `load`) and translates them into their respective 16-bit opcodes for the target hardware.

## 💡 Development Workflow Notes

1.  **Compilation Order:** The process is strictly sequential: Source Code $\\rightarrow$ Tokens $\\rightarrow$ Parse Tree/AST $\\rightarrow$ Semantic Check (Type/Scope) $\\rightarrow$ Object Code.
2.  **Key Abstractions:** The system heavily relies on `std::shared_ptr` and polymorphism (e.g., `Compilable` interface) to allow various compiler components (Statements, Expressions, Functions) to share a common generation mechanism (`generate_code()`).
3.  **Target Hardware:** All emitted code assumes interaction with memory-mapped I/O and adheres to the instruction set of the target processor family (implied by `CR-CPU` magic numbers).

## ⚠️ Important Exclusion Rule

**The files containing prefixes starting with `simulator_` are explicitly excluded from this summary, as they appear to be test harnesses or runtime environment emulators rather than core compiler logic.**
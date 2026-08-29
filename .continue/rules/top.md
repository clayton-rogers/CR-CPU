---
name: Project Rules
alwaysApply: true
---

The "rules" available in this project follow. To retrieve the rule you must use only the name. Everything after the `:` is a description to help you do know what information is inside so you know when to call it.
- Test: Contains a secret.
- cr-cc: Contains a description of the cr-cc folder.
- cr-cc2: Contains a description of the nested folder cr-cc/cr-cc.
- cr-cc-lib: Contains a description of the nested folder cr-cc/cr-cc-lib. This is the core compiler.
- fpga: Contains a description of the fpga folder.
- Other: Contains data about aliens.


# CR-CPU

## 💡 Project Overview

This project appears to be a comprehensive platform for developing and simulating embedded systems, focusing heavily on hardware acceleration and robust language runtime testing. It integrates custom digital logic design (HDL) with advanced compiler/language features testing, potentially targeting specialized low-power or high-efficiency devices (indicated by the reference datasheets).

**Purpose:** To provide a complete development environment for creating, verifying, and simulating embedded components, ranging from basic arithmetic units to complex peripherals like VGA and UART.

**Key Technologies Used:**
*   **Hardware Description Language (HDL):** Verilog (`*.v`) used within the `fpga/` directory for defining digital logic circuits (IP cores).
*   **Low-Level Programming:** C/C++ used in utility programs (`PixelClockCalc/`) and extensive testing environments (`cr-cc/`).
*   **Scripting:** Python (`font/extract_font.py`) for utilities and build scripts (e.g., `run_tests.sh`).
*   **Simulation/Verification:** Comprehensive test benches, unit tests, and compiler conformance tests are central to the development cycle.

**High-Level Architecture:**
The architecture is highly layered:
1.  **Peripheral Layer (`fpga/`):** Contains fundamental building blocks (e.g., `ALU`, `DFF`, `UART`) implemented in HDL. These form the digital backbone.
2.  **CPU Core (`fpga/cpu.v`):** The central processing unit that orchestrates data flow between peripherals.
3.  **Language Runtime Layer (`cr-cc/`):** Provides a rigorous testing framework to ensure language features (C standards) are implemented correctly, acting as the software foundation for the core logic.
4.  **Application/Utility Layer:** High-level programs that initialize and control the system (e.g., `PixelClockCalc`) or process input assets (e.g., font generation).

---

## 🚀 Getting Started

### Prerequisites

To successfully develop and run this project, the following software dependencies are required:
*   **Toolchain:** A C/C++ compiler suite (GCC/Clang) capable of handling embedded targets.
*   **HDL Tools:** A simulator environment (e.g., Icarus Verilog or ModelSim) compatible with Verilog to simulate the `fpga/` core logic.
*   **Python:** Python 3.x for running utility scripts like those in the `font/` directory.
*   **Build System:** CMake and Make are required, particularly for the `cr-cc/` component.
*   **Hardware Reference:** Access to relevant datasheets (e.g., iCE40LPHXFamilyDataSheet) found in the `reference/` directory is critical for understanding constraints and pinouts.

### Installation Instructions

1.  **Clone Repository:** Clone the repository into your workspace.
2.  **Compile Utilities:** Build any necessary application components (e.g., compiling `PixelClockCalc/main.cpp`).
3.  **Setup Compiler Environment (CR-CC):** Navigate to the `cr-cc/` directory and execute setup scripts provided here:
    *   `./setup_paths.sh` (To set up environment variables and include paths).
4.  **Build FPGA Core:** Use your HDL toolchain within the `fpga/` directory, referencing the core IP files (`*.v`) to generate synthesizable netlists or test vectors using the provided test benches (`*_tb.v`).

### Basic Usage Examples

*   **Calculating Clocks:** Run the application in `PixelClockCalc/`. This utility takes display dimensions and refresh rates and calculates necessary pixel clock values, storing results in `pixel_clocks.txt`.
*   **Running Language Tests:** Execute unit tests for language compliance by running scripts like `./cr-cc/run_tests.sh` located within the `cr-cc/` directory. These tests cover specific C features (e.g., assignments, bitwise operations).
*   **Simulating Hardware:** Run the top-level test bench: `vvp fpga/top_tb.v` (Assuming Icarus Verilog usage; replace with actual simulator command).

### Running Tests

There are two main testing dimensions:

1.  **Language Feature Tests:** Located in `cr-cc/test_data/`. These tests validate the correct implementation of C language standards (e.g., scope rules, variable declarations) and should be run via `./run_tests.sh`.
2.  **Hardware Functional Tests:** Located in `fpga/`. Use dedicated test benches (`*_tb.v`) to verify the functionality of individual IP cores (e.g., CPU or ALU).

---

## 📂 Project Structure

| Directory/File | Purpose | Details |
| :--- | :--- | :--- |
| **`fpga/`** | **Hardware Core Logic / IP Cores** | Contains the Verilog implementation of all digital hardware components. This is the core physical design layer. Includes `cpu.v`, `alu.v`, `uart.v`, etc. |
| **`cr-cc/`** | **Compiler Runtime & Testing** | The dedicated environment for testing C language compliance and runtime semantics. Contains CMake files, test data directories (`test_data/`), and build scripts. |
| **`PixelClockCalc/`** | **Utility Application (C++)** | A standalone application responsible for calculating necessary clock frequencies for the display hardware based on resolution parameters. |
| **`font/`** | **Asset Processing** | Contains scripts (`extract_font.py`) and source images for generating optimized font bitmaps used by the system UI. |
| **`reference/`** | **Documentation / Datasheets** | Crucial external documentation (PDFs) detailing constraints, electrical specifications, and usage guides for the target hardware family (iCE40). |
| **`.gitignore`, etc.** | Standard Version Control & Metadata. | |

---

## ⚙️ Development Workflow

### Coding Standards
*   **HDL:** Adhere to standard Verilog/VHDL practices. Use modular design, clear hierarchical naming conventions, and include test benches for *every* major component.
*   **C/C++:** Follow modern C++ standards (C++17+ assumed). Code in `PixelClockCalc/` should be clean, optimized, and focus on minimizing dependencies.
*   **Scripting:** Python scripts must have clear docstrings, type hints, and robust error handling.

### Testing Approach
The project employs a **multi-faceted testing strategy**:
1.  **Unit Testing (HDL):** Verifying small components in `fpga/` using test benches.
2.  **Functional Testing (Compiler):** Using the exhaustive suite of C code tests in `cr-cc/test_data/` to validate language semantics and compiler behavior.
3.  **Integration Testing:** Running high-level simulation runs that connect multiple IP cores (e.g., simulating data flow from CPU -> ALU -> VGA).

### Build and Deployment Process
*   **Development Cycle:** Modify a component $\rightarrow$ Write/Update Test Bench $\rightarrow$ Run Tests ($\text{fpga/} \rightarrow \text{simulate} \rightarrow \text{fix}$) $\rightarrow$ Recompile.
*   **Synthesis & Bitstream Generation (Assumption):** After successful simulation, the HDL code must be synthesized using vendor-specific tools (e.g., iCE40 flow) to generate a final bitstream/hex file (`fpga/top.hex`).

### Contribution Guidelines
1.  Ensure all new feature development includes corresponding unit tests.
2.  Keep `fpga/` modular; do not tightly couple unrelated IPs.
3.  When adding C language test cases, follow the naming and structure conventions established in `cr-cc/test_data/`.

---

## 🧠 Key Concepts

*   **IP Core:** An Intellectual Property core is any self-contained functional block defined within `fpga/` (e.g., ALU, UART).
*   **Clock Domain Crossing (CDC):** Be extremely mindful of signal integrity when data passes between different clock domains in the FPGA design. Utilize synchronizers where necessary (`fpga/signal_crossdomain.v`).
*   **Instruction Set Architecture (ISA):** The `cpu.v` component defines the project's ISA. All peripheral designs must interact correctly with this defined instruction set.
*   **Memory Mapping:** Hardware peripherals communicate via memory-mapped I/O. Understanding which address ranges map to specific functions is critical for writing control software.

---

## 🛠️ Common Tasks

### Task 1: Adding a New Peripheral (e.g., GPIO)
1.  Design the core logic in Verilog (`gpio.v`).
2.  Write a comprehensive test bench (`gpio_tb.v`) to verify all edge cases (read/write, high/low).
3.  Integrate the IP into `fpga/top.v` by adding connections and control signals.
4.  Update memory maps and documentation.

### Task 2: Updating a C Language Feature Test (e.g., New Scope Rule)
1.  Create or modify a file in `cr-cc/test_data/valid_c/`.
2.  Ensure the test code is minimal, focused solely on demonstrating the language feature being tested.
3.  Update the build scripts to include the new test data path.

---

## ⚠️ Troubleshooting

**Issue:** Simulation fails with "Unknown signal" or connectivity errors in `fpga/`.
*   **Solution:** Check clock domains and ensure all necessary wires are explicitly connected, even if they appear unused. Verify port definitions (`.v` files) against the top module's requirements.

**Issue:** Compiler tests fail intermittently in `cr-cc/`.
*   **Solution:** This often points to race conditions or compiler optimization differences. Review the specific test file and ensure it is deterministic across different build environments.

**Issue:** Peripheral data seems corrupted (`fpga/uart_ram.v`).
*   **Solution:** Check for missing synchronization logic (CDC) if the peripheral handles asynchronous communication, or verify bus protocols (e.g., address decoding).

---

## 📚 References

*   **Target Hardware Datasheet:** [iCE40LPHXFamilyDataSheet](reference/iCE40LPHXFamilyDataSheet.pdf)
*   **Usage Guide:** [iCE40sysCLOCKPLLDesignandUsageGuide](reference/iCE40sysCLOCKPLLDesignandUsageGuide.pdf)
*   **Compiler Testing Framework:** (Internal Documentation needed for `cr-cc/`)

***

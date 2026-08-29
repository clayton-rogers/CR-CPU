# CR-CPU Project Guide

This guide provides a comprehensive overview of the **CR-CPU** ecosystem, including its hardware design (FPGA), custom compiler toolchain (`cr-cc`), operating system/standard library, and development workflows.

## 1. Project Overview

### Purpose
The **CR-CPU** project is a full-stack custom computing platform consisting of:
1. A 16-bit Von Neumann soft CPU architecture implemented in Verilog on a Lattice iCE40 FPGA (TinyFPGA BX).
2. A custom C compiler toolchain (`cr-cc`) supporting C compilation, assembly, linking, and cycle-accurate emulation.
3. A lightweight embedded operating system and C standard library (`stdlib/`) providing OS primitives, math routines, and I/O drivers.
4. Integrated peripherals: UART (115200 baud), monochrome text VGA (800x600 @ 60Hz), 32-bit hardware timer, and 8-bit GPIO.
5. Developer tooling, including an emulator, VS Code / Notepad++ syntax highlighting, and a PLL pixel clock calculator.

### Key Technologies Used
- **HDL / Digital Design:** Verilog-2001, Lattice iCE40 synthesis toolchain (`apio`, `yosys`, `nextpnr`, `tinyprog`).
- **Compiler / Toolchain:** C++20, CMake (minimum version 3.8), Clang/GCC.
- **Languages Target:** Custom 16-bit CR-CPU Assembly and a subset of C.
- **Peripheral / Tooling:** Python 3 (font generation), C++ (PLL pixel clock calculation), S-Record (`srec`) hex format for loading code over UART.

### High-Level Architecture
```
+-------------------------------------------------------------------------+
|                              Host Machine                               |
|  +--------------------+    +--------------------+    +---------------+  |
|  | cr-cc (C Compiler) | -> | Assembler & Linker | -> | SREC / Hex /  |  |
|  +--------------------+    +--------------------+    | Bin / Map     |  |
|                                       |              +---------------+  |
|                                       v                      |          |
|                           +-----------------------+          | UART     |
|                           | Cycle Emulator (sim)  |          | upload   |
+---------------------------------------|----------------------|----------+
                                        v                      v
+-------------------------------------------------------------------------+
|                  Target Hardware (TinyFPGA BX - iCE40)                  |
|  +-------------------------------------------------------------------+  |
|  |                            cpu.v                                  |  |
|  |  +-----------------+  16-bit Bus  +----------------------------+  |  |
|  |  |   core.v / ALU  | <==========> | 4096-Word Block RAM (ram.v)|  |  |
|  |  +-----------------+              +----------------------------+  |  |
|  |          ^                                                        |  |
|  |          | Memory-Mapped Bus (0x8100 - 0x8300)                    |  |
|  |          v                                                        |  |
|  |  +---------------+  +---------------+  +------------------------+ |  |
|  |  | GPIO (0x8100) |  | Timer (0x8200)|  | UART (0x8300)          | |  |
|  |  +---------------+  +---------------+  +------------------------+ |  |
|  |  +--------------------------------------------------------------+ |  |
|  |  | VGA Peripheral (Char RAM & Bitmap ROM Font Engine)             | |  |
|  |  +--------------------------------------------------------------+ |  |
|  +-------------------------------------------------------------------+  |
+-------------------------------------------------------------------------+
```

## 2. Getting Started

### Prerequisites
- **For Compiler (`cr-cc`):**
  - CMake $\ge 3.8$
  - C++20 compatible compiler (GCC 10+, Clang 11+, or MSVC 2019+)
  - Make / Ninja build tools
- **For Hardware (`fpga/`):**
  - Python 3.x
  - [Apio](https://github.com/FPGAwars/apio) (`pip install apio`)
  - Apio packages: `apio install system scons icestorm iverilog`
  - [TinyProg](https://github.com/tinyfpga/TinyFPGA-BX) for flashing: `pip install tinyprog`
- **For Serial Communication & Utilities:**
  - PuTTY / Plink or `screen`/`minicom` for serial communication.
  - Python PIL/Pillow for font extraction (`font/extract_font.py`).

### Building the Toolchain (`cr-cc`)
```bash
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)
```

To set up environment paths for the standard library and `cr-cc` binary:
```bash
source ./setup_paths.sh
```

### Running Tests
- **Compiler / Assembler / Linker Test Suite:**
  ```bash
  ./run_tests.sh
  ```
  *(Or execute `./build/cr-cc-test/cr-cc-test` directly.)*

- **HDL Simulations:**
  ```bash
  cd fpga
  apio sim
  ```

### Building and Programming the FPGA
```bash
cd fpga
apio clean
apio build -v
tinyprog -p hardware.bin
```


## 3. Project Structure

```
cr-cpu/
├── cr-cpu.md                  # Comprehensive CPU specification (ISA, registers, opcodes)
├── README.md                  # Quickstart overview and status summary
├── cr-assembler-npp-language.xml # Notepad++ syntax highlighting definition
│
│                              # Complete C compiler toolchain and runtime
├── CMakeLists.txt             # Root build script for compiler suite
├── setup_paths.sh             # Exports PATH and CRSTDLIBPATH
├── run_tests.sh               # Test execution runner script
├── cr-cc/                     # CLI driver executable (main.cpp)
├── cr-cc-lib/                 # Compiler backend, tokenizer, parser, AST, linker, emulator
├── cr-cc-test/                # C++ unit and integration test runner
├── stdlib/                    # Minimal OS kernel (os.s), runtime routines, standard library
├── test_data/                 # Test suite with valid/invalid C, asm, and whole-program tests
│
├── fpga/                      # Synthesizable Verilog HDL design for TinyFPGA BX
│   ├── apio.ini               # Apio target configuration (TinyFPGA-BX)
│   ├── pins.pcf               # Physical pin constraint definitions
│   ├── opcodes.vh             # Instruction opcode constants
│   ├── top.v / top_tb.v       # Top-level module and simulation testbench
│   ├── cpu.v / core.v / ALU.v # CPU interconnect, control unit, and arithmetic logic unit
│   ├── ram.v                  # 4096-word system Block RAM (initialized from top.hex)
│   ├── uart*.v                # Full duplex UART core and FIFO RAM
│   ├── vga*.v / sync_gen.v    # VGA controller, PLL, character generator, bitmap ROM
│   └── timer.v / gpio.v       # 32-bit hardware timer and 8-bit GPIO
│
├── cr-asm-ext/                # VS Code extension for CR-CPU assembly syntax highlighting
├── font/                      # Font bitmap asset and Python generator (`extract_font.py`)
├── PixelClockCalc/            # C++ helper tool calculating valid iCE40 PLL divider values
├── reference/                 # Lattice iCE40 datasheets, memory guides, and flash docs
└── .continue/rules/           # Context-specific agent rules for Continue
```


## 4. Development Workflow

### Toolchain Architecture & Pipeline
Source files are processed through the following stages in `cr-cc-lib`:
1. **Preprocessor:** Handles includes and basic macros (`preprocessor.cpp`).
2. **Tokenizer:** Lexical analysis into strongly typed tokens (`tokenizer.cpp`, `tokens.h`).
3. **Parser:** Builds Abstract Syntax Tree (`parser.cpp`, `AST.cpp`, `AST_declaration.cpp`).
4. **Type Check & Scope:** Symbol and variable mapping (`type.cpp`, `var_map.cpp`, `environment.h`).
5. **Code Generation:** Lowers AST into CR-CPU assembly and binary instructions (`AST_code_gen.cpp`, `c_to_asm.cpp`).
6. **Assembler & Linker:** Resolves relative offsets, exports/imports, and memory relocations (`assembler.cpp`, `linker.cpp`).
7. **Emulation / Output:** Emits `.bin`, `.hex`, `.srec`, or simulates execution (`simulator*.cpp`).

### Standard Library & OS Kernel (`stdlib`)
- `os.s` / `os-*.s`: Implements the boot loader, UART S-Record receiver, and system calls.
- `top.hex`: The compiled ROM image of `os.s`, embedded directly into the FPGA memory configuration (`ram.v`).
- When modifying OS routines:
  ```bash
  cd stdlib
  make clean && make
  ```
  Verify generated `os.hex` matches `top.hex` using `./verify_os_hex.sh`.

### Coding Standards
- **C++:** Strict modern C++20. Builds with pedantic warnings enabled (`-Wall -Wextra -Wpedantic -Wcast-align=strict -Wnull-dereference`). Warnings should be treated as errors.
- **Verilog:** Synchronous design principles. Use non-blocking assignments (`<=`) in sequential blocks and blocking assignments (`=`) in combinational blocks. Always handle clock domain crossing (`signal_crossdomain.v`, `DFF.v`).
- **Assembly:** Follow the naming conventions defined in `cr-cpu.md` (labels prefixed with `.`, e.g., `.my_label:`).


## 5. Key Architecture & Hardware Concepts

### Processor Specifications
- **Data & Address Width:** 16-bit words (64k word addressable memory space; 4096 words built-in BRAM).
- **Architecture:** Von Neumann with common 16-bit full-duplex read/write bus.
- **Instructions:** 16-bit fixed width `[oooo aaaa cccc cccc]` (4-bit Opcode, 4-bit Operand/Modifier, 8-bit Immediate).
- **Execution Timing:** Single cycle execution for ALU and write operations (`store`, `push`, `call`). Two cycles for memory reads (`load`, `pop`, `ret`).

### Registers
| Register | Code | Purpose | Calling Convention |
| :--- | :---: | :--- | :--- |
| `ra` | `00` | General Purpose / Return value / Argument 1 | Callee-saved (Caller-saved for args) |
| `rb` | `01` | General Purpose / Argument 2 | Callee-saved (Caller-saved for args) |
| `rp` | `10` | Pointer register (dereference offset base) | Caller-saved |
| `sp` | `11` | Stack pointer (grows downward) | Callee-saved (must restore before `ret`) |
| `addr` | -- | 8-bit high address register (loaded via `loada`) | Caller-saved |
| `ip` | -- | 16-bit instruction pointer | Manipulated by `call`/`ret`/`jmp` |

### Peripheral Memory Map
| Peripheral | Base Address | Notes |
| :--- | :---: | :--- |
| **System RAM** | `0x0000 - 0x0FFF` | 4096 words built-in BRAM |
| **GPIO** | `0x8100` | Write to `0x8101` updates output LEDs |
| **Timer** | `0x8200` | 32-bit hardware counter (low word / high word) |
| **UART** | `0x8300` | Data and status registers (115200 baud, 256B FIFO) |
| **VGA Frame Buffer** | `0x9000` | Monochrome text mode (char RAM & attribute registers) |


## 6. Common Tasks

### Task 1: Compiling a C Program and Simulating It
```bash
# Compile and simulate directly in cr-cc
cr-cc --sim --sim-steps 50000 program.c

# Dump simulation state at each cycle to sim_out.txt
cr-cc --sim --sim-out program.c
```

### Task 2: Compiling and Flashing Over Serial (S-Record)
The built-in OS kernel receives SREC files over UART:
```bash
# Compile program to SREC format
cr-cc --srec -o program.srec program.c

# Upload to FPGA via serial port (Windows example with plink)
plink COM6 -serial -sercfg 115200,8,n,1,N < program.srec
```

### Task 3: Adding a New C Language Feature or Unit Test
1. Add test case in `test_data/valid_c/<feature>.c` or `test_data/invalid_c/<feature>.c`.
2. Implement parsing in `cr-cc-lib/parser.cpp` and `AST.cpp`.
3. Add code generation logic in `cr-cc-lib/AST_code_gen.cpp`.
4. Run `./run_tests.sh` to ensure all tests pass without regressions.

### Task 4: Adding or Modifying a Peripheral in Verilog
1. Create peripheral module in `fpga/` (e.g., `spi_peripheral.v`).
2. Instantiate module in `fpga/cpu.v` or `fpga/top.v`, wire into memory bus with dedicated address decode logic.
3. Add peripheral simulation model in `cr-cc-lib/simulator_*.h/cpp` to ensure toolchain simulation support.
4. Add C header and driver implementation in `stdlib/`.


## 7. Troubleshooting

- **Compiler Error: "Jump out of range" during assembly:**
  Relative jumps (`jmp.r`, `call.r`) have a signed 8-bit range (+127 to -128 words). For larger distances, use absolute addressing with `loada .target` followed by `jmp .target` or `call .target`.
- **FPGA Synthesis Warning: Timing Violation:**
  Check VGA PLL settings and clock domain crossings. Refer to `PixelClockCalc/` output and ensure `signal_crossdomain.v` is used for signals passing between the 16 MHz main clock and VGA pixel clock.
- **Serial Port Output Garbage or No Response:**
  Verify baud rate is set to exactly 115200 8N1. Ensure the board is powered and TinyFPGA BX pin 23 (TX) and 24 (RX) are properly connected.
- **OS Verification Failure (`verify_os_hex.sh`):**
  If changes were made to `stdlib/os.s`, ensure `os.hex` is rebuilt and synced with `fpga/top.hex`.


## 8. References
- [CR-CPU ISA and Specification](cr-cpu.md)
- [FPGA Architecture Details](fpga/README.md)
- [Lattice iCE40 LP/HX Datasheet](reference/iCE40LPHXFamilyDataSheet.pdf)
- [iCE40 sysCLOCK PLL Design and Usage Guide](reference/iCE40sysCLOCKPLLDesignandUsageGuide.pdf)
- [TinyFPGA BX User Guide](https://tinyfpga.com/bx/guide.html)


## Terminal Interaction (Manual Execution)

**The agent does not have direct access to execute shell commands.** When a terminal operation is required, the agent must request the user to execute the command manually.

### Protocol:
1.  **Environment:** The target system is **x86-64 Ubuntu Linux**.
2.  **Command Request:** You do not need to explain the command. You do not need to instruct the user to run the command, the user knows to run the command and reply with the result. The agent should provide the exact command in a markdown code block for the user to copy and run. The command should be the end of the agent message, only one command can be issued at a time. The user will reply with the complete terminal output, treat this as if it was being returned by a tool call.
3.  **Command Chaining:** Because the terminal state may not be preserved between manual executions, the agent should combine multi-step operations into a single command string using `&&` (e.g., `mkdir build && cd build && cmake ..`).

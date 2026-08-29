---
name: fpga
---
# FPGA Core Architecture Rule Set (fpga/)

This document serves as a comprehensive, high-level summary of the Hardware Description Language (HDL) components, Intellectual Property (IP) cores, and utility modules contained within the `fpga/` directory. It is designed to provide an AI reader with immediate context regarding the system's physical layer design, architectural flow, memory hierarchy, and peripheral responsibilities.

## 🌐 System Goal: Embedded CPU Simulation Platform
The project implements a complete embedded digital computer simulator targeting an FPGA (TinyFPGA-BX). The core function is to simulate a custom RISC-like CPU executing code while simultaneously driving complex peripherals like VGA and UART. All components are interconnected via standardized bus interfaces (`read_addr`, `write_addr`, etc.) managed by the `cpu` wrapper.

## 🚀 Architectural Flow (The Data Path)
1.  **System Entry Point:** The process starts at `top.v`.
2.  **CPU Execution:** The `cpu.v` module acts as the central arbiter, connecting all major components to a common bus.
3.  **Instruction Fetch/Execute:** The `core.v` module fetches instructions from system memory (`ram.v`), decodes them using defined opcodes (`opcodes.vh`), and executes actions (e.g., ALU operations, register updates).
4.  **Data Processing:** Arithmetic and Logic are handled by the dedicated `alu.v`.
5.  **Memory Interaction:** Program data is stored in general memory (`ram.v`) or specialized buffers like character/bitmap RAMs (`char_ram.v`, `bitmap_ram.v`).
6.  **I/O & Output:** The CPU routes necessary operations to peripherals: GPIO, VGA, and UART.
7.  **Safety Measure:** All inter-module communication involving asynchronous signals is managed by the synchronizers (`signal_crossdomain.v`, `DFF.v`).

## 🧩 Key Modules and Responsibilities (The IP Cores)

### 1. Central Processing Unit & Control:
*   **`cpu.v`**: The main wrapper module. It instantiates the CPU core, memory, and all peripherals, routing signals to establish the system topology.
*   **`core.v`**: Contains the entire control logic (Control Unit). It handles PC incrementing, instruction decoding based on `opcodes.vh`, calculating jump targets, and managing multiple execution stages (e.g., 2-cycle instructions).
*   **`alu.v`**: The Arithmetic Logic Unit. Executes core mathematical operations defined by opcodes (`ADD`, `SUB`, `AND`, etc.).
*   **`ram.v`**: The primary system memory module, responsible for storing the program code and general data structure, initialized from `empty.hex`.

### 2. I/O & Peripherals:
*   **`gpio.v`**: Manages General Purpose Input/Output pins. It controls `output_pins` based on a write strobe to a specific address (`BASE_ADDR + 1`).
*   **`uart_peripheral.v` / `uart.v`**: Provides duplex serial communication. The wrapper handles memory mapping for status registers and buffers, while `uart.v` performs low-level bit timing for TX (`PIN_23`) and RX (`PIN_24`).
*   **`vga_peripheral.v` / `vga.v`**: Handles video output synchronization and buffering, generating RGB color signals, H-sync, and V-sync. It manages connections to pixel data via a frame buffer.
*   **`timer.v`**: Provides timekeeping capabilities, crucial for controlling timing loops, frame rates, or simulating delays.

### 3. Memory & Display Subsystems:
*   **`char_ram.v`**: Specialized read-only memory block for character bitmaps.
*   **`bitmap_ram.v`**: Specialized read-only memory block for full font maps, initialized from `font_bitmap.hex`.
*   **`char_counter.v`**: A complex timing utility that tracks character coordinates (`x`, `y`) and generates a blanking signal, synchronizing the CPU's view of screen space with the pixel clock.*

### 4. Synchronization & Timing Utilities:
*   **`signal_crossdomain.v`, `DFF.v`**: Critical modules implementing Clock Domain Crossing (CDC) logic. These use double flip-flops (`DFF`) to safely transfer control signals or data between two physically separate clock domains (`i_clk_a` and `i_clk_b`), preventing metastability errors.
*   **`sync_gen.v`**: Generates the necessary horizontal (`h_sync`) and vertical (`v_sync`) sync pulse timing signals required for standard VGA operation.

## 📂 Supporting Assets & Definitions
*   **`opcodes.vh`**: A header file defining all machine instructions (e.g., `ADD`, `STORE`, `LOADI`), their corresponding opcode values, and special sub-command flags (`SUB_RET`, `SUB_PUSH`).
*   **`apio.ini`**: External configuration file used by the build system to identify the target board parameters (TinyFPGA-BX).
*   **`readme.md`**: Contains operational instructions for building and programming the simulated hardware core.

## 🛠️ Development Workflow & Best Practices
1.  **Modularity:** The design is highly modular, with each peripheral or function contained in its own module (`gpio`, `uart`, etc.).
2.  **Bus Protocol:** All communication uses a standardized bus protocol: read/write address/data and a write strobe signal.
3.  **Simulation Testing:** Unit tests (e.g., `ALU.test`, `cpu.test`) are mandatory for verifying individual module correctness before integration into the top level."

## 🗺️ Memory Map & Pinouts
*   **GPIO:** `16'h8100` (Base Address)
*   **Timer:** `16'h8200` (Base Address)
*   **UART:** `16'h8300` (Base Address)
*
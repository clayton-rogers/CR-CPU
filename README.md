# CR-CC CPU project

This project aims to produce a soft (FPGA) CPU design and a C compiler for it. See the [detailed description](cr-cpu.md) for more details.

## Progress

Currently functional are:
- The 16 bit CPU implemented on a [TinyFPGA BX](https://tinyfpga.com/bx/guide.html)
- 8192 bytes (4096 words) of built in RAM (using block RAM on the FPGA)
- A 8-bit GPO connected to LEDs
- A UART running at 115200, with 256 bytes TX and RX buffers
- A VGA adaptor running at 800x600, 60Hz, monochrome, text mode
- A 32-bit basic timer
- An assembler (handles symbol resolution within translation unit, calculates relative offsets)
- A C compiler (not all language features implement, see detailed description)
- A linker (handles relocations, exported and imported references)
- A partial standard library
- A CPU emulator which can be run on the host device (includes most peripherals)

Additional support components:
- A python font generator to convert from PNG to FPGA hex file
- A pixel clock calculator to calculate every possible PLL clock supported with the on board 16 MHz input clock

Currently planned features:
- A SPI peripheral to communicate with the on board QSPI flash
- A fully working C compiler
- A full C standard library

## Development Tips and Tricks

A notepad++ language file is included in the root of the project to more easily view CR-CPU assembler.

Recommended args when hex dumping object or bin files: `xxd -e -g 2 obj.o` This will set the endianness and specify 2 bytes per word.

You can use PuTTY (plink) to connect to the device's serial. Ex. `plink COM6 < flash.srec`. This allows to you have access to the UART without having to copy/paste the SREC into a PuTTY terminal.

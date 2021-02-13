# CR-CC CPU project

This project aims to produce a soft (FPGA) CPU design and a C compiler for it.

## Progress

Currently the CPU and several peripherals are working on a Tiny FPGA BX. An assembler with symbol resolution works. A linker with relocation, and import/export symbols works. A partial implementation of a C compiler exists.

## Tips and Tricks

Recommended args when hex dumping object or bin files: `xxd -e -g 2 obj.o` This will set the endianness and specify 2 bytes per word.

You can use PuTTY (plink) to connect to the device's serial. Ex. `plink COM6 < flash.srec`.

# AGENTS.md

CR-CPU is a custom 16-bit soft CPU plus its C toolchain. Key parts:
- `fpga/` - Verilog CPU + peripherals (apio / TinyFPGA BX project)
- `cr-cc-lib/` - compiler, assembler, linker, simulator (single C++20 library)
- `cr-cc/` - CLI frontend: `cr-cc` drives compile, assemble, link, and `--sim` simulation
- `cr-cc-test/` - Catch2 v2.13.x unit tests (vendored single-header `catch.hpp`)
- `stdlib/` - OS + partial standard library, built with make
- `test_data/` - per-program makefiles exercising `.c`/`.s` end to end

`cr-cpu.md` is the authoritative ISA, assembler, and calling-convention reference. Read it before touching the assembler or codegen.

## Build and test (repo root only)

Everything is cwd-relative: `setup_paths.sh` uses `pwd`, and unit tests hardcode `./test_data` and `./stdlib`. Run from the repository root:

```
cmake -S . -B build
cmake --build build -j
source ./setup_paths.sh   # puts build/cr-cc on PATH, sets CRSTDLIBPATH=stdlib
./run_tests.sh            # runs ./build/cr-cc-test/cr-cc-test
```

- C++20 with strict warnings on by default (`-Wall -Wextra -Wpedantic` plus more). Don't introduce new warnings; the repo keeps a warning-clean history. No formatter config; C++ and Verilog use tab indentation.
- Benchmarks are excluded by default; run `cr-cc-test "[bench]"` and `cr-cc-test "[long_bench]"` (CI does both, `long_bench` is slow).

## Standard library

```
cd stdlib && make
```

Requires `cr-cc` on PATH and `CRSTDLIBPATH` set (see `setup_paths.sh`). The `os_verify` target aborts unless the freshly built `os.hex` is byte-identical to the committed `fpga/top.hex` (the FPGA boots this image). If you change OS assembly, rebuild and commit the new `fpga/top.hex`.

## Toolchain notes

- `cr-cc` flags: `--sim`/`--sim-steps N` (emulator), `--hex`, `--srec`/`--srec-stdout`, `--map`, `--link-addr N` (default 0x200), `--no-main`, `--no-stdlib`, `-c`, `-S` (emit asm), `--lib`, `--function-size`, `--dump` (inspect an object).
- Objects are little-endian 16-bit words; dump with `xxd -e -g 2 file.o`.
- Each `test_data/*/` has its own makefile: `make sim` runs the emulator, `make upload` sends srec to `TTY`. `./build_and_run_test_code.sh` (from `test_data/`) drives the CI set; several dirs are build-only.

## FPGA

Built with apio: `apio clean && apio build -v`; also `apio lint -a -t top`, `apio sim`, `apio time`, `apio verify`. Program the TinyFPGA BX with `tinyprog -p <bitstream>`. iCE40/Flash datasheets are in `reference/`.
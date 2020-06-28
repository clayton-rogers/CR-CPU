# CR CPU FPGA

## Description
This is the FPGA implementation of the CR CPU and all peripherals.
It includes a default OS (top.hex) which should be identical to os.s
in the stdlib.

## How to build
apio clean
apio build -v

## See also
apio lint -a -t top
apio time
apio sim
apio verify

## How to program
tinyprog -p <bistreamname>

# VGA TEST
## Description
This is a simple sub repo to test VGA output on the TINY BX.

## Breadboard setup
Output of the FPGA is nominally 3V3, but often seems to be closer to 3V0.

VGA expects 0 to 0.7 V and has an internal 75 Ohm resistance. If we put an x Ohm
resistor ahead of the monitor then we get:

0.7 = 3 (75 / (x + 75))

thus x = 246.4 Ohm
which is similar to 240 Ohm standard resistor

## Output
The output has two modes. First a static test pattern. Second a user supplied
image built into the ROM.
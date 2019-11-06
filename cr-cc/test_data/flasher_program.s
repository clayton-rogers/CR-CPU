.constant 0x8100 io_offset

# load alternating pattern and all ones
 loadi RA, 0x55
 loadi RB, 0xFF

# output reg and then flip it
 store ra, .io_offset[1] # 1 for output
 xor ra, rb

# jump back to the out instruction
 jmp 2

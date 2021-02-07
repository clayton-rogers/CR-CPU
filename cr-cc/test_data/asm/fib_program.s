.constant 0x8100 io_offset

loada .io_offset

# load first two numbers of series
loadi RA, 1
loadi RB, 1

# add and slide over results
store ra, .io_offset[1] # output
add ra, rb
mov rp, ra
mov ra, rb
mov rb, rp
jmp 2
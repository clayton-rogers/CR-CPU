# start at 10 and keep counting

.constant 0x8100 io_offset
loada .io_offset

loadi ra, 10

.top_loop:
add ra, 1
store ra, .io_offset[1] # 1 is output, 0 is input
jmp.r .top_loop
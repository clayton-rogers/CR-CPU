# technically this is overwriting program memory

.constant 0x8100 io_offset

loada 0
loadi ra, 10
store ra 0
loadi ra, 11
store ra 1
loadi ra, 23
store ra 2
loadi ra, 8
store ra 3
loadi ra, 3
store ra 4

loadi rp 4

.add_loop:
load.rp ra, 0
add ra, 1
store.rp ra, 0
sub rp, 1
mov ra, rp
jmp.gz.r .add_loop

loadi rp 4
xor rb, rb

.sum_loop:
mov ra, rb
load.rp rb, 0
add ra, rb
sub rp, 1
mov rb, ra
mov ra, rp
jmp.gz.r .sum_loop

loada .io_offset
store rb, .io_offset[1]
halt


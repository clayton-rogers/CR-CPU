.static 4 data 12 43 23 52



loadi sp 0xFF

loadi rp .data
loadi.h rp .data


load.rp ra, 0
push ra
load.rp ra, 1
push ra
load.rp ra, 2
load.rp rb, 3

call.r .add_all

.constant 0x8101 io_offset
loada .io_offset
store ra .io_offset
halt


# add_all - adds four numbers together
# INPUT
#   ra
#   rb
#   sp + 1
#   sp + 2
.add_all:
	add ra, rb
	load.sp rb, 1
	add ra, rb
	load.sp rb, 2
	add ra, rb
ret
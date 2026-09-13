## Static data
## sums all the staticall allocated data

.static 5 data 10 22 3 0xF 5
.constant 0x8100 io_offset

loadi sp, 0xFF
loadi rp, .data
loadi.h rp, .data

loadi ra, 0
push ra # sp + 1: counter
push ra # sp + 0: sum


# add the next data point
.top_loop:
load.sp ra, 0
load.rp rb, 0
add ra,rb
store.sp ra, 0
add rp, 1

load.sp ra, 1
add ra, 1
store.sp ra, 1
sub ra, 5 # 5 == sizeof(data)
jmp.nz .top_loop

load.sp ra, 0
loada .io_offset
store ra, .io_offset[1]
halt

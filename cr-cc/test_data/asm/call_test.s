.static 11 rand_buffer
.static 6 vars

.constant 0x8100 io_offset

loada .io_offset

# Make assumption we have at least 256 words of ram,
# and won't need more than that
loadi sp 0xFF


loadi rp .vars
loadi ra 11
store.rp ra 0
loadi ra 12
store.rp ra 1
loadi ra 13
store.rp ra 2
loadi ra 14
store.rp ra 3
loadi ra 15
store.rp ra 4
loadi ra 16
store.rp ra 5



load.rp ra, 0
push ra
load.rp ra, 1
load.rp rb, 2
call.r .fn_add
add sp, 1 # clean up from call
push ra
load.rp ra, 3
push ra
load.rp ra, 4
load.rp rb, 5
call.r .fn_add
add sp, 1 # clean up from call
pop rb # arg from first call
add ra, rb
store ra, .io_offset[1] # output
halt


# add three numbers
#   INPUT
#       ra, rb, sp+1
#   OUTPUT
#       ra
.fn_add:
add ra, rb
load.sp rb, 1
add ra, rb
ret

# offset for io module
.constant 0x8100 io_offset

# create array var
.static 2 var
.static 1 second

loada .var

# load three numbers
loadi ra, 10
store ra, .var[0]
loadi ra, 11
store ra, .var[1]
loadi ra, 22
store ra, .second


# sum up single number
loadi ra, 0
load rb, .second
add rb, ra
# keep sum in rb

# for loop to add up array
loadi rp, 1
.array_loop:
load.rp ra, .var
add rb, ra
sub rp, 1
mov ra, rp
jmp.r.gz .array_loop

loada .io_offset
store rb, .io_offset[1] # output
halt


# create simple var
.static 1 var
.static 1 second

.constant 0x8100 io_offset

loada .var
loadi ra, 10
store ra, .var
loadi ra, 11
loadi rb, 1
add ra, rb
store ra .second
load ra, .var
load rb, .second
add ra, rb
store ra, .var

loada .io_offset
store ra, .io_offset[1] # offset 1 is output
halt


.constant 0x8123 c

nop
nop
nop
nop
.a:

# these are all non-relocatable
# addr 0x04
loada .c
call .c
jmp .c
store ra, .c
load ra, .c
loadi.h ra, .c
loadi ra, .c

# these are all relocatable
# addr 0x0b
loada .a
call .a
jmp .a
store ra, .a
load ra, .a
loadi.h ra, .a
loadi ra, .a
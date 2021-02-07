.constant 0x8123 c

nop
nop
nop
nop
.a: # addr 0x0004

.static 0x01 var 0x4323

# these are all non-relocatable
# addr 0x04
loada .c
call .c
jmp .c
store ra, .c
load ra, .c
loadi.h ra, .c
loadi ra, .c

# these are all non-relocatable
# addr 0x0b
loada .c[1]
#call .c[1] offsets not allowed for JMP or CALL inst
#jmp .c[1]
store ra, .c[1]
load ra, .c[1]
loadi.h ra, .c[1]
loadi ra, .c[1]

# these are all relocatable
# addr 0x10
loada .a
call .a
jmp .a
store ra, .a
load ra, .a
loadi.h ra, .a
loadi ra, .a

# these are all relocatable
# addr 0x17
loada .var
call .var
jmp .var
store ra, .var
load ra, .var
loadi.h ra, .var
loadi ra, .var

# var located here at:
# addr 0x001E

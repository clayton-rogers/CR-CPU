.constant 0x8100 io_offset

loada 0

### all on ###
.top:
loadi ra, 0xff
loada .io_offset
store ra, .io_offset[1] # 1 for output
loada 0


### loop for 400 count ###
loadi ra, 0x90  #0x190 == 400
loadi.h ra, 0x01
.top_loop:
sub ra, 1
jmp.r.gz .top_loop


### all off ###
loadi ra, 0x00
loada .io_offset
store ra, .io_offset[1] # output
loada 0


### loop for another 400 count ###
loadi ra, 0x90  #0x190 == 400
loadi.h ra, 0x01
.top_loop2:
sub ra, 1
jmp.gz.r .top_loop2

### start the whole thing again
jmp .top
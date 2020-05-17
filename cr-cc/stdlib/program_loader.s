.constant 0x1000 RAM_SIZE
.constant 0x0200 PROGRAM_ADDR

# Preamble to setup stack pointer
loadi sp .RAM_SIZE
loadi.h sp .RAM_SIZE
sub sp, 1
loadi ra .end
loadi.h ra .end # load the return address for the program
push ra

loadi ra .PROGRAM_ADDR
loadi.h ra .PROGRAM_ADDR
push ra
ret # jump to last pushed, i.e PROGRAM_ADDR

.end: # when the program returns halt
halt

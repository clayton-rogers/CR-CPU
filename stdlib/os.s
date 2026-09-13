.constant 0x8100 IO_OFFSET
.constant 0x0FFF RAM_SIZE

.os_entry:
# From the last execution, we might have left a value in ra,
# so output it on the LEDs
loada    .IO_OFFSET[1]
store ra .IO_OFFSET[1]

# Preamble to setup stack pointer
loadi sp .RAM_SIZE
loadi.h sp .RAM_SIZE

.static 0x13 WELCOME_STR "Welcome to CR-CPU! "
.static 0x14 unused2 "PASTE SREC PROGRAM >"
.static 0x02 unused3 0x0A 0x00 # LF NULL

loadi ra .WELCOME_STR
loadi.h ra .WELCOME_STR
loada .__uart_puts
call .__uart_puts
.extern __uart_puts

loadi ra .os_entry
loadi.h ra .os_entry
# store the top address on the stack
# this is where the program will go when the client code ret's
push ra
.extern __load_srec_uart
loada .__load_srec_uart
call .__load_srec_uart
push ra
ret # jump to the load address of the srec
halt # should never get here

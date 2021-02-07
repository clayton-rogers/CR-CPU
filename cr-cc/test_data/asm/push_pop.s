.constant 0x8100 IO_OFFSET
.constant 0x8200 TIMER_OFFSET
.constant 0x8300 UART_OFFSET
.constant 0x0100 RAM_SIZE

# Preamble to setup stack pointer
loadi sp .RAM_SIZE
loadi.h sp .RAM_SIZE
sub sp, 1

# The MAIN is called then if it returns,
# processor is halted
loada .main
call .main
halt

.static 6 data 31 25 74 23 42 0
.main:
	loadi ra, 0
	push ra
	loadi rp .data
	loadi.h rp .data
	.L1:
		load.rp ra, 0
		jmp.r.z .L2
		push ra
		add rp, 1
		jmp.r .L1
	.L2:
	
	.L3:
		pop rb
		pop ra
		jmp.r.z .L4
		add ra, rb
		push ra
		jmp.r .L3
	.L4:
	
	loada .IO_OFFSET
	store rb .IO_OFFSET[1]
ret

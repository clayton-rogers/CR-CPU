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



.static 0x0D HELLO_STR 0x68 0x65 0x6C 0x6C 0x6F 0x20 0x77 0x6F 0x72 0x6C 0x64 0x21 0x00
.main:
	loadi ra .HELLO_STR
	loadi.h ra .HELLO_STR
	call.r .put_string
ret

.set_gpio:

ret

# write_uart
# INPUT
#   ra - character to write
.write_uart:
	loada    .UART_OFFSET[2]
	store ra .UART_OFFSET[2]
ret

# put_string
# INPUT
#   ra - address of null terminated string
.put_string:
	push rp
	mov  rp, ra
	.put_string_top:
		load.rp ra, 0
		jmp.r.z .put_string_exit
		call.r .write_uart
		add rp, 1
	jmp.r .put_string_top

	.put_string_exit:
	pop rp
ret

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
.constant 10 NUM_CHAR_TO_WAIT
.main:
	call.r .read_uart_rx_size
	call.r .set_led
	sub ra, .NUM_CHAR_TO_WAIT
	jmp.r.nz .main # go until we have enough char

	.main_top:
	call.r .read_uart
	call.r .write_uart
	call.r .read_uart_rx_size
	jmp.r.nz .main_top
	jmp.r .main
ret

# set_led - sets the led states
# INPUT
#   ra - low 8 bits set leds
.set_led:
	loada    .IO_OFFSET[1]
	store ra .IO_OFFSET[1]
ret

# write_uart
# INPUT
#   ra - character to write
.write_uart:
	loada    .UART_OFFSET[2]
	store ra .UART_OFFSET[2]
ret

# read uart rx - reads a character
# OUTPUT
#   ra - character
.read_uart:
	loada   .UART_OFFSET[3]
	load ra .UART_OFFSET[3]
ret

# read uart rx size - reads number of characters waiting in rx
# OUTPUT
#   ra - number
.read_uart_rx_size:
	loada   .UART_OFFSET[1]
	load ra .UART_OFFSET[1]
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

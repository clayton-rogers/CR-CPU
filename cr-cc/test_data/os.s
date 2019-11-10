.constant 0x8100 IO_OFFSET
.constant 0x8200 TIMER_OFFSET
.constant 0x8300 UART_OFFSET
.constant 0x0100 RAM_SIZE
.constant     16 CLOCKS_PER_MICRO

# Preamble to setup stack pointer
loadi sp .RAM_SIZE
loadi.h sp .RAM_SIZE
sub sp, 1

# The MAIN is called then if it returns,
# processor is halted
loada .main
call .main
halt


# ====================================================================
# YOUR MAIN GOES HERE
# ====================================================================
.static 0x0D HELLO_STR "hello world!"
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

# ====================================================================
# END MAIN
# ====================================================================



# ====================================================================
# OS Library code
# ====================================================================
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

# read_uart_tx_size - reads the number of characters waiting in tx
# OUTPUT
#   ra - number of characters
.read_uart_tx_size:
	loada .UART_OFFSET[0]
	load ra, .UART_OFFSET[0]
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

# sleep_micro - busy waits the given number of microseconds
# INPUT
#   ra - number of microseconds (max 8000)
.sleep_micro:
	jmp.r.z .sleep_micro_done
	shftl ra, 3 # multiply by 8
	sub ra, 6 # 1 call, 1 jmp, 1 shft, 1 sub, 2 ret
	# this will sleep for 16 cycles per loop
	.sleep_micro_top:
		sub ra, 1
	jmp.r.nz .sleep_micro_top
.sleep_micro_done:
ret

# sleep_milli - busy waits for the given number of milliseconds
# INPUT
#   ra - number of milliseconds
.sleep_milli:
	push rb
	jmp.r.z .sleep_milli_done

	.sleep_milli_top:
	mov rb, ra
	loadi ra, 0xE8
	loadi.h ra, 0x03 # 1000 in ra
	call.r .sleep_micro
	mov ra, rb
	sub ra, 1
	jmp.r.nz .sleep_milli_top

	.sleep_milli_done:
	pop rb
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

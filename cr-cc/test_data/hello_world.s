.constant 0x8100 IO_OFFSET
.constant 0x8200 TIMER_OFFSET
.constant 0x8300 UART_OFFSET

.text_offset 0x200

.static 13 HELLO_STR "Hello world!"

# must be the first instruction of the program
.main:
	loadi ra 0x0D
	call.r ._write_uart
	loadi ra 0x0D
	call.r ._write_uart

	loadi ra 20
	push ra
	.main_top_loop:
		loadi ra, .HELLO_STR
		loadi.h ra, .HELLO_STR
		call.r ._put_string
		loadi ra 0x0D
		call.r ._write_uart

		# sleep one second between outputs
		#.constant 200 ONE_SECOND
		#loadi ra .ONE_SECOND
		#loadi.h ra .ONE_SECOND
		#call.r ._sleep_milli

		load.sp ra 0
		sub ra 1
		store.sp ra 0
	jmp.r.nz .main_top_loop
	add sp 1


	loadi ra 0x0D
	call.r ._write_uart
	loadi ra 0x0D
	call.r ._write_uart
ret

# _write_uart
# INPUT
#   ra - character to write
### VERIFIED ###
._write_uart:
	push ra
	.write_uart_retry:
	call.r ._read_uart_tx_size
	sub ra, 0xFF
	# as long as there are not 0xFF characters in TX buffer
	jmp.r.z .write_uart_retry
	.write_uart_good:
	pop ra
	loada    .UART_OFFSET[2]
	store ra .UART_OFFSET[2]
ret

# _read_uart_tx_size - reads the number of characters waiting in tx
# OUTPUT
#   ra - number of characters
._read_uart_tx_size:
	loada .UART_OFFSET[0]
	load ra, .UART_OFFSET[0]
ret

# put_string
# INPUT
#   ra - address of null terminated string
._put_string:
	push rp
	mov  rp, ra
	.put_string_top:
		load.rp ra, 0
		jmp.r.z .put_string_exit
		call.r ._write_uart
		add rp, 1
	jmp.r .put_string_top

	.put_string_exit:
	pop rp
ret

# _sleep_micro - busy waits the given number of microseconds
# INPUT
#   ra - number of microseconds (max 8000)
### VERIFIED ###
._sleep_micro:
	jmp.r.z .sleep_micro_done
	shftl ra, 3 # multiply by 8
	sub ra, 6 # 1 call, 1 jmp, 1 shft, 1 sub, 2 ret
	# this will sleep for 16 cycles per loop
	.sleep_micro_top:
		sub ra, 1
	jmp.r.nz .sleep_micro_top
.sleep_micro_done:
ret

# _sleep_milli - busy waits for the given number of milliseconds
# INPUT
#   ra - number of milliseconds
### VERIFIED ###
._sleep_milli:
	push rb
	jmp.r.z .sleep_milli_done

	.sleep_milli_top:
	mov rb, ra
	loadi ra, 0xE8
	loadi.h ra, 0x03 # 1000 in ra
	call.r ._sleep_micro
	mov ra, rb
	sub ra, 1
	jmp.r.nz .sleep_milli_top

	.sleep_milli_done:
	pop rb
ret

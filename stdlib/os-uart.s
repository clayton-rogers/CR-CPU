.constant 0x8300 UART_OFFSET

# _write_uart
# INPUT
#   ra - character to write
### VERIFIED ###
.__write_uart:
.export __write_uart
	push ra
	.write_uart_retry:
	call.r .__read_uart_tx_size
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
.__read_uart_tx_size:
.export __read_uart_tx_size
	loada .UART_OFFSET[0]
	load ra, .UART_OFFSET[0]
ret

# read uart rx - reads a character,
#                waits for a character to be available
# OUTPUT
#   ra - character
### VERIFIED ###
.__read_uart:
.export __read_uart
	call.r .__read_uart_rx_size
	jmp.r.z .__read_uart
	loada   .UART_OFFSET[3]
	load ra .UART_OFFSET[3]
ret

# read uart rx size - reads number of characters waiting in rx
# OUTPUT
#   ra - number
### VERIFIED ###
.__read_uart_rx_size:
.export __read_uart_rx_size
	loada   .UART_OFFSET[1]
	load ra .UART_OFFSET[1]
ret

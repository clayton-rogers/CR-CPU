
.constant  0x0400 DATA_ADDR
.constant  0x20 NUM_ITEMS

.text_offset 0x0200


.main:
loadi rb .NUM_ITEMS
push rb
loadi ra 0
push ra
# sp + 1 = number remaining
# sp + 0 = sum of values
	loadi rp .DATA_ADDR
	loadi.h rp .DATA_ADDR

	.main_top:
	load.rp ra 0
	load.sp rb 0
	add ra rb
	store.sp ra 0
	
	load.sp ra 1
	sub ra 1
	store.sp ra 1
	
	add rp 1
	
	jmp.r.z .main_done
	jmp.r .main_top

.main_done:
load.sp ra 0
call.r ._print_hex
call.r ._print_nl
pop ra
pop ra
ret

._print_nl:
push ra
	loadi ra 0x0A # LF char
	call.r ._write_uart
pop ra
ret

# _print_hex - prints a 16bit value as hex to an ascii string
#   NOTE: this does not add a null to the end
#   NOTE: four char will be generated: 784 -> "0310"
# INPUT
#   ra - value to be converted
._print_hex:
push ra
# sp + 0 = value to be converted

	# each four bits can be handled independently
	load.sp ra 0
	shftr ra 12
	call.r .print_hex_char
	call.r ._write_uart

	load.sp ra 0
	shftr ra 8
	call.r .print_hex_char
	call.r ._write_uart

	load.sp ra 0
	shftr ra 4
	call.r .print_hex_char
	call.r ._write_uart

	load.sp ra 0
	call.r .print_hex_char
	call.r ._write_uart

add sp 1
ret
# print_hex_char - prints a single 4bit number
# INPUT
#   ra - value to convert, only low 4 bits considered
# OUTPUT
#   ra - value in ascii hex (0-9,A-F)
.print_hex_char:
push rb
push ra
# sp + 0 = value to convert, masked

	loadi rb 0x0F # set up mask
	and ra rb # apply mask
	store.sp ra 0

	# if value is >= 10 then use letters, else numbers
	sub ra 10
	jmp.r.gz .print_hex_char_alpha
	.print_hex_char_num:
	# need to restore the value in this case
	load.sp ra 0
	add ra 0x30 # ascii offset to numerals
	jmp.r .print_hex_char_done

	.print_hex_char_alpha:
	# value already has 10 sub-ed off of it, apply alpha offset
	add ra 0x41
	# fall through to exit
	# jmp.r .print_hex_char_done

.print_hex_char_done:
add sp 1
pop rb
ret



.constant 0x8300 UART_OFFSET
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

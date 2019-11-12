.constant 0x8100 IO_OFFSET
.constant 0x8200 TIMER_OFFSET
.constant 0x8300 UART_OFFSET
.constant 0x0200 OS_RAM_SIZE
.constant 0x0FFF RAM_SIZE
.constant     16 CLOCKS_PER_MICRO


# Preamble to setup stack pointer
loadi sp .OS_RAM_SIZE
loadi.h sp .OS_RAM_SIZE
sub sp, 1

# The MAIN is called then if it returns,
# processor is halted
loada ._main
call ._main
halt


# ====================================================================
# YOUR MAIN GOES HERE
# ====================================================================
.static 0x13 WELCOME_STR "Welcome to CR-CPU!"
.static 0x18 PROMPT_TEXT "READY. PASTE PROGRAM > "
._main:
	loadi sp .OS_RAM_SIZE
	loadi.h sp .OS_RAM_SIZE
	sub sp, 1
	loadi ra, .WELCOME_STR
	loadi.h ra, .WELCOME_STR
	call.r .put_line
	loadi ra, .PROMPT_TEXT
	loadi.h ra, .PROMPT_TEXT
	call.r .put_line

	call.r ._load_srec_uart ### TODO REMOVE comment
	loadi sp .RAM_SIZE
	loadi.h sp .RAM_SIZE
	loadi rb ._main
	loadi.h rb ._main
	push rb # store actual return address
	push ra # store the fake ret addr to execute on stack
	ret # jmps to top location on the stack

	# should never get this far
	halt

	# TODO actually call into the loaded code


	# TODO REMOVE
	loadi rb 0
	.main_top:
		mov ra rb
		call.r ._set_led
		call.r ._read_uart
		add rb 1
	#	# high byte first into rb
	#	call.r ._read_uart
	#	mov rb ra
	#	call.r ._read_uart
	#	call.r ._hex_to_number
	#	call.r ._set_led
	jmp.r .main_top
	# TODO REMOVE END
ret

# ====================================================================
# END MAIN
# ====================================================================



# ====================================================================
# OS Library code
# ====================================================================
# _set_led - sets the led states
# INPUT
#   ra - low 8 bits set leds
### VERIFIED ###
._set_led:
	loada    .IO_OFFSET[1]
	store ra .IO_OFFSET[1]
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

# read uart rx - reads a character,
#                waits for a character to be available
# OUTPUT
#   ra - character
### VERIFIED ###
._read_uart:
	call.r ._read_uart_rx_size
	jmp.r.z ._read_uart
	loada   .UART_OFFSET[3]
	load ra .UART_OFFSET[3]
ret

# read uart rx size - reads number of characters waiting in rx
# OUTPUT
#   ra - number
### VERIFIED ###
._read_uart_rx_size:
	loada   .UART_OFFSET[1]
	load ra .UART_OFFSET[1]
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

# _put_string
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

# put_line - same as _put_string but adds a new line
# INPUT
#   ra - string to put on a line
.constant 0x0A NEW_LINE_CHAR
.put_line:
	call.r ._put_string
	loadi ra .NEW_LINE_CHAR
	call.r ._write_uart
ret

# _read_hex_byte - read the next two hex ascii characters and converts to number
# OUTPUT
#   ra - the number
._read_hex_byte:
push rb
	call.r ._read_uart
	mov rb ra
	call.r ._read_uart
	call.r ._hex_to_number
pop rb
ret

# _load_srec_uart - reads uart and parses SREC
# OUTPUT
#   ra - first address or -1 on error
._load_srec_uart:
push rb
push rp
loadi ra 0
push ra
push ra
push ra
push ra
push ra
# sp + 7 = return address
# sp + 6 = saved rb
# sp + 5 = saved rp
# sp + 4 = temp value for combining low and high bytes
# sp + 3 = remaining bytes to read on this line (default 0)
# sp + 2 = running cksum value (default 0)
# sp + 1 = number of records received (default 0)
# sp + 0 = address of first data (default 0)

	.load_srec_uart_start_of_rec:
	# Read until we get an "S"
	call.r ._read_uart
	sub ra, 0x53 # "S"
	jmp.r.nz .load_srec_uart_start_of_rec

	# We got an "S", now get the record type
	# If this is 1, then this is data read to the given address
	# If this is a 5, then this is a confirmation of the number of records
	# If this is a 9, then this is an end transmission
	# If this is anything else, don't care, read out the data, but ignore
	call.r ._read_uart
	sub ra, 0x30 # this is an ascii number so sub off the offset
	mov rb ra # backup the value in rb
	sub ra, 1
	jmp.r.z .load_srec_uart_handle_data
	mov ra, rb
	sub ra, 5
	jmp.r.z .load_srec_uart_handle_number_of_rec
	mov ra, rb
	sub ra, 9
	jmp.r.z .load_srec_uart_handle_end_of_transmission

	########### Fall through, anything else we don't care
	# this will go back to keep reading till an 'S' is found
	jmp.r .load_srec_uart_start_of_rec

	############ Handle data line
	.load_srec_uart_handle_data:
	call.r ._read_hex_byte
	# ra is now size of line in bytes
	store.sp ra 3 # store in remaining_bytes
	store.sp ra 2 # start checksum with this value

	# sub 2 bytes from the remaing for the address
	load.sp ra 3
	sub ra 2
	store.sp ra 3
	call.r ._read_hex_byte  # high byte of address
	mov rp ra
	shftl rp 8
	# add to the cksum total
	load.sp rb 2 # load cksum
	add ra rb
	store.sp ra 2 # store cksum

	call.r ._read_hex_byte  # low byte of address
	or rp ra
	shftr rp 1 # divide rp by 2 since we're using 16 bit words
	# add to the cksum total
	load.sp rb 2
	add ra rb
	store.sp ra 2
	# RP is now the address we want to start writing to

	# if this is the first data line, we need to store the address
	load.sp ra 0
	jmp.r.nz .load_srec_uart_read_data_loop
	mov ra rp
	store.sp ra 0

	# NOTE: THIS ASSUMES THERE IS AT LEAST 2 DATA BYTES!
	.load_srec_uart_read_data_loop:
		# do {
		#   sub 2 count
		#   read hex
		#   update cksum
		#   shift
		#   read hex
		#   update cksum
		#   write value to location
		#   increment ptr for next loop
		# } while (count != 1)
		load.sp ra 3
		sub ra 2
		store.sp ra 3

		call.r ._read_hex_byte
		load.sp rb 2
		add rb ra
		store.sp rb 2

		shftl ra 8  # we always get the high byte first
		store.sp ra 4 # store to temp location

		call.r ._read_hex_byte
		load.sp rb 2
		add rb ra
		store.sp rb 2

		load.sp rb 4 # load back the high half
		or ra rb

		store.rp ra 0 # actually write the number
		add rp 1 # increment rp for next loop

		load.sp ra 3 # load remaining count
		sub ra 1 # we expect 1 to remain for the cksum
	jmp.r.nz .load_srec_uart_read_data_loop

	# handle the cksum
	load.sp rb 2
	and rb 0xFF
	xor rb 0xFF

	call.r ._read_hex_byte
	sub ra rb # ra will be zero on success

	# add one to the number of lines received
	load.sp rb 1
	add rb 1
	store.sp rb 1

	jmp.r.nz .load_srec_uart_bad_exit # exit on error
	jmp.r .load_srec_uart_start_of_rec # else wait for another line

	########### Handle S5 record and verify number of received records is correct
	.load_srec_uart_handle_number_of_rec:
	# for now just go back to the beginning and wait for 'S'
	jmp.r .load_srec_uart_start_of_rec

	###########
	.load_srec_uart_handle_end_of_transmission:
	load.sp ra, 0 # load the start address to be returned
	jmp.r .load_srec_uart_good_exit

.load_srec_uart_bad_exit:
loadi ra 0
sub ra 1  # return 0xFFFF (-1) on failure
.load_srec_uart_good_exit:
add sp, 5 # correspond to push ra's in preamble
pop rp
pop rb
ret

# hex_to_number - converts 2 digit ascii hex to number
# INPUT
#   ra - the LOW byte
#   rb - the HIGH byte
# OUTPUT
#   ra - the resulting number (0x00 .. 0xFF)
### VERIFIED ###
._hex_to_number:
push ra
push rb
# sp + 2 = return address
# sp + 1 = LOW byte
# sp + 0 = HIGH byte

	# convert low byte
	load.sp ra 1
	call.r ._char_to_number
	mov rb ra

	# convert high byte
	load.sp ra 0
	call.r ._char_to_number
	shftl ra 4 # make room for low part
	or ra rb

add sp 2
ret

# _char_to_number - converts a single hex char to a number
# INPUT
#   ra - char
# OUTPUT
#   ra - resulting number (0x00 .. 0x0F)
### VERIFIED ###
._char_to_number:
push rb

	mov rb ra # backup
	sub ra 0x3A # if lower than this then it is a hex digit, else alpha
	jmp.r.gz .char_to_number_handle_alpha
	# else digitA
	mov ra, rb
	sub ra, 0x30
	jmp.r .char_to_number_done

	.char_to_number_handle_alpha:
	mov ra, rb
	sub ra, 0x37 # offset of 'A'
	# fall through to done

.char_to_number_done:
pop rb
ret

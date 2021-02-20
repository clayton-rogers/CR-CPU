.extern __read_uart

# _load_srec_uart - reads uart and parses SREC
# OUTPUT
#   ra - first address or -1 on error
.__load_srec_uart:
.export __load_srec_uart
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
	loada .__read_uart
	call .__read_uart
	sub ra, 0x53 # "S"
	jmp.r.nz .load_srec_uart_start_of_rec

	# We got an "S", now get the record type
	# If this is 1, then this is data read to the given address
	# If this is a 5, then this is a confirmation of the number of records
	# If this is a 9, then this is an end transmission
	# If this is anything else, don't care, read out the data, but ignore
	loada .__read_uart
	call .__read_uart
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

# _read_hex_byte - read the next two hex ascii characters and converts to number
# OUTPUT
#   ra - the number
._read_hex_byte:
push rb
# sp + 1 = __return_address
# sp + 0 = __saved rb
	loada .__read_uart
	call .__read_uart
	call.r ._char_to_number
	shftl ra 4 # make room for low part
	mov rb ra
	loada .__read_uart
	call .__read_uart
	call.r ._char_to_number
	or ra rb

pop rb
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
	mov ra rb
	sub ra 0x30
	jmp.r .char_to_number_done

	.char_to_number_handle_alpha:
	mov ra rb
	sub ra 0x37 # offset of 'A'
	# fall through to done

.char_to_number_done:
pop rb
ret

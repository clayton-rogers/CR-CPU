.extern __read_uart

#############################
# int __load_srec_uart(void)
#############################
# Returns: The first address written to it is assumed this is the address to
#          start execution
.__load_srec_uart:
.export __load_srec_uart
push rb
sub sp 6

# sp + 7 = return addr
# sp + 6 = saved rb
# sp + 5 =
# sp + 4 =
# sp + 3 =
# sp + 2 = is_first_line
# sp + 1 = number_of_data_lines
# sp + 0 = exec_address

.constant 0 exec_address
loadi ra 0
store.sp ra .exec_address

.constant 1 number_of_data_lines
store.sp ra .number_of_data_lines

.constant 2 is_first_line
loadi ra 1
store.sp ra .is_first_line



########### READ START OF LINE
.start_line:
# Read until we get an "S"
loada .__read_uart
call .__read_uart
sub ra, 0x53 # "S"
jmp.r.nz .start_line

call.r .init_crc

# We got an "S", now get the record type
# If this is 1, then this is data to write to the given address
# If this is a 5, then this is a confirmation of the number of records
# If this is a 9, then this is an end transmission (note we don't use the given addr)
# If this is anything else, don't care, read out the data, but ignore
loada .__read_uart
call .__read_uart
sub ra, 0x30 # this is an ascii number so sub off the offset
mov rb ra # backup the value in rb
sub ra, 1
jmp.r.z .handle_data
mov ra, rb
sub ra, 5
jmp.r.z .handle_number_of_rec
mov ra, rb
sub ra, 9
jmp.r.z .handle_end_of_transmission

########### Fall through, anything else we don't care
# this will go back to keep reading till an 'S' is found
jmp.r .start_line

########### HANDLE DATA LINE
.handle_data:
call.r .read_hex_byte
call.r .crc_byte
sub ra 3 # 1 byte crc and 2 bytes address
jmp.r.z .start_line # if this is empty line
shftr ra 1 # div 2
mov rb ra # rb will hold the count
call.r .read_word # read addr
mov rp ra

# store exec_address if required
load.sp ra .is_first_line
jmp.r.z .handle_data_top_loop
store.sp rp .exec_address
loadi ra 0
store.sp ra .is_first_line

.handle_data_top_loop:
call.r .read_word
store.rp ra 0
add rp 1
sub rb 1
mov ra rb
jmp.r.z .finished_reading_data_words
jmp.r .handle_data_top_loop
.finished_reading_data_words:
call.r .read_hex_byte
mov rb ra
call.r .get_crc
sub ra rb
jmp.r.nz .bad_exit  # check crc for this line
# if everything went well, add one to the line count
load.sp ra .number_of_data_lines
add ra 1
store.sp ra .number_of_data_lines
jmp.r .start_line

########### HANDLE NUMBER OF REC LINE
.handle_number_of_rec:
call.r .read_hex_byte
call.r .crc_byte
sub ra 0x03
jmp.r.nz .bad_exit # size of S5 rec should always be 3
call.r .read_word # built in crc
# ra is now the current number of records
load.sp rb .number_of_data_lines
sub ra rb
jmp.r.nz .bad_exit # count of previous rec should be the same
call.r .get_crc
mov rb ra
call.r .read_hex_byte
sub ra rb
jmp.r.nz .bad_exit # check CRC on count record
jmp.r .start_line

########### HANDLE END OF TRANSMISSION
.handle_end_of_transmission:
# Don't actually care about the contents of the address,
# just read and discard two words
# i.e. 1 byte size, 2 bytes addr, 1 byte crc
call.r .read_word
call.r .read_word

# Once we're done reading all the extra stuff,
# return the exe addr
load.sp ra .exec_address
jmp.r .exit

.bad_exit:
loadi ra 0xFF
loadi.h ra 0xFF

.exit:
add sp 6
pop rb
ret


.static 1 CRC_state
#############################
# void init_crc (void)
.init_crc:
#############################
push ra
loada .CRC_state
loadi ra 0
store ra .CRC_state
pop ra
ret

#############################
# int crc_byte(int)
# CRC's a byte with the current state,
# then returns that same byte
.crc_byte:
#############################
push rb
loada .CRC_state
load rb .CRC_state
add rb ra
store rb .CRC_state
pop rb
ret

#############################
# int get_crc(void)
.get_crc:
#############################
loada .CRC_state
load ra .CRC_state
and ra 0xFF
xor ra 0xFF
ret

#############################
# int read_word(void)
# reads the next word (16bits) from uart and crc's it
#############################
.read_word:
push rb
call.r .read_hex_byte
call.r .crc_byte
shftl ra 0x08
mov rb ra
call.r .read_hex_byte
call.r .crc_byte
or ra rb
pop rb
ret


#############################
# read_hex_byte - read the next two hex ascii characters and converts to number
# OUTPUT
#   ra - the number
#############################
.read_hex_byte:
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
	jmp.r.ge .char_to_number_handle_alpha
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

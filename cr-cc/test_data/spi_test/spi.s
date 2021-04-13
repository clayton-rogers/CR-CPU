
.main:
#.export main
loadi ra 0xFE
loadi.h ra 0xCA
call.r .print_num
loadi ra 0x0A #LF
loada .__write_uart
call .__write_uart
loadi ra 0x11
call.r .print_num
loadi ra 0x0A #LF
loada .__write_uart
call .__write_uart

call.r .read_spi_id

ret


# MEMORY MAP
# 0x00 = data (read and write)
# 0x01 = data addr
# 0x02 = status (0x01 = start tx/busy)
# 0x03 = transfer_size

.constant 0x8500 spi
.constant 0x009F OPT_READ_MF_ID

.read_spi_id:
loada .spi
loadi ra 0xFF # transfer size, op plus 3 returned bytes
store ra .spi[3]
loadi ra 0 # reset data ptr to 0
store ra .spi[1]
loadi ra .OPT_READ_MF_ID # set the OPT_CODE
store ra .spi[0]
loadi ra 1 # start the transfer
store ra .spi[2]

# wait for operation to complete
.read_top:
	# output dot while waiting
	loadi ra 0x2E #"."
	loada .__write_uart
	call .__write_uart
loada .spi
load ra .spi[2]
jmp.r.nz .read_top

# finally output the code
loadi ra 0x0A #LF
loada .__write_uart
call .__write_uart

loadi ra 0
loada .spi
store ra .spi[1] # reset ptr

# write each of the three expected pieces with a space between each
loada .spi
load ra .spi[0]
call.r .print_num
loadi ra 0x20
loada .__write_uart
call .__write_uart

loada .spi
load ra .spi[0]
call.r .print_num
loadi ra 0x20
loada .__write_uart
call .__write_uart

loada .spi
load ra .spi[0]
call.r .print_num
loadi ra 0x20
loada .__write_uart
call .__write_uart

ret





#################################################
#int print_num(int num);
#    prints a number as hexadecimal
.extern __write_uart
.print_num:
.export print_num
push ra
shftr ra 12
call.r .print_hex
load.sp ra 0
shftr ra 8
and ra 0x0F
call.r .print_hex
load.sp ra 0
shftr ra 4
and ra 0x0F
call.r .print_hex
load.sp ra 0
and ra 0x0F
call.r .print_hex
add sp 1
ret

#################################################
# print_hex(ra) prints a single hex
# if the value is < 10 then add 48 to ascii char
# if the value is >= 10 then sub 10 and add 65 to ascii uppercase
.print_hex:
push ra
sub ra 10  # >= 10
jmp.r.ge ._is_letter
load.sp ra 0
add ra 48
jmp.r ._print_after
._is_letter:
load.sp ra 0
add ra 55
._print_after:
loada .__write_uart
call .__write_uart
add sp 1
ret

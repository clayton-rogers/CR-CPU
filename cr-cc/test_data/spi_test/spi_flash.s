.extern __write_uart

# MEMORY MAP
# 0x00 = data (read and write)
# 0x01 = data addr
# 0x02 = status (0x01 = start tx/busy)
# 0x03 = transfer_size

.constant 0x8500 spi
.constant 0x009F OPT_READ_MF_ID
.constant 0x0003 OPT_READ
.constant 0x0006 OPT_WRITE_EN
.constant 0x0002 OPT_WRITE
.constant 0x0005 OPT_STATUS_BYTE
.constant 0x0020 OPT_ERASE_4K



#######################################
# int spi_flash_read_mf_id(int* id);
# Reads the manufacturer and device id (3 bytes) and places it in three words
# in the provided pointer.
# Returns 0 on success and -1 on failure
.spi_flash_read_mf_id:
.export spi_flash_read_mf_id
mov rp ra
call.r .wait_for_bus_idle

loada .spi
loadi ra 0x04 # transfer size, op plus 3 returned bytes
store ra .spi[3]
loadi ra 0 # reset data ptr to 0
store ra .spi[1]
loadi ra .OPT_READ_MF_ID # set the OPT_CODE
store ra .spi[0]
loadi ra 1 # start the transfer
store ra .spi[2]

call.r .wait_for_bus_idle

# reset data pointer
loadi ra 1
loada .spi[1]
store ra .spi[1]

# read three bytes
load ra .spi[0]
store.rp ra 0
load ra .spi[0]
store.rp ra 1
load ra .spi[0]
store.rp ra 2

loadi ra 0
ret


##########################################################
# int spi_flash_read(int page_addr, int* data, int data_size);
# Reads up to a page of data.
# Each page is 256 bytes, thus the max data size is 128, since the data will
# be packed.
# Returns 0 on success and -1 on failure
.spi_flash_read:
.export spi_flash_read
push rb
push ra
sub sp 5

call.r .wait_for_bus_idle

### same constants used from below
#.constant 8 data_size
# 7 = ret addr
#.constant 6 data_ptr
#.constant 5 page_addr
#.constant 4 data_ptr_end
# 3 =
# ...
# 0 =

load.sp ra .data_size
#load.sp rb .data_ptr # rb still had data_ptr in it
add ra rb
store.sp ra .data_ptr_end


### if the transfer size is too big, then exit
.constant 129 FIRST_BAD_READ_SIZE
load.sp ra .data_size
jmp.r.ge .read_good_data_size
jmp.r .read_bad_exit
.read_good_data_size:
loadi rb .FIRST_BAD_READ_SIZE
sub ra rb
jmp.r.ge .read_bad_exit

### if the page_addr is too large, exit
.constant 0x0B00 FIRST_BAD_PAGE
load.sp ra .page_addr
jmp.r.ge .read_good_page_addr
jmp.r .read_bad_exit
.read_good_page_addr:
loadi rb .FIRST_BAD_PAGE
loadi.h rb .FIRST_BAD_PAGE
sub ra rb
jmp.r.ge .read_bad_exit


### Setup the transfer
load.sp ra .data_size
shftl ra 1 # data size is in words, but spi works in bytes
add ra 4 # 1 opcode, 3 addr
loada .spi
store ra .spi[3]
loadi ra 0
store ra .spi[1] # reset ptr to 0
loadi ra .OPT_READ
store ra .spi[0] # set opcode


### Calculate and set the address
# address is three bytes
# addr = page_addr * 256 + 0x5.0000
# addr = page_addr << 8 + 0x5.0000
# addr_high = page_addr >> 8 + 0x5
# addr_low = page_addr << 8
# The address is pushed into the peripheral MSB first
load.sp ra .page_addr
shftr ra 8
add ra 0x5
store ra .spi[0]

load.sp ra .page_addr
and ra 0xFF
store ra .spi[0]

loadi ra 0 # TODO future, allow intra page offset
store ra .spi[0]


### Perform the transfer
loadi ra 1 # start the transfer
store ra .spi[2]
call.r .wait_for_bus_idle
# reset data pointer
loadi ra 4 # skip opcode and three addr bytes
loada .spi[1]
store ra .spi[1]

### Read out the data from the transfer
load.sp rp .data_ptr

.read_loop_top:
# check end condition
load.sp ra .data_ptr_end
sub ra rp
jmp.r.z .read_after_loop

# read two bytes out of the peripheral
loada .spi[0]
load ra .spi[0]
load rb .spi[0]
shftl ra 8
or ra rb
store.rp ra 0
add rp 1

jmp.r .read_loop_top
.read_after_loop:


loadi ra 0 # return 0 on success
jmp.r .read_exit
.read_bad_exit:
loadi ra 0
sub ra 1
.read_exit:
add sp 7
ret


##########################################################
.wait_for_bus_idle:
push ra
loada .spi
.wait_top:
load ra .spi[2]
jmp.r.nz .wait_top
pop ra
ret



##########################################################
# int spi_flash_write(int page_addr, int* data, int data_size);
# Writes up to a page of data.
# Each page is 256 bytes, thus the max data size is 128, since
# the data will be packed.
# Returns 0 on success and -1 on failure
.spi_flash_write:
.export spi_flash_write
push rb
push ra
sub sp 5

call.r .wait_for_bus_idle

.constant 8 data_size
# 7 = ret addr
.constant 6 data_ptr
.constant 5 page_addr
.constant 4 data_ptr_end
# 3 =
# ...
# 0 =

load.sp ra .data_size
#load.sp rb .data_ptr # rb still had data_ptr in it
add ra rb
store.sp ra .data_ptr_end

### if the transfer size is too big, then exit
#.constant 129 FIRST_BAD_READ_SIZE # defined above
load.sp ra .data_size
jmp.r.ge .write_good_data_size
jmp.r .write_bad_exit
.write_good_data_size:
loadi rb .FIRST_BAD_READ_SIZE
sub ra rb
jmp.r.ge .write_bad_exit

### if the page_addr is too large, exit
#.constant 0x0B00 FIRST_BAD_PAGE # defined above
load.sp ra .page_addr
jmp.r.ge .write_good_page_addr
jmp.r .write_bad_exit
.write_good_page_addr:
loadi rb .FIRST_BAD_PAGE
loadi.h rb .FIRST_BAD_PAGE
sub ra rb
jmp.r.ge .write_bad_exit


### When writing, we first need to set the write enable
loada .spi
loadi ra 1
store ra .spi[3] # tx size = 1
loadi ra 0
store ra .spi[1] # reset data ptr
loadi ra .OPT_WRITE_EN
store ra .spi[0] # enter the opcode
loadi ra 1
store ra .spi[2] # initiate the transfer

call.r .wait_for_bus_idle

### Now do the actual write
# setup the transfer
loada .spi
loadi ra 0
store ra .spi[1] # reset data ptr
# calculate size
load.sp ra .data_size
shftl ra 1 # word to byte conversion
add ra 4 # 1 opcode and 3 address
store ra .spi[3]
# enter the opcode
loadi ra .OPT_WRITE
store ra .spi[0]
# calculate and enter the addr
# address is three bytes
# addr = page_addr * 256 + 0x5.0000
# addr = page_addr << 8 + 0x5.0000
# addr_high = page_addr >> 8 + 0x5
# addr_low = page_addr << 8
# The address is pushed into the peripheral MSB first
load.sp ra .page_addr
shftr ra 8
add ra 0x5
store ra .spi[0]

load.sp ra .page_addr
and ra 0xFF
store ra .spi[0]

loadi ra 0 # TODO future, allow intra page offset
store ra .spi[0]

### Write the data to the peripheral
load.sp rp .data_ptr

.write_loop_top:
# check end condition
load.sp ra .data_ptr_end
sub ra rp
jmp.r.z .write_loop_after

load.rp ra 0
mov rb ra
shftr ra 8  # store high then low
store ra .spi[0]
and rb 0xFF
store rb .spi[0]

add rp 1
jmp.r .write_loop_top
.write_loop_after:

### Perform the transfer
loadi ra 1 # start the transfer
store ra .spi[2]
call.r .wait_for_bus_idle
# Transfer is now complete


loadi ra 0 # return 0 on success
jmp.r .write_exit
.write_bad_exit:
loadi ra 0
sub ra 1
.write_exit:
add sp 7
ret



##########################################################
# int spi_flash_erase(int page_addr);
# Erases 4 pages of data. The least significant 2 bits of the page address
# should be zero.
# Returns 0 on success and -1 on failure
.spi_flash_erase:
.export spi_flash_erase
push rb
push ra
sub sp 5

# 7 = ret addr
# 6 = saved rb
.constant 5 erase_page_addr
# 4
# 0


### Verify page address is multiple of 4
load.sp ra .erase_page_addr
and ra 0x3
jmp.r.nz .erase_bad_exit


### Verify page address is in bounds
#.constant 0x0B00 FIRST_BAD_PAGE # defined above
load.sp ra .erase_page_addr
jmp.r.ge .erase_good_page_addr
jmp.r .erase_bad_exit
.erase_good_page_addr:
loadi rb .FIRST_BAD_PAGE
loadi.h rb .FIRST_BAD_PAGE
sub ra rb
jmp.r.ge .erase_bad_exit


### Turn on WEL
loada .spi
loadi ra 1
store ra .spi[3] # tx size = 1
loadi ra 0
store ra .spi[1] # reset data ptr
loadi ra .OPT_WRITE_EN
store ra .spi[0] # enter the opcode
loadi ra 1
store ra .spi[2] # initiate the transfer

call.r .wait_for_bus_idle


### Send the erase command
loada .spi
loadi ra 4
store ra .spi[3] # tx size = 1 opcode, 3 addr
loadi ra 0
store ra .spi[1] # reset data ptr
loadi ra .OPT_ERASE_4K
store ra .spi[0] # enter the opcode

# calculate and enter the addr
# address is three bytes
# addr = page_addr * 256 + 0x5.0000
# addr = page_addr << 8 + 0x5.0000
# addr_high = page_addr >> 8 + 0x5
# addr_low = page_addr << 8
# The address is pushed into the peripheral MSB first
load.sp ra .erase_page_addr
shftr ra 8
add ra 0x5
store ra .spi[0]

load.sp ra .erase_page_addr
and ra 0xFF
store ra .spi[0]

loadi ra 1
store ra .spi[2] # initiate the transfer



loadi ra 0 # return 0 on success
jmp.r .erase_exit
.erase_bad_exit:
loadi ra 0
sub ra 1
.erase_exit:
add sp 6
pop rb
ret

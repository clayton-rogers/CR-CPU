
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
.constant 0x0005 OPT_READ_STATUS_1



#######################################
# int spi_flash_read_mf_id(int* id);
# Reads the manufacturer and device id (3 bytes) and places it in three words
# in the provided pointer.
# Returns 0 on success and -1 on failure
.spi_flash_read_mf_id:
.export spi_flash_read_mf_id
mov rp ra

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

### same constants used from below
#.constant 8 data_size
# 7 = ret addr
#.constant 6 data_ptr
#.constant 5 page_addr
#.constant 4 data_ptr_end
# 3 =
# ...
# 0 =

### Calculate the end of the array
load.sp ra .data_size
#load.sp rb .data_ptr # rb still had data_ptr in it
add ra rb
store.sp ra .data_ptr_end

### Check data size is valid
load.sp ra .data_size
call.r .is_data_size_valid
jmp.r.z .read_bad_exit

### Check page addr is valid
load.sp ra .page_addr
call.r .is_page_addr_valid
jmp.r.z .read_bad_exit

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
load.sp ra .page_addr
call.r .calculate_and_load_addr


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
# void wait_for_bus_idle(void)
.wait_for_bus_idle:
push ra
loada .spi
.wait_top:
load ra .spi[2]
jmp.r.nz .wait_top
pop ra
ret


##########################################################
# void wait_for_not_busy_flash(void)
.wait_for_not_busy_flash:
push ra

call.r .wait_for_bus_idle


### keep sampling the busy bit until it is zero
.wait_busy_top:
loada .spi
loadi ra 2
store ra .spi[3] # opcode + 1 rx data byte
loadi ra 0
store ra .spi[1] # reset ptr
loadi ra .OPT_READ_STATUS_1
store ra .spi[0]
loadi ra 1
store ra .spi[2] # init transfer

call.r .wait_for_bus_idle

loadi ra 1
store ra .spi[1] # reset ptr
load ra .spi[0]
and ra 0x01 # mask status bit
jmp.r.nz .wait_busy_top

pop ra
ret


##########################################################
# void calculate_and_load_addr(int page_addr)
# Converts the page addr to read addr and loads it into the
# peripheral.
.calculate_and_load_addr:
# address is three bytes
# addr = page_addr * 256 + 0x5.0000
# addr = page_addr << 8 + 0x5.0000
# addr_high = page_addr >> 8 + 0x5
# addr_low = page_addr << 8
# The address is pushed into the peripheral MSB first
push ra

shftr ra 8
add ra 0x5
store ra .spi[0]

load.sp ra 0 # load given page address again
and ra 0xFF
store ra .spi[0]

loadi ra 0 # TODO future, allow intra page offset
store ra .spi[0]

add sp 1
ret

##########################################################
# int is_page_addr_valid(int page_addr)
.is_page_addr_valid:
push rb

.constant 0x0B00 FIRST_BAD_PAGE
#load.sp ra 0 # ra is already loaded
jmp.r.ge .good_page_addr  #check that its positive
jmp.r .bad_page_addr
.good_page_addr:
loadi rb .FIRST_BAD_PAGE
loadi.h rb .FIRST_BAD_PAGE
sub ra rb
jmp.r.ge .bad_page_addr  # if page addr is >= FIRST_BAD_PAGE

loadi ra 1  # everything is good if we got here
jmp.r .page_addr_exit

.bad_page_addr:
loadi ra 0

.page_addr_exit:
pop rb
ret


##########################################################
# int is_data_size_valid(int data_size)
.is_data_size_valid:
push rb

.constant 129 FIRST_BAD_READ_SIZE
# load.sp ra .data_size # ra already loaded with data size
jmp.r.ge .good_data_size
jmp.r .bad_data_size
.good_data_size:
loadi rb .FIRST_BAD_READ_SIZE
sub ra rb
jmp.r.ge .bad_data_size

loadi ra 1 # everything is good if we got here
jmp.r .data_size_exit

.bad_data_size:
loadi ra 0

.data_size_exit:
pop rb
ret


##########################################################
# void set_WEL(void)
.set_WEL:
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

### Check that data size is valid
load.sp ra .data_size
call.r .is_data_size_valid
jmp.r.z .write_bad_exit

### Check page_addr is valid
load.sp ra .page_addr
call.r .is_page_addr_valid
jmp.r.z .write_bad_exit

### When writing, we first need to set the write enable
call.r .set_WEL

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

### Calculate and load address into peripheral
load.sp ra .page_addr
call.r .calculate_and_load_addr


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

# wait for the spi bus to be idle
# then poll the flash busy bit
call.r .wait_for_not_busy_flash
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


### Check page addr is valid
load.sp ra .erase_page_addr
call.r .is_page_addr_valid
jmp.r.z .erase_bad_exit


### Turn on WEL
call.r .set_WEL


### Send the erase command
loada .spi
loadi ra 4
store ra .spi[3] # tx size = 1 opcode, 3 addr
loadi ra 0
store ra .spi[1] # reset data ptr
loadi ra .OPT_ERASE_4K
store ra .spi[0] # enter the opcode

### Calculate and load the address into peripheral
load.sp ra .erase_page_addr
call.r .calculate_and_load_addr

# wait for bus idle then poll flash busy bit
call.r .wait_for_not_busy_flash


loadi ra 0 # return 0 on success
jmp.r .erase_exit
.erase_bad_exit:
loadi ra 0
sub ra 1
.erase_exit:
add sp 6
pop rb
ret

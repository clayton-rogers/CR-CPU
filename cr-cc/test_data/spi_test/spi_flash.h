///////////////////////////////////////////////////////////////////////////////
//
// The SPI flash attached to the TinyFPGA BX is 8mbit, i.e. 1MB. The valid
// addresses are thus 0x00.0000 to 0x10.0000. However the user data starts at
// 0x5.0000. Since this is still larger that what will fit in a (16bit) int,
// we must decide whether to perform addressing with (32bit) long, or something
// else.
//
// Since up to 256 bytes may be written at a time, we will use that as our page
// size. Thus we will have (0x10.0000 - 0x5.0000)/256 = 0xB00 (2816) pages which
// physically start at byte address 0x5.0000.
//
// Note that since the minimum erase size is 4KB, four pages will always be
// erased at a time.


///////////////////////////////////////////////////////////////////////////////
int spi_flash_read_mf_id(int* id);
// Reads the manufacturer and device id (3 bytes) and places it in three words
// in the provided pointer.
// Returns 0 on success and -1 on failure


///////////////////////////////////////////////////////////////////////////////
int spi_flash_read(int page_addr, int* data, int data_size);
// Reads up to a page of data.
// Each page is 256 bytes, thus the max data size is 128, since the data will
// be packed.
// Returns 0 on success and -1 on failure


///////////////////////////////////////////////////////////////////////////////
int spi_flash_write(int page_addr, int* data, int data_size);
// Writes up to a page of data.
// Each page is 256 bytes, thus the max data size is 128, since the data will
// be packed.
// Returns 0 on success and -1 on failure

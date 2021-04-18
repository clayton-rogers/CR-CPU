#include "spi_flash.h"
#include <cr-os.h>

int print_num(int); // defined in spi.s

int data[128];

int test_mf() {
	int id[3];
	int ret;


	ret = spi_flash_read_mf_id(id);
	if (ret != 0) return 1;
	if (id[0] != 0x1F) return 2;
	if (id[1] != 0x85) return 3;
	if (id[2] != 0x01) return 4;

	return 0;
}

int test_read() {
	int ret;

	ret = spi_flash_read(0, &data, 1);
	if (ret != 0) return 1;
	if (data[0] != 0xFFFF) return 7;

	ret = spi_flash_read(0, &data, 128);
	if (ret != 0) return 2;

	// size upper
	ret = spi_flash_read(0, &data, 129);
	if (ret != -1) return 3;

	ret = spi_flash_read(0, &data, 130);
	if (ret != -1) return 4;

	// size lower
	ret = spi_flash_read(0, &data, -1);
	if (ret != -1) return 5;

	ret = spi_flash_read(0, &data, -1000);
	if (ret != -1) return 6;

	// page lower
	ret = spi_flash_read(-1, &data, 1);
	if (ret != -1) return 8;

	// page upper
	ret = spi_flash_read(0xAFF, &data, 1);
	if (ret != 0) return 9;
	if (data[0] != 0xFFFF) return 10;

	ret = spi_flash_read(0xB00, &data, 1);
	if (ret != -1) return 11;

	ret = spi_flash_read(0xB01, &data, 1);
	if (ret != -1) return 11;

	return 0;
}

int test_write() {
	int ret;

	data[0] = 0xABCD;
	data[1] = 0xEF12;
	data[2] = 3213;
	data[3] = 9513;

	ret = spi_flash_write(0, &data, 4);
	if (ret != 0) return 1;
	for (int i = 0; i < 4; ++i) {
		data[i] = 0;
	}
	ret = spi_flash_read(0, &data, 4);
	if (ret != 0) return 2;
	if (data[0] != 0xABCD) return 3;
	if (data[1] != 0xEF12) return 4;
	if (data[2] != 3213) return 5;
	if (data[3] != 9513) return 6;

	ret = spi_flash_write(1, &data, 128);
	if (ret != 0) return 7;

	// size upper
	ret = spi_flash_write(0, &data, 129);
	if (ret != -1) return 8;

	ret = spi_flash_write(0, &data, 130);
	if (ret != -1) return 9;

	// size lower
	ret = spi_flash_write(0, &data, -1);
	if (ret != -1) return 10;

	ret = spi_flash_write(0, &data, -100);
	if (ret != -1) return 11;

	// page lower
	ret = spi_flash_write(-1, &data, 1);
	if (ret != -1) return 12;

	// page upper
	data[0] = 0xCAFE;
	ret = spi_flash_write(0xAFF, &data, 1);
	if (ret != 0) return 13;
	data[0] = 0;
	ret = spi_flash_read(0xAFF, &data, 1);
	if (ret != 0) return 14;
	if (data[0] != 0xCAFE) return 15;

	ret = spi_flash_write(0xB00, &data, 1);
	if (ret != -1) return 16;

	ret = spi_flash_write(0xB01, &data, 1);
	if (ret != -1) return 17;

	return 0;
}

int test() {

	int mf = test_mf();
	if (mf != 0) return 0x100 + mf;

	int read = test_read();
	if (read != 0) return 0x200 + read;

	int write = test_write();
	if (write != 0) return 0x300 + write;


	return 0;
}

int main() {
	int id[3];

	spi_flash_read_mf_id(id);

	for (int i = 0; i < 3; ++i) {
		print_num(id[i]);
		__write_uart(0x20);
	}

	data[0] = 0xcafe;

	int ret = spi_flash_read(0, &data, 1);

	print_num(ret);
	__write_uart(0x20);

	print_num(data[0]);
	__write_uart(0x20);

	return test();
}

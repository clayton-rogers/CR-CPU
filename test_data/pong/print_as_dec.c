#include <vga.h>

int print_as_dec(int value) {
	int last_digit = value % 10;
	int new_value = value / 10;
	if (new_value) {
		print_as_dec(new_value);
	}

	last_digit = last_digit + 48;

	write_vga_char(last_digit);
}

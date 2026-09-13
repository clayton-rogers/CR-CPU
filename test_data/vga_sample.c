
#include "vga.h"

int main() {
	clear_screen();

	set_vga_cursor(80);
	for (int i = 65; i <= 122; i = i + 1) {
		write_vga_char(i);
	}

	set_vga_cursor(160);
	for (int i = 65; i <= 122; i = i + 1) {
		write_vga_char(i);
	}

}
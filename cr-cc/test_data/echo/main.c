#include <cr-os.h>
#include <vga.h>
#include <unistd.h>
#include <print_num.h>

// Echo's the uart key presses to the screen.
// Also prints the key code

// ^x = clear screen
// ^c = exit

int main() {
	clear_screen();

	int character = __read_uart();
	int count = 0;
	int line = 0;

	while (character != 0x3) {
		if (character == 0x18) { // ctrl-x (cancel)
			clear_screen();
			count = 0;
			line = 0;
		} else {
			write_vga_char(character);
			write_vga_char(' ');
			__print_num(character);
			write_vga_char(' ');

			// To keep the lines a multiple of 80
			count = count + 1;
			if (count == 11) {
				write_vga_char(' ');
				write_vga_char(' ');
				write_vga_char(' ');
				count = 0;
				line = line + 1;
				if (line == 24) {
					set_vga_cursor(0);
					line = 0;
				}
			}
		}
		character = __read_uart();

	}
}
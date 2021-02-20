
#include <cr-os.h>
#include <vga.h>

int main() {
	clear_screen();
	for (int i = 0; i < 0x100; i = i + 1) {
		__set_led(i);
		msleep(10);
	}
	for (int i = 0; i < 16; i = i + 1) {
		__set_led(i);
		msleep(250);
	}
	for (int i = 0; i < 16; i = i + 1) {
		__set_led(i << 4);
		msleep(250);
	}
	__set_led(0xFF);
	msleep(750);
	__set_led(0xCA);
	msleep(750);
	__set_led(0x22);
	msleep(750);
	__set_led(0x55);
	msleep(750);
	__set_led(0x88);
	msleep(750);

	return 0x23;
}

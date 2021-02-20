
#include <unistd.h>
#include <gpio.h>

int main() {
	for (int j = 0; j < 30; j = j + 1) {
		int a = 1;

		for (int i = 0; i < 8; i = i + 1) {
			__set_led(a);
			a = a << 1;
			msleep(45);
		}
	}
}

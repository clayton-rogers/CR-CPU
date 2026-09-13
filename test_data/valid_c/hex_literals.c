// ret: 12

int array[0xf];
int array_2[0xFF];

int main() {
	int a = 0xFF;
	int b = 255;

	if (a != b) return 0;

	a = 0XF; // deliberate capital X
	b = 15;

	if (a != b) return 0;

	a = 0xFFFF;
	b = 65535;

	if (a != b) return 0;

	a = 0xacac;
	b = 44204;

	if (a != b) return 0;

	return 12;
}

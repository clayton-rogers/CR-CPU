// ret: 1
int main() {

	// basic case with on bit
	if (1 >> 1 != 0) {
		return 2;
	}

	// another basic case
	if (2 >> 1 != 1) {
		return 3;
	}

	// verify this is a logical shift: 0x8000 -> 0x4000
	if (32768 >> 1 != 16384) {
		return 4;
	}

	// verify multiple shift off: 0x01FF -> 0x0001
	if (511 >> 8 != 1) {
		return 5;
	}

	// again verify multiple shiff off: 0xFFFF -> 0x00FF
	if (65535 >> 8 != 255) {
		return 6;
	}

	// shift one bit all the way across: 0x8000 -> 0x0001
	if (32768 >> 15 != 1) {
		return 7;
	}

	// again: 0xFFFF -> 0x0000
	if (65535 >> 16 != 0) {
		return 8;
	}

	// test precedence
	// addition is higher
	// ne is lower
	if (2 + 2 >> 1 + 1 != 2 >> 1) {
		return 9;
	}

	return 1;
}


// ret: 1
int main() {
	// basic one bit
	if (1 << 1 != 2) {
		return 2;
	}

	// basic two bits
	if (3 << 1 != 6) {
		return 3;
	}

	// ensure bits can be shifted off the top: 0xFFFF -> 0xFFFE
	if (65535 << 1 != 65534) {
		return 4;
	}

	// if only top bit is set, then it is shifted off: 0x8000 -> 0x0000
	if (32768 << 1 != 0) {
		return 5;
	}

	// shift more than one spot
	if (1 << 2 != 4) {
		return 6;
	}

	// shift multiple bits more than one spot
	if (3 << 3 != 24) {
		return 7;
	}

	// shift all of the top bits off: 0xFF01 -> 0x0100
	if (65281 << 8 != 256) {
		return 8;
	}

	// shift full range: 0x0001 -> 0x8000
	if (1 << 15 != 32768) {
		return 9;
	}

	// test precedence
	// addition is higher
	// ne is lower
	if ( 1 + 1 << 1 + 1 != 1 << 3) {
		return 10;
	}

	return 1;
}

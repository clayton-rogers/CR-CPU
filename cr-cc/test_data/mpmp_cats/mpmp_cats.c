

int true = 1;
int false = 0;
int MAX_WIDTH = 16;
int width = 0;

// From ASM
int check_bits(int bits, int value);


int get_max_from_width(int width) {
	if (width > MAX_WIDTH) {
		return 0;
	}

	if (width == 16) {
		return 65535;
	}

	return (1 << width) - 1;
}

int test_get_max_from_width() {
	if (!(get_max_from_width(1) == 1)) return 1;
	if (!(get_max_from_width(2) == 3)) return 2;
	if (!(get_max_from_width(3) == 7)) return 3;
	if (!(get_max_from_width(4) == 15)) return 4;
	if (!(get_max_from_width(5) == 31)) return 5;
	if (!(get_max_from_width(6) == 63)) return 6;
	if (!(get_max_from_width(8) == 255)) return 8;
	if (!(get_max_from_width(10) == 1023)) return 10;
	return 0;
}

int is_valid_config(int bits) {

	for (int i = 0; i < (width-1); i = i + 1) {
		int check_val = 3 << i;

		if (check_bits(bits, check_val)) {
		//if ((bits & check_val) == check_val) {
			return false;
		}
	}

	return true;
}

int test_is_valid_config() {
	width = 10;

	if (!is_valid_config(0x01)) return 1;
	if (!is_valid_config(0x02)) return 2;
	if (is_valid_config(0x03)) return 3;
	if (!is_valid_config(0x04)) return 4;
	if (!is_valid_config(0x05)) return 5;
	if (is_valid_config(0x06)) return 6;
	if (is_valid_config(0x07)) return 7;
	if (!is_valid_config(0x08)) return 8;
	if (!is_valid_config(0x09)) return 9;
	if (!is_valid_config(0x0A)) return 10;
	if (is_valid_config(0x0B)) return 11;
	if (is_valid_config(0x0C)) return 12;
	if (is_valid_config(0x0D)) return 13;
	if (is_valid_config(0x0E)) return 14;
	if (is_valid_config(0x0F)) return 15;

	if (is_valid_config(0x18)) return 0x18;

	return 0;
}

int count_configurations() {
	if (width > MAX_WIDTH) {
		return 0;
	}

	int max_value = get_max_from_width(width);
	int value = 0;
	int count = 0;

	while (value < max_value) {
		if (is_valid_config(value)) {
			count = count + 1;
		}

		value = value + 1;
	}

	return count;
}







int main() {
	//return test_get_max_from_width();
	//return test_is_valid_config();

	width = 10;
	int results = count_configurations();

	return results;
}
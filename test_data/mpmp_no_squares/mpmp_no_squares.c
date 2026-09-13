
#include "vga.h"

int get_width();
int get_size();

int set_value(int pos, int value);
int get_value(int pos);
int check_good();

int and(int, int);

int check_good() {
	int square_size = 2;
	int width = get_width();
	int size = get_size();
	while (1) {
		// Check regular squares
		int num_pos = width - square_size + 1;
		for (int i = 0; i < num_pos; i = i + 1) {
			for (int j = 0; j < num_pos; j = j + 1) {
				int position = i + j * width;
				int count = 0;
				if (get_value(position)) {
					count = count + 1;
				}

				if (get_value(position + (square_size-1))) {
					count = count + 1;
				}

				position = position + width;

				if (get_value(position)) {
					count = count + 1;
				}

				if (get_value(position + (square_size-1))) {
					count = count + 1;
				}

				if (count == 4 || count == 0) {
					return 0;
				}
			}
		}

		// Check diamonds:
		// .X.
		// X.X
		// .X.
		num_pos = width - square_size + 2; // diamonds are one wider than square_size
		for (int i = 0; i < num_pos; i = i + 1) {
			for (int j = 0; j < num_pos; j = j + 1) {
				int position = i + j * width;
				int count = 0;
				if (get_value(position + 1)) {
					count = count + 1;
				}

				position = position + width; // Next row

				if (get_value(position)) {
					count = count + 1;
				}
				if (get_value(position + 2)) {
					count = count + 1;
				}

				position = position + width; // Next row

				if (get_value(position + 1)) {
					count = count + 1;
				}

				if (count == 4 || count == 0) {
					return 0;
				}
			}
		}

		// Check other angles
		// TODO

		square_size = square_size + 1;
		if (square_size > width) {
			break;
		}
	}

	return 1;
}

int print_puzzle() {
	int size = get_size();
	for (int i = 0; i < size; i = i + 1) {
		int value = get_value(i);
		value = value + 48;
		write_vga_char(value);
	}
	// print a space to separate
	write_vga_char(32);
	return 0;
}

int get_number_of_ones() {
	int size = get_size();
	int count = 0;
	for (int i = 0; i < size; i = i + 1) {
		count = count + get_value(i);
	}
	return count;
}

int print_num(int num);

int next_puzzle() {
	int size = get_size();
	int required_number_ones = size / 2;

	while (1) {
		int residual = 0;

		for (int i = 0; i < size; i = i + 1) {
			int value = get_value(i);
			if (i == 0) {
				value = value + residual + 1;
				if (value == 1) {
					set_value(i, 1);
					residual = 0;
				}
				if (value == 2) {
					set_value(i, 0);
					residual = 1;
				}
				if (value == 3) {
					set_value(i, 1);
					residual = 1;
				}
			} else {
				if (residual == 0) {
					break;
				}
				if (value == 0) {
					set_value(i, 1);
					residual = 0;
				}
				if (value == 1) {
					set_value(i, 0);
					residual = 1;
				}
				// can never be 3 because there is only the bit and carry
				//if (value == 3) {
				//	set_value(i, 1);
				//	residual = 1;
				//}
			}
		}

		int number_ones = get_number_of_ones();
		if (number_ones == required_number_ones) {
			break;
		}
	}
}

int main() {
	clear_screen();
	print_num(13483); // 0x34AB
	write_vga_char(32); // space

	int width = get_width();
	int size = get_size();
	int count = size / 2;

	// Initially setup the square with all the ones at the beginning
	for (int i = 0; i < size; i = i + 1) {
		if (count != 0) {
			set_value(i, 1);
			count = count - 1;
		} else {
			set_value(i, 0);
		}
	}

	int num_loops = 0;
	int outer_count = 0;

	print_puzzle();

	while (1) {

		if (check_good()) {
			clear_screen();
			print_puzzle();
			print_puzzle();
			return 0;
		}

		next_puzzle();
		//print_puzzle();

		num_loops = num_loops + 1;
		if (and(num_loops, 255) == 0) {
			print_num(outer_count);
			print_num(num_loops);
			write_vga_char(32);
		}
		if (32767 == num_loops) {
			num_loops = 0;
			outer_count = outer_count + 1;
		}
		//if (num_loops == 20000) {
		//	break;
		//}
	}

	write_vga_char(32);
	write_vga_char(32);
	write_vga_char(32);
	print_num(num_loops);
	write_vga_char(32);
	return num_loops;
}

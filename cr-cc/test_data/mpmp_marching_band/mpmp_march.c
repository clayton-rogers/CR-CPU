#include "print_num.h"
#include "vga.h"
#include "timer.h"
#include "mpmp_alt.h"


int mult_two(int);
int calc_number_of_arrangements_asm(int num_member);

int get_max_arrangements();
int set_soln(int num_arrangements, int num_members);
int get_soln(int num_arrangements);


// This is a brute force solution to
// Matt Parkers Maths Puzzles
// https://www.think-maths.co.uk/marchingband

int output_soln(int i, int j) {
	// TODO
	return 0;
}

int calc_number_of_arrangements(int num_member) {
	int number_arrangements = 0;

	for (int i = 1; i <= num_member; i = i + 1) {
		for (int j = i; j <= num_member; j = j + 1) {
			if (i * j == num_member) {
				//output_soln(i, j);
				number_arrangements = number_arrangements + 1;
			}
		}
	}

	return mult_two(number_arrangements);
}

int check() {

	for (int i = 6; i < 30; i = i + 1) {
		int c = calc_number_of_arrangements(i);
		int asm = calc_number_of_arrangements_asm(i);
		int alt = calc_number_of_arrangements_alt(i);
		if (c != asm || c != alt) {
			print_num(c);
			write_vga_char(32);
			print_num(asm);
			write_vga_char(32);
			print_num(alt);
			write_vga_char(32);
			print_num(i);
			write_vga_char(32);
			return 0;
		}
	}
	int a = calc_number_of_arrangements(24);
	if (a == 8) {
		return 1;
	} else {
		return 0;
	}
}

// currently takes:
// 62850518 - C
// 46200976 - asm
// 32433280 - asm with only looking at half of i items

// 4 to 22:
// 294835439 - asm
//   5539037 - alt
//   5825508 - alt with fixed sqrt
//   5807527 - alt with second fixed sqrt
//   5793212 - more fixed
//   5489233 - more fixed
//   5486758 - more fixed

// 4 to 64:
// 2 654 890 420 - alt
// 2 458 095 951 - alt with fix sqrt
// 2 643 884 111 - alt with more fixed sqrt (????)
//   243 592 173 - find all arrangements
int find_arrangement(int required) {
	int result = 0;
	int i = 5;
	while (1) {
		i = i + 1;
		int a = calc_number_of_arrangements_alt(i);
		if (a == required) {
			result = i;
			break;
		}
		if (i == 8000) { break; } // prevent infinite loop
	}
	write_vga_char(45);
	print_num(required);
	write_vga_char(45);
	print_num(result);
	write_vga_char(32);

	return result;
}

int find_all_arrangements_up_to(int n) {
	int MAX = get_max_arrangements();
	int count = 0;
	for (int i = 2; i < n; i = i + 1) {
		// i is the band size
		int num_arrangements = calc_number_of_arrangements_alt(i);
		if (num_arrangements < MAX) {
			// this will only set the solution the first time
			// after that it's a noop
			set_soln(num_arrangements, i);
		}
		count = count + 1;
		if (count == 100) {
			count = 0;
			write_vga_char(46); // period
		}
	}
}

// prints all the solutions in the format
// -AAAA-SSSS
// where A is the number of arrangements
// and S is the number of band members required
int print_all_arrangments() {
	int MAX = get_max_arrangements();
	for (int i = 4; i < MAX; i = i + 2) {
		write_vga_char(45);
		print_num(i);
		write_vga_char(45);
		print_num(get_soln(i));
		write_vga_char(32);
	}
}

int print_factors() {
	int num = 7560; // soln !!!!
	int max = 86; // == sqrt(n);

	for (int i = 1; i <= max; i = i + 1) {
		if (num % i == 0) {
			print_num(i);
			write_vga_char(32);
		}
	}
}

int main() {
	clear_screen();
	if (!check()) {
		return 1;
	}
	//print_factors();
	//return 0;

	print_num(get_timer_upper());
	print_num(get_timer_lower());
	write_vga_char(32);

	find_all_arrangements_up_to(8000); // same value used in find_arrangements
	print_all_arrangments();

	//for (int i = 4; i < 65; i = i + 2) {
	//	find_arrangement(i);
	//}

	write_vga_char(32);
	print_num(get_timer_upper());
	print_num(get_timer_lower());

	return 0;
}


#include "math.h"

int calc_number_of_arrangements_alt(int num_member) {
	int sqrt_num_member = sqrt(num_member);
	int num_arrangements = 1; // always will be at least 1 by X

	for (int i = 2; i <= sqrt_num_member; i = i + 1) {
		if (num_member % i == 0) {
			num_arrangements = num_arrangements + 1;
		}
	}

	return num_arrangements * 2;
}

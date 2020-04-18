// ret: 1
// whole program should return true on success

int no_arg() {
	return 1;
}

int one_arg(int foo) {
	return foo + 2;
}

int two_arg(int bar, int foo) {
	return bar + foo + 4;
}

int three_arg(int a, int b, int arg) {
	return a + b + arg + 8;
}

int four_arg(int a, int i, int j, int ewq) {
	return a + i + j + ewq + 16;
}

int unused_arg(int, int, int, int) {
	return 32;
}

int also_unused(int a, int, int, int) {
	return a + 64;
}

int more_unused(int, int, int, int number) {
	return 128 + number;
}

int main() {
	int ret_val = 1;
	
	if (1 != no_arg()) {
		ret_val = 0;
	}
	
	if (102 != one_arg(100)) {
		ret_val = 0;
	}
	
	if (114 != two_arg(100, 10)) {
		ret_val = 0;
	}
	
	if (1118 != three_arg(1000, 100, 10)) {
		ret_val = 0;
	}
	
	if (11126 != four_arg(10000, 1000, 100, 10)) {
		ret_val = 0;
	}
	
	if (32 != unused_arg(1, 2, 4, 8)) {
		ret_val = 0;
	}
	
	if (164 != also_unused(100, 2, 4, 8)) {
		ret_val = 0;
	}
	
	if (1128 != more_unused(2, 4, 8, 1000)) {
		ret_val = 0;
	}
	
	// duplicate all calls to ensure it doesn't just work the first time
	if (1 != no_arg()) {
		ret_val = 0;
	}
	
	if (102 != one_arg(100)) {
		ret_val = 0;
	}
	
	if (114 != two_arg(100, 10)) {
		ret_val = 0;
	}
	
	if (1118 != three_arg(1000, 100, 10)) {
		ret_val = 0;
	}
	
	if (11126 != four_arg(10000, 1000, 100, 10)) {
		ret_val = 0;
	}
	
	if (32 != unused_arg(1, 2, 4, 8)) {
		ret_val = 0;
	}
	
	if (164 != also_unused(100, 2, 4, 8)) {
		ret_val = 0;
	}
	
	if (1128 != more_unused(2, 4, 8, 1000)) {
		ret_val = 0;
	}
	
	return ret_val; // if we got this far then we're good;
}
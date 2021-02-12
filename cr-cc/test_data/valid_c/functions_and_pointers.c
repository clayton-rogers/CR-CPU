// ret: 12

int * global;
int data = 11;

int * get_ptr() {
	return global;
}

int test_1() {
	global = &data;

	int * p = get_ptr();
	*p = *p + 1;

	return data == 12;
}

int array[10];

int* inc_pointer(int* input) {
	return input + 1;
}

int test_2() {
	array[0] = 21;
	array[1] = 22;
	array[2] = 23;

	int * a = array;
	int * b = inc_pointer(a);
	return *b == 22;
}

int main() {
	if (!test_1()) {
		return 0;
	}
	if (!test_2()) {
		return 0;
	}

	return 12;
}
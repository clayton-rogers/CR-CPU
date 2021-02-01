// ret: 2


int simple_array() {
	int a[2];

	a[0] = 10;
	a[1] = 12;

	return a[0] == 10 && a[1] == 12;
}

int array_complex_index(int index) {
	int a[3];
	a[0] = 11;
	a[1] = 12;
	a[2] = 13;

	return a[index];
}

int array_complex_index_2() {
	int a[3];
	a[0] = 14;
	a[1] = 15;
	a[2] = 16;

	int b = a[0 + 3 - 1] + a[7-1-5];

	return b == (16 + 15);
}

int array_loop() {
	int size = 10;
	int a[10];

	for (int i = 0; i < size; i = i + 1) {
		a[i] = 32;
	}

	int total = 0;
	for (int i = 0; i < size; i = i + 1) {
		total = total + a[i];
	}

	return total == 320;
}

int static_array[10];

int load_static_array() {
	for (int i = 0; i < 10; i = i + 1) {
		static_array[i] = 22;
	}
	return 0;
}
int deref_static_array() {
	int a = 0;
	a = a + static_array[0];
	a = a + static_array[1];
	a = a + static_array[9];

	return a == 66;
}

int pointers_to_array() {
	int size = 10;
	int a[10];

	// load the array
	for (int i = 0; i < size; i = i + 1) {
		a[i] = 32 + i;
	}

	// TODO this is actually a quirk,
	// arrays references should decay
	// into pointer to first member,
	// but currently you have to manually,
	// take the ref.
	int * b = a;
	b = b + 5;

	return *b == 37;
}

int main() {

	if (!simple_array()) {
		return 1;
	}
	if (array_complex_index(1) != 12) {
		return 1;
	}
	if (array_complex_index(2) != 13) {
		return 1;
	}
	if (array_complex_index(0) != 11) {
		return 1;
	}
	if (!array_complex_index_2()) {
		return 1;
	}
	if (!array_loop()) {
		return 1;
	}

	load_static_array();
	if (!deref_static_array()) {
		return 1;
	}

	if (!pointers_to_array()) {
		return 1;
	}

	return 2;
}

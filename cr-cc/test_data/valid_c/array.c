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

	return 2;
}


// wtf how does the assignment work?
// the assignment expression doesn't seem to account for arrays...
// do array declarations even work?

// pretty sure it doesn't....
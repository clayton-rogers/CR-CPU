// ret: 12


int basic_inc() {
	int a = 10;
	++a;
	return a == 11;
}

int basic_dec() {
	int a = 10;
	--a;
	return a == 9;
}

int assign_inc() {
	int a = 10;
	int b;
	b = ++a;

	return b == a && a == 11 && b == 11;
}

int assign_dec() {
	int a = 10;
	int b;
	b = --a;

	return b == a && a == 9 && b == 9;
}

int for_loop() {
	int a = 0;
	for (int i = 0; i < 5; ++i) {
		a = a + i;
	}

	return a == 10;
}

//int inc_array_val() {
//	int a[10];
//	a[0] = 10;
//	++a[0];
//
//	return a[0] == 11;
//}

int main() {

	if (!basic_inc()) { return 0; }
	if (!basic_dec()) { return 1; }
	if (!assign_inc()) { return 2; }
	if (!assign_dec()) { return 3; }
	if (!for_loop()) { return 4; }
	// TODO doesn't work yet
	//if (!inc_array_val()) { return 5; }

	return 12;
}

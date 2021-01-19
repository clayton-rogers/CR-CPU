// ret: 2


int simple_array() {
	int a[2];

	a[0] = 10;
	a[1] = 12;

	return a[0] == 10 && a[1] == 12;
}

int test() {
	int a[2];
	a[0] = 11;
	a[1] = 12;

	return a[0];
}

int main() {
	//if (!simple_array()) {
	//	return 1;
	//}

	if (test() != 11) {
		return test();
	}

	return 2;
}


// wtf how does the assignment work?
// the assignment expression doesn't seem to account for arrays...
// do array declarations even work?

// pretty sure it doesn't....
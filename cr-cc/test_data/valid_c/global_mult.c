

int num = 11;

int add_one() {
	num = num + 1;
}

int add_ten() {
	num = num + 10;
}

int get_num() {
	return num;
}

// ret: 0
int main() {
	add_one();
	add_one();
	add_ten();
	add_ten();

	int val = get_num();
	if (val != 33) {
		return 1;
	}

	return 0;
}
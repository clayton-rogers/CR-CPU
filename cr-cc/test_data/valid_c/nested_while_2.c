// ret:1

int test1() {
	int count = 0;
	while (1) {
			for (int i = 0; i < 2; i = i + 1) {
					for (int j = 0; j < 2; j = j + 1) {
							// TODO
							count = count + 1;
					}
			}
			break;
	}
	return count;
}

int test2() {
	int outer_ans;
	while (1) {
		int ans = 0;
		int a = 4;
		for (int i = 0; i < 10; i = i + 1)
			for (int j = 0; j < 10; j = j + 1)
				if ( a == i)
					break;
				else
					ans = ans + i;

		outer_ans = ans;
		break;
	}

	return outer_ans;
}

int main() {
	int res1 = test1();
	int res2 = test2();

	if (res1 != 4) {
		return 0;
	}
	if (res2 != 410) {
		return 0;
	}

	return 1;
}

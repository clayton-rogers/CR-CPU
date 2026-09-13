// ret:12
// Tests character literals as well as all types of numbers
int array_test() {
	int size = 'a';
	int array['a'];

	for (int i = 0; i < size; i = i + 1) {
		array[i] = i;
	}

	if (array['a'] != 97) return 0;
	if (array['A'] != 65) return 0;
	if (array[8] != '\b') return 0;
	if (array['\041'] != '!') return 0;
	if (array['\x30'] != '0') return 0;
	if (array['\n'] != 0x0A) return 0;
	return 1;
}

int main() {
	int a;
	int b;

	a = 'a'; b = 97;
	if (a != b) { return 0;	}

	a = 'b'; b = 0x62; // hex
	if (a != b) { return 0;	}

	a = 'A'; b = 0101; // oct
	if (a != b) { return 0;	}

	a = '\0'; b = 0;
	if (a != b) { return 0;	}

	a = '\''; b = 0x27;
	if (a != b) { return 0;	}

	a = '\"'; b = 0x22;
	if (a != b) { return 0;	}

	a = '\?'; b = 0x3f;
	if (a != b) { return 0;	}

	a = '\n'; b = 0x0a;
	if (a != b) { return 0;	}

	a = '\r'; b = 0x0d;
	if (a != b) { return 0;	}

	a = '\t'; b = 0x09;
	if (a != b) { return 0;	}

	a = '\v'; b = 0x0b;
	if (a != b) { return 0;	}

	a = '\0102'; b = 0102;
	if (a != b) { return 0;	}

	// Technically the following is not legal, but we accept it
	a = '\0xCA'; b = 0xCA;
	if (a != b) { return 0;	}

	a = '\xCA'; b = 0xCA;
	if (a != b) { return 0;	}

	a = array_test();
	if (!a) return 0;

	return 12;
}

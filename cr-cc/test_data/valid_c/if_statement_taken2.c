// ret: 23
int main () {
	int a = 1;
	int b = 1;
	if (a == b) {
		int c = 11;
		a = c;
		b = 12;
	} else {
		a = 13;
		b = 14;
	}
	
	return a + b;
}

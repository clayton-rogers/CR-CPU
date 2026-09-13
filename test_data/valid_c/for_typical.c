// ret: 16
int main () {
    int i = 10;
	int a = 0;
    
	for (int i = 0; i < 3; i = i + 1) {
		a = 2 + a;
	}

    return a + i;
}

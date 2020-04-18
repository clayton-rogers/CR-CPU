int sum(int a, int b) {
    return a + b;
}

// ret: 21
int main() {
    //int a = sum(1, 2) - (sum(1, 2) / 2) * 2;
	int a = sum(1,2) - (sum(1,2) + 11) + 17;
	// 3 - (3+11) + 7
	// 20 - 14
	// 6
    int b = 17 + sum(3, 4) + sum(1, 2);
	// 17 + 7 + 3
	// 27
    return b - a;
	// 27 - 6
	// 21
}
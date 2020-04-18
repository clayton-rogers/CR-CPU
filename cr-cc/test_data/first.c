int fib(int n) {
    if (n == 0 || n == 1) {
        return n;
    } else {
        return fib(n - 1) + fib(n - 2);
    }
}

// ret: 8
int main() {
    int n = 6;
	return fib(n);
}

//int test() {
//	int a = 1;
//	return 1 == a;
//}

// This is the first program possible
int main () {
    int a = 2;
    {
        a = 3;
        int a = 4;
    }
	int b = 2;
    return a;
}

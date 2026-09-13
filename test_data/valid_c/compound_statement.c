// ret: 11
int main() {
	int a = 4;
	{
		int b = 7;
		a = b + a;
	}
	return a;
}
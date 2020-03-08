// ret: 8
// a = 3
// b = 5
int main() {
    int a = 1 ? 2 ? 3 : 4 : 5;
    int b = 0 ? 2 ? 3 : 4 : 5;
    return a + b;
}
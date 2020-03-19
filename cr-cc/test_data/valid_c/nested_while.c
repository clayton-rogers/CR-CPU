// ret: 40
int main() {
    int a = 1;

    while (a - 20 < 20) {
        int b = 1;
        while (b < 10)
            b = b+6;
        a = a + b;
    }

    return a;
}
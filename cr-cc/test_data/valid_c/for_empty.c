// ret: 4
int main() {
    int a = 0;
    for (; ; ) {
        a = a + 1;
        if (a > 3)
            break;
    }

    return a;
}
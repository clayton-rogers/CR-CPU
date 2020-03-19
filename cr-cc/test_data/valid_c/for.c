// ret: 8
int main() {
    int a = 0;

    for (a = 0; a < 3; a = a + 1)
        a = a + 7;
    return a;
}
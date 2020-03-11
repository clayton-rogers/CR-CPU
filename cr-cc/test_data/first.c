// ret: 5
int main() {
    int sum = 5;
    for (int i = 0; i < 10;) {
        i = i + 1;
        sum = sum + i;
    }
    return sum;
}

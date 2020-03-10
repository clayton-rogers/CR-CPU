// ret: 30
int main() {
    int sum = 0;
    for (int i = 0; i < 10;) {
        i = i + 1;
        if (i & 1)
            continue;
        sum = sum + i;
    }
    return sum;
}
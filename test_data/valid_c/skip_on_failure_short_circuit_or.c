// ret: 2
// should not eval assignment of b=5 and thus returns
// prev value
int main() {
    int a = 1;
    int b = 2;
    a || (b = 5);
    return b;
}
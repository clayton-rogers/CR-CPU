// ret: 6
// a is false, so b does not eval
int main() {
    int a = 0;
    int b = 6;
    a && (b = 5);
    return b;
}
// ret: 3
int main() {
    int a = 2;
    {
        a = 3;
        int a = 4;
    }
    return a;
}
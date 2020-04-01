int foo(int a, int b);

// ret: -1
int main() {
    return foo(1, 2);
}

int foo(int x, int y){
    return x - y;
}
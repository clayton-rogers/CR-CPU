int foo(int a, int b);

// ret: 0xFFFF
// result of the sim is always unsigned
int main() {
    return foo(1, 2);
}

int foo(int x, int y){
    return x - y;
}
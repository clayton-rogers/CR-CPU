int foo(int b);

// ret: 4
int main(){
    return foo(3);
}

int foo(int a){
    return a + 1;
}
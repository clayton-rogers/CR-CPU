// ret: 2


int basic_pointer_to_pointer()
{
    int a = 10;

    int* pa = &a;

    int** ppa = &pa;

    return **ppa == 10;
}

int main()
{
    if (!basic_pointer_to_pointer()) {
        return 0;
    }

    return 2;
}

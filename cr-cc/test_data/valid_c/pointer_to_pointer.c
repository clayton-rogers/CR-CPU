// ret: 2


int basic_pointer_to_pointer()
{
    int a = 10;

    int* pa = &a;

    int** ppa = &pa;

    int *deref_1 = *ppa;

    int deref_2 = *deref_1;

    return deref_2 == 10;
}

int double_deref() {
    int a = 11;
    int *pa = &a;
    int ** ppa = &pa;

    return **ppa == 11;
}

int main()
{
    if (!basic_pointer_to_pointer()) {
        return 0;
    }

    if (!double_deref()) {
        return 0;
    }

    return 2;
}

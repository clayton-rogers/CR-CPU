// ret: 410
// ran on gcc, might not actually be true
// might overflow or something
int main() {
    int ans = 0;
	int a = 4;
    for (int i = 0; i < 10; i = i + 1)
        for (int j = 0; j < 10; j = j + 1)
            if ( a == i)
                break;
            else
                ans = ans + i;
    return ans;
}

// defined in assembly
int mpmp_triangle(int a, int b, int c);

int mpmp_pack_ans(int a, int b, int c);

int main() {
	int MAX = 9;
	int total = 0;
	int last_i = 0;
	int last_j = 0;
	int last_k = 0;

	for (int i = 1; i < MAX; i = i + 1) {
		for (int j = 1; j < MAX; j = j + 1) {
			for (int k = 1; k < MAX; k = k + 1) {;
				if (mpmp_triangle(i,j,k)) {
					total = total + 1;
					last_i = i;
					last_j = j;
					last_k = k;
				}
			}
		}
	}

	return mpmp_pack_ans(last_i, last_j, last_k);

	return total;
}

// Number of clocks to compute for a given max N
//  8  2.6M
//  9  3.9M
// 10  5.6M
// 11  7.7M
// 12 10.2M
// 13 13.3M
// 14 16+M  (10.4+s clock time)

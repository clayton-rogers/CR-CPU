
int sqrt(int a) {
	if (a == 0) { return 0; } // prevent div/0
	int next_guess;
	int guess = (a >> 1) + 1; // shift equivalent to div 2

	while (1) {
		next_guess = (guess + a/guess) >> 1;
		if (next_guess >= guess) {
			break;
		}
		guess = next_guess;
	}

	return guess;
}
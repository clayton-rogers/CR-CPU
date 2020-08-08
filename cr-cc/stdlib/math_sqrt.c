
int sqrt(int a) {
	if (a == 0) { return 0; } // prevent div/0
	int next_guess;
	int guess = a/2 + 1;

	while (1) {
		next_guess = (guess + a/guess) / 2; // TODO change to shift
		if (next_guess >= guess) {
			break;
		}
		guess = next_guess;
	}

	return guess;
}
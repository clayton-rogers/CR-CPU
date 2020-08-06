

int sqrt(int a) {
	int guess = a/2; // TODO change to shift
	int last_guess = guess;

	while (1) {
		guess = (last_guess + a/last_guess) / 2; // TODO again change to shift
		if (guess == last_guess) {
			break;
		}
		last_guess = guess;
	}

	return guess;
}
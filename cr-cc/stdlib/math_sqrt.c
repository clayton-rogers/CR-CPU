
int sqrt(int a) {
	// TODO for some reason doesn't work with 1;
	if (a == 1) { return 1; }
	// TODO doesn't work for 3 because of the way the default guess works
	if (a == 3) { return 1; }

	int guess = a/2; // TODO change to shift
	int last_last_guess = 0;
	int last_guess = guess;

	while (1) {
		guess = (last_guess + a/last_guess) / 2; // TODO again change to shift
		if (guess == last_guess) {
			break;
		}
		// using integers this algo is unstable for some numbers
		if (guess == last_last_guess) {
			guess = guess - 1;
			break;
		}
		last_last_guess = last_guess;
		last_guess = guess;
	}

	return guess;
}
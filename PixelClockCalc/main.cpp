#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>

static const double INPUT_FREQ = 16.0;

double calc_clock(int DIVF, int DIVQ, int DIVR) {
	const double divf = DIVF;
	const double divq = DIVQ;
	const double divr = DIVR;

	// Fout = Fref (DIVF + 1) / ((2^DIVQ)*(DIVR+1))

	const double top = INPUT_FREQ * (divf + 1);
	const double bottom = std::exp2(divq) * (divr + 1);

	if (bottom == 0.0) {
		return 0.0;
	}

	return top / bottom;
}

const int DIVR_MAX = 0b1111;
const int DIVF_MAX = 0b1111111;
const int DIVQ_MAX = 0b111;

struct Result {
	double clock = 0.0;
	int divr = 0;
	int divf = 0;
	int divq = 0;

	double phase_detector_input_freq = 0.0;
	double pll_vco_freq = 0.0;
};

bool operator<(const Result& a, const Result& b) {
	return a.clock < b.clock;
}

int main() {

	std::vector<Result> results;
	
	for (int divr = 0; divr <= DIVR_MAX; ++divr) {
		for (int divf = 0; divf <= DIVF_MAX; ++divf) {
			for (int divq = 0; divq <= DIVQ_MAX; ++divq) {
				const auto output_clock = calc_clock(divf, divq, divr);
				results.push_back(
					{
						output_clock,
						divr,
						divf,
						divq,
						INPUT_FREQ / (divr+1),
						output_clock * std::exp2(divq)
					}
				);
			}
		}
	}

	std::sort(results.begin(), results.end());

	for (const auto& result : results) {
		std::cout << "Clock: " << result.clock
			<< " DIVR: " << result.divr
			<< " DIVF: " << result.divf
			<< " DIVQ: " << result.divq
			<< " phase_detec: " << result.phase_detector_input_freq
			<< " VCO: " << result.pll_vco_freq;

		// See page 3-16 of datasheet for ICE40 LP/HX
		if (result.phase_detector_input_freq < 10.0 || result.phase_detector_input_freq > 133.0) {
			std::cout << " WARNING: phase detector out of range 10 .. 133 MHz ";
		}
		if (result.pll_vco_freq < 533 || result.pll_vco_freq > 1066.0) {
			std::cout << " WARNING: vco out of range 533 .. 1056 MHz";
		}
		std::cout << std::endl;
	}

}
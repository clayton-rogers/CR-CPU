#pragma once

#include <vector>
#include <cstdint>

class Simulator_Spi_Flash {
public:

	void transfer(std::vector<std::uint8_t>* transfer_data);

	Simulator_Spi_Flash() :
		data(1024*1024, 0xFF),
		data_erased(1024*1024, true) { }

private:
	bool WEL_bit = false;

	std::vector<std::uint8_t> data;
	std::vector<bool> data_erased;
};

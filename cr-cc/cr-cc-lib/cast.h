#pragma once

#include <cstdint>
#include <stdexcept>

template<typename T>
std::uint16_t u16(T value) {

	constexpr T UPPER = static_cast<T>(65535);
	constexpr T LOWER = static_cast<T>(0);

	// UPPER can end up as -1 for small signed types
	if constexpr (UPPER > LOWER) {
		if (value > UPPER) {
			throw std::logic_error("Tried to cast invalid u16");
		}
	}

	if (value < LOWER) {
		throw std::logic_error("Tried to cast invalid u16");
	}

	return static_cast<std::uint16_t>(value);
}
#pragma once

#include "fake_person_fixture.hpp"

#include <array>
#include <cstddef>
#include "fake_stream.hpp"

class fake_person
{
	static constexpr auto number_of_chromossomes = 23;
	static constexpr auto default_chunk_size = 512;

	std::array<fake_stream, number_of_chromossomes> chroms_;

	static constexpr void check_index(std::integral auto index)
	{
		if constexpr (std::signed_integral<decltype(index)>) {
			if (index < 0) {
				throw std::invalid_argument("index cannot be negative");
			}
		}
		if (index >= number_of_chromossomes) {
			throw std::invalid_argument("index is out of range for the number of chromosomes available");
		}
	}

public:
	template<typename T>
	fake_person(const T& chromosome_data, std::size_t chunk_size = default_chunk_size)
	{

		std::size_t index = 0;
		auto it = chromosome_data.begin();
		for (; index < chromosome_data.size() && it != chromosome_data.end(); ++index, ++it)
			chroms_[index] = fake_stream(*it, chunk_size);
	}

	fake_stream& chromosome(std::size_t chromosome_index)
	{

		check_index(chromosome_index);
		return chroms_[chromosome_index];  // NOLINT:cppcoreguidelines-pro-bounds-constant-array-index
	}

	// Copy the fake_stream on the const verstion
	// This is necessary to be able to seek on this object.
	fake_stream chromosome(std::size_t chromosome_index) const
	{
		check_index(chromosome_index);
		return chroms_[chromosome_index];  // NOLINT:cppcoreguidelines-pro-bounds-constant-array-index
	}

	constexpr std::size_t chromosomes() const
	{
		return chroms_.size();
	}
};



#pragma once

#include "base.hpp"
#include "sequence_buffer.hpp"
#include "interesting.hpp"

#include <cstddef>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <ranges>

namespace dna
{

template<typename T>
concept HelixStream = requires(T a) {
	{ a.seek(long{}) };
	{ a.read() } -> is_sequence_buffer;
	{ a.size() } -> std::convertible_to<std::size_t>;
};

struct Location {
    std::size_t pos = 0;

    Location(std::bidirectional_iterator auto start, std::bidirectional_iterator auto pos)
        : pos { static_cast<std::size_t>(std::distance(start, pos)) }
    { }

    friend std::ostream& operator<<(std::ostream& out, Location loc)
	{
        return out << '[' << loc.pos << ']';
	}
};

struct InterestingLocation {
	static constexpr int NOT_DEFINED = -1;

	all_interesting_t what;
	Location loc;
	int chromosome_num = NOT_DEFINED;
};

inline std::ostream& operator<<(std::ostream& out, const InterestingLocation& interesting)
{
	if (interesting.chromosome_num != InterestingLocation::NOT_DEFINED) {
		out << interesting.chromosome_num << ":";
	}
	return out << interesting.loc << ": " << interesting.what;
}

template <typename T>
concept Person = requires(T a)
{
    { a.chromosome(1) } -> HelixStream;
    { a.chromosomes() } -> std::convertible_to<std::size_t>; 
};

}


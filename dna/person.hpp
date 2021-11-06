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

namespace dna {

template <typename T>
concept HelixStream = requires(T a)
{
    { a.seek(long {}) };
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

auto locate_interesting(is_sequence_view auto& start_source, is_sequence_view auto& start_target, int chromosome_num = InterestingLocation::NOT_DEFINED)
{
    auto result = std::vector<InterestingLocation> {};

    auto source = std::ranges::subrange(start_source);
    auto target = std::ranges::subrange(start_target);

	const auto source_begin = std::ranges::begin(source);
	const auto target_begin = std::ranges::begin(target);

	const auto source_end = std::ranges::end(source);
	const auto target_end = std::ranges::end(target);

    while (!std::ranges::empty(source) && !std::ranges::empty(target)) {
        auto [found_source, found_target] = std::ranges::mismatch(source, target);

		if (found_source == source_end || found_target == target_end) break;

		source.advance(std::ranges::distance(std::ranges::begin(source), found_source));
		target.advance(std::ranges::distance(std::ranges::begin(target), found_target));

        auto found_result = check(source, target);

        if (found_result.found.has_value()) {
			auto loc = Location(source_begin, std::begin(source));
            result.push_back(InterestingLocation{ found_result.found.value(), loc, chromosome_num });
        }
		source = found_result.source;
		target = found_result.target;
    }
    return result;
}

}

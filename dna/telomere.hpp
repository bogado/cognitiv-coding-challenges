#ifndef TELOMERE_HPP_INCLUDED
#define TELOMERE_HPP_INCLUDED

#include "base.hpp"
#include "sequence_buffer.hpp"

#include <algorithm>
#include <array>
#include <ranges>

namespace dna {

static constexpr auto Telomere = std::array {
    dna::T,
    dna::T,
    dna::A,
    dna::G,
    dna::G,
    dna::G
};

constexpr auto TelomereSize = std::size(Telomere);

constexpr auto skip_telomere(is_sequence_view auto& sequence) noexcept
{
    const auto empty_result = std::tuple {
		std::ranges::subrange(std::begin(sequence), std::begin(sequence)),
		std::ranges::subrange(std::begin(sequence), std::end(sequence))
	};
    if (std::ranges::size(sequence) <= 2) {
        return empty_result;
    }

    auto start       = std::begin(sequence);
    auto finish      = std::end(sequence);
    auto telomere_it = std::begin(Telomere);

    auto initial = *start;
    switch (initial) {
    case dna::A:
        std::advance(telomere_it, 2);
        break;
    case dna::T:
        if (*std::next(start) == dna::A) {
            std::advance(telomere_it, 1);
        }
        break;
    case dna::G: {
        auto count = std::ranges::count(sequence | std::views::take(3), dna::G);

        std::advance(telomere_it, 3 - count);
    } break;
    case dna::C:
        return empty_result;
    }

    auto result = start;
    while (start != finish && *telomere_it == *start) {
        start++;
        telomere_it++;
        if (telomere_it == std::end(Telomere)) {
            result      = start;
            telomere_it = std::begin(Telomere);
        }
    }
    return std::tuple { std::ranges::subrange(std::begin(sequence), result), std::ranges::subrange(result, std::end(sequence)) };
}

}

#endif

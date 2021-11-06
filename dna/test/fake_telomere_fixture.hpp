#ifndef FAKE_TELEROMES_FIXTURES_HPP_INCLUDED
#define FAKE_TELEROMES_FIXTURES_HPP_INCLUDED

#include "base.hpp"
#include "telomere.hpp"
#include <algorithm>
#include <array>

// NOLINT: cppcoreguidelines-avoid-magic-numbers
// This is irrelevant for tests.

namespace fixtures {

template <std::size_t... SIZE>
constexpr auto join_sequences(const std::array<dna::base, SIZE>&... sub_streams)
{
    auto result = std::array<dna::base, (SIZE + ...)> {};
	auto iterator = std::begin(result);
	[[maybe_unused]] auto unused = ((std::copy(std::begin(sub_streams), std::end(sub_streams), iterator) != std::end(result)) && ...);
    return result;
}

template <std::size_t COUNT, std::size_t SIZE>
constexpr auto repeat_sequence(const std::array<dna::base, SIZE>& sequence)
{
	if constexpr (COUNT == 0) {
		return std::array<dna::base, 0>{};
	} else {
		auto result = std::array<dna::base, SIZE * COUNT>{};
		for (auto iterator = std::begin(result); iterator != std::end(result);) {
			iterator = std::copy(std::begin(sequence), std::end(sequence), iterator);
		}
		return result;
	}
}

template <std::size_t COPIES>
constexpr auto fake_telomeres = std::invoke([]() {
    auto result = std::array<dna::base, dna::TelomereSize * COPIES> {};
    if constexpr (COPIES == 0) {
        return result;
    } else {
        std::generate(
            std::begin(result),
            std::end(result),
            [source_iterator = std::begin(dna::Telomere)]() mutable {
                auto result = *source_iterator;
				source_iterator++;
                if (source_iterator == std::end(dna::Telomere)) {
                    source_iterator = std::begin(dna::Telomere);
                }
				return result;
            });
        return result;
    }
});

static_assert(std::size(fake_telomeres<0>) == 0);
static_assert(std::size(fake_telomeres<1>) == dna::TelomereSize);
static_assert(std::size(fake_telomeres<2>) == dna::TelomereSize * 2);

static_assert(fake_telomeres<1>[0] == dna::T);
static_assert(fake_telomeres<1>[1] == dna::T);
static_assert(fake_telomeres<1>[2] == dna::A);
static_assert(fake_telomeres<1>[3] == dna::G);
static_assert(fake_telomeres<1>[4] == dna::G);
static_assert(fake_telomeres<1>[5] == dna::G);

static_assert(fake_telomeres<2>[6]  == dna::T);
static_assert(fake_telomeres<2>[7]  == dna::T);
static_assert(fake_telomeres<2>[8]  == dna::A);
static_assert(fake_telomeres<2>[9]  == dna::G);
static_assert(fake_telomeres<2>[10] == dna::G);
static_assert(fake_telomeres<2>[11] == dna::G);

constexpr auto empty_telomere = fake_telomeres<0>;

template <std::size_t COUNT, int CUT>
constexpr auto fake_partial_telomere = std::invoke([]()
{
    constexpr auto source = fake_telomeres<COUNT>;
    auto           result = std::array<dna::base, std::size(source) - CUT> {};

    if (CUT < 0) {
        std::copy(
            std::begin(source),
			std::next(std::end(source), CUT),
			std::begin(result));
    } else {
        std::copy(
            std::next(std::begin(source), CUT),
            std::end(source),
			std::begin(result));
    }
	return result;
});

static_assert(fake_partial_telomere<1,  2>[0] == dna::A);
static_assert(fake_partial_telomere<1, -3>[3] == dna::A);

}

#endif

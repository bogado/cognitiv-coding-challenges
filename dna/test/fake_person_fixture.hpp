#ifndef FAKE_PERSON_FIXTURE_HPP_INCLUDED
#define FAKE_PERSON_FIXTURE_HPP_INCLUDED

#include "base.hpp"
#include "telomere.hpp"
#include "util.hpp"

#include <concepts>
#include <array>
#include <iterator>
#include <type_traits>
#include <algorithm>
#include <span>
#include <ranges>

namespace fixtures {
enum class telomere_status {
    COMPLETE,
    EXTRA_PARTIAL_START,
    EXTRA_PARTIAL_END
};

template <std::size_t... SIZE>
constexpr auto join_sequences(const std::array<dna::base, SIZE>&... sub_streams)
{
    auto result = std::array<dna::base, (SIZE + ...)> {};
	auto iterator = std::begin(result);
	[[maybe_unused]] auto unused = ((std::copy(std::begin(sub_streams), std::end(sub_streams), iterator) != std::end(result)) && ...);
    return result;
}

//requires(std::same_as<std::ranges::range_value_t<SEQ>, dna::base>)
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

template <telomere_status STATUS>
constexpr auto incomplete_telomere = std::invoke([]() {
    const auto SKIP_START = 4;
    const auto SKIP_END   = dna::TelomereSize / 2;

    if constexpr (STATUS == telomere_status::COMPLETE) {
        return std::array<dna::base, 0> {};
    } else if constexpr (STATUS == telomere_status::EXTRA_PARTIAL_START) {
		auto result = std::array<dna::base, dna::TelomereSize - SKIP_START>{};
		std::copy(
			std::next(std::begin(dna::Telomere), SKIP_START),
            std::end(dna::Telomere),
			std::begin(result));
		return result;

    } else if constexpr (STATUS == telomere_status::EXTRA_PARTIAL_END) {
		auto result = std::array<dna::base, dna::TelomereSize - SKIP_END>{};
		std::copy(
            std::begin(dna::Telomere),
            std::prev(std::end(dna::Telomere), SKIP_END),
			std::begin(result));
		return result;
    }
});

constexpr auto empty_telomere = incomplete_telomere<telomere_status::COMPLETE>;

template <std::size_t COUNT, telomere_status STATUS>
constexpr auto make_fake_telomere()
{
    auto start_seq = std::invoke([]() {
        if constexpr (STATUS == telomere_status::EXTRA_PARTIAL_START) {
            return incomplete_telomere<telomere_status::EXTRA_PARTIAL_START>;
        } else {
            return empty_telomere;
        }
    });

	auto middle = repeat_sequence<COUNT>(dna::Telomere);

    auto end_seq = std::invoke([]() {
        if constexpr (STATUS == telomere_status::EXTRA_PARTIAL_END) {
            return incomplete_telomere<telomere_status::EXTRA_PARTIAL_END>;
        } else {
            return empty_telomere;
        }
    });

    return join_sequences(start_seq, middle, end_seq);
}

template <std::size_t COUNT, telomere_status STATUS = telomere_status::COMPLETE>
constexpr auto fake_telomere = make_fake_telomere<COUNT, STATUS>();

static_assert(
    fake_telomere<0, telomere_status::EXTRA_PARTIAL_START>[0] == dna::G
        && fake_telomere<0, telomere_status::EXTRA_PARTIAL_START>[1] == dna::G,
    "Invalid fake partial start Telomere");

static_assert(
    fake_telomere<0, telomere_status::EXTRA_PARTIAL_END>[0] == dna::T
        && fake_telomere<0, telomere_status::EXTRA_PARTIAL_END>[1] == dna::T
        && fake_telomere<0, telomere_status::EXTRA_PARTIAL_END>[2] == dna::A,
    "Invalid fake end partial Telomere");

static_assert(
    fake_telomere<1, telomere_status::COMPLETE>[0] == dna::T
        && fake_telomere<1, telomere_status::COMPLETE>[1] == dna::T
        && fake_telomere<1, telomere_status::COMPLETE>[2] == dna::A
        && fake_telomere<1, telomere_status::COMPLETE>[3] == dna::G
        && fake_telomere<1, telomere_status::COMPLETE>[4] == dna::G,
    "Invalid fake end partial Telomere");

static_assert(
    std::size(fake_telomere<0, telomere_status::COMPLETE>) == 0
        && std::size(fake_telomere<1, telomere_status::COMPLETE>) == dna::TelomereSize
        && std::size(fake_telomere<2, telomere_status::COMPLETE>) == 2 * dna::TelomereSize,
    "Invalid size for fake_telomere");

template <std::unsigned_integral auto SEED>
using base_random_generator_type = util::constexpr_random<dna::base, dna::G, dna::A, SEED>;

template <std::integral auto SIZE, std::unsigned_integral auto START_SEED>
constexpr auto random_sequence() {
	auto random_generator = base_random_generator_type<START_SEED>{};
	auto result = std::array<dna::base, SIZE>{};
	std::generate(std::begin(result), std::end(result), random_generator);
	return result;
}

constexpr auto chromossome_sizes = std::array {
    48'956'422 ,
	42'193'529 ,
	98'295'559 ,
	90'214'555 ,
	81'538'259 ,
	70'805'979 ,
	59'345'973 ,
	45'138'636 ,
	38'394'717 ,
	133'797'422 ,
	135'086'622 ,
	133'275'309 ,
	114'364'328 ,
	107'043'718 ,
	101'991'189 ,
	90'338'345 ,
	83'257'441 ,
	80'373'285 ,
	58'617'616 ,
	64'444'167 ,
	46'709'983 ,
	50'818'468 ,
	156'040'895 ,
	57'227'415
};

static constexpr auto chromossome_X = 24;
static constexpr auto chromossome_Y = 23;

template <int NUM, size_t TELOMERE_START_COUNT, size_t TELOMERE_END_COUNT, telomere_status STATUS = telomere_status::COMPLETE>
constexpr auto fake_chomossome = std::invoke([]() {
	return join_sequences(
		fake_telomere<TELOMERE_START_COUNT, STATUS>,
		random_sequence<chromossome_sizes[NUM], NUM>(),
		fake_telomere<TELOMERE_END_COUNT, STATUS>
	);
});

}

#endif

#ifndef FAKE_PERSON_FIXTURE_HPP_INCLUDED
#define FAKE_PERSON_FIXTURE_HPP_INCLUDED

#include "base.hpp"
#include "telomere.hpp"

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

	template <telomere_status STATUS>
	constexpr auto incomplete_telomere = std::invoke([]()
	{
		const auto SKIP_START = 4;
		const auto SKIP_END = dna::TelomereSize/2;

		if constexpr (STATUS == telomere_status::COMPLETE) {
			return std::span<dna::base, 0>{};
		} else if constexpr (STATUS == telomere_status::EXTRA_PARTIAL_START) {
			return std::span{
				std::next(std::begin(dna::Telomere), SKIP_START),
				std::end(dna::Telomere)
			};
		} else if constexpr(STATUS == telomere_status::EXTRA_PARTIAL_END) {
			return std::span{
				std::begin(dna::Telomere),
				std::prev(std::end(dna::Telomere), SKIP_END)
			};
		}
	});

	template <std::size_t COUNT, telomere_status STATUS>
	constexpr auto make_fake_telomere()
	{
		std::array<dna::base, COUNT * dna::TelomereSize + std::size(incomplete_telomere<STATUS>)> result{};
		auto iterator = std::begin(result);

		if constexpr (STATUS == telomere_status::EXTRA_PARTIAL_START) {
			iterator = std::copy(
				std::begin(incomplete_telomere<telomere_status::EXTRA_PARTIAL_START>),
				std::end(incomplete_telomere<telomere_status::EXTRA_PARTIAL_START>),
				iterator
			);
		}

		auto stop_copy = std::prev(
			std::end(result),
			(STATUS == telomere_status::EXTRA_PARTIAL_END)
				? std::size(incomplete_telomere<telomere_status::EXTRA_PARTIAL_END>)
				: 0
		);

		while(iterator != stop_copy) {
			iterator = std::copy(std::begin(dna::Telomere), std::end(dna::Telomere), iterator);
		}

		if constexpr (STATUS == telomere_status::EXTRA_PARTIAL_END) {
			iterator = std::copy(
				std::begin(incomplete_telomere<telomere_status::EXTRA_PARTIAL_END>),
				std::end(incomplete_telomere<telomere_status::EXTRA_PARTIAL_END>),
				iterator
			);
		}

		return result;
	}

	template <std::size_t COUNT, telomere_status STATUS = telomere_status::COMPLETE>
	constexpr auto fake_telomere = make_fake_telomere<COUNT, STATUS>();

	static_assert(
		   fake_telomere<0, telomere_status::EXTRA_PARTIAL_START>[0] == dna::G
		&& fake_telomere<0, telomere_status::EXTRA_PARTIAL_START>[1] == dna::G
	 , "Invalid fake partial start Telomere");

	static_assert(
		   fake_telomere<0, telomere_status::EXTRA_PARTIAL_END>[0] == dna::T
		&& fake_telomere<0, telomere_status::EXTRA_PARTIAL_END>[1] == dna::T
		&& fake_telomere<0, telomere_status::EXTRA_PARTIAL_END>[2] == dna::A
	 , "Invalid fake end partial Telomere");

	static_assert(
		   fake_telomere<1, telomere_status::COMPLETE>[0] == dna::T
		&& fake_telomere<1, telomere_status::COMPLETE>[1] == dna::T
		&& fake_telomere<1, telomere_status::COMPLETE>[2] == dna::A
		&& fake_telomere<1, telomere_status::COMPLETE>[3] == dna::G
		&& fake_telomere<1, telomere_status::COMPLETE>[4] == dna::G
	 , "Invalid fake end partial Telomere");

	static_assert(
			std::size(fake_telomere<0, telomere_status::COMPLETE>) == 0
		&&	std::size(fake_telomere<1, telomere_status::COMPLETE>) == dna::TelomereSize
		&&	std::size(fake_telomere<2, telomere_status::COMPLETE>) == 2 * dna::TelomereSize
		, "Invalid size for fake_telomere");

}


#endif

#ifndef UTIL_HPP_INCLUDED
#define UTIL_HPP_INCLUDED

#include <concepts>
#include <utility>

namespace util {
	template <typename DOMAIN, DOMAIN MIN, DOMAIN MAX, std::unsigned_integral auto START_SEED = 0LL, typename SEED_TYPE = decltype(START_SEED)>
	requires(std::constructible_from<DOMAIN, SEED_TYPE> && std::convertible_to<DOMAIN, SEED_TYPE>)
	struct constexpr_random {
		using value_type = DOMAIN;
		using seed_type  = SEED_TYPE;

		static constexpr auto min_seed = static_cast<SEED_TYPE>(MIN);
		static constexpr auto max_seed = static_cast<SEED_TYPE>(MAX);

		seed_type seed = START_SEED;

		// Constants used by the posix random: (see https://en.wikipedia.org/wiki/Linear_congruential_generator#Parameters_in_common_use)
		static constexpr auto multiplier = 0x5DEECE66DLL;
		static constexpr auto increment  = 11LL;
		static constexpr auto modulo     = 1LL << 48;

		static constexpr auto bit_range  = std::pair{15, 47};

		constexpr void update_seed() {
			seed = (seed * multiplier + increment) % modulo;
		}

		constexpr auto raw_value() const {
			return (seed && ((1 << bit_range.second) -1)) >> bit_range.first;
		}

		constexpr value_type next() {
			update_seed();
			if constexpr ((MAX - MIN) <= ((1 << bit_range.second) - 1) >> bit_range.first)
			return static_cast<value_type>((raw_value() + MIN) % (MAX - MIN)); // TODO: this is not uniform distributed but good enouth
		}
	};
}

#endif

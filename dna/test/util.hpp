#ifndef UTIL_HPP_INCLUDED
#define UTIL_HPP_INCLUDED

#include <bits/ranges_base.h>
#include <concepts>
#include <type_traits>
#include <utility>
#include <limits>
#include <functional>
#include <ranges>

namespace util {
	
	template<typename GENERATOR>
	concept simple_generator = requires (GENERATOR gen, const GENERATOR cgen)
	{
		std::invocable<GENERATOR>;
		typename GENERATOR::value_type;
		std::same_as<std::invoke_result_t<GENERATOR>, typename GENERATOR::value_type>;
		{  gen.skip(size_t(0)) } -> std::same_as<GENERATOR>;
	};

	template<typename TYPE, typename OTHER_TYPE>
	concept explicitly_convertible_to = requires(TYPE value) {
		{ static_cast<OTHER_TYPE>(value) } -> std::same_as<OTHER_TYPE>;
	};

	constexpr auto skiped_copy(simple_generator auto generator, size_t count = 1)
	{
		return generator.skip(count);
	}

	template <typename DOMAIN, DOMAIN MIN, DOMAIN MAX, std::integral auto START_SEED = 0ULL, std::integral SEED_TYPE = long long unsigned>
	requires (explicitly_convertible_to<DOMAIN, SEED_TYPE>)
	struct constexpr_random
	{
		using value_type = DOMAIN;
		using seed_type  = SEED_TYPE;

		static constexpr auto min_seed = static_cast<SEED_TYPE>(MIN);
		static constexpr auto max_seed = static_cast<SEED_TYPE>(MAX);

		seed_type seed = static_cast<seed_type>(START_SEED);

		// Constants used by the posix random: (see https://en.wikipedia.org/wiki/Linear_congruential_generator#Parameters_in_common_use)
		static constexpr auto multiplier = 0x5DEECE66DLL;
		static constexpr auto increment  = 11LL;
		static constexpr auto modulo     = 1LL << 48;

		static constexpr auto bit_range  = std::pair<seed_type, seed_type>{15, 47};

		constexpr auto skip(size_t size = 1) {
			for (int i = 0; i < size; i++) {
				update_seed();
			}
			return *this;
		}

		constexpr auto skip(size_t size = 1) const {
			auto copy = *this;
			return copy.skip(size);
		}

		constexpr void update_seed() {
			seed = (seed * multiplier + increment) % modulo;
		}

		static constexpr auto normalize_mask = static_cast<seed_type>((seed_type(1) < bit_range.second) -1);
		constexpr static auto normalize(seed_type value) {
			return value & normalize_mask >> bit_range.first;
		}

		[[nodiscard]]
		constexpr auto raw_value() const {
			return normalize(seed);
		}

		[[nodiscard]]
		constexpr value_type operator()() {
			update_seed();
			if constexpr ((max_seed - min_seed) <= normalize(std::numeric_limits<seed_type>::max()))
			{
				// TODO: this is not uniform distributed but good enouth
				return static_cast<value_type>(raw_value() % (max_seed - min_seed) + min_seed);
			} 
			return static_cast<value_type>(seed % (max_seed - min_seed) + min_seed);
		}
	};

	static_assert(simple_generator<constexpr_random<size_t, 0, 10>>);

	template <std::invocable<size_t> FUNCTION>
	requires(!std::same_as<void, std::invoke_result_t<FUNCTION, size_t>>)
	struct generator
	{
		using value_type = std::invoke_result_t<FUNCTION, size_t>;

		FUNCTION generator_function;
		size_t position = 0;

		constexpr generator(FUNCTION&& func) :
			generator_function(std::forward<FUNCTION>(func))
		{}

		constexpr auto skip(size_t value = 1)
		{
			position += value;
			return (*this);
		}
	
		constexpr value_type operator()()
		{
			return generator_function(position++);
		}
	};

	template <std::ranges::range SOURCE_TYPE>
	struct generate_from
	{
		using source_type     = SOURCE_TYPE;
		using value_type      = std::ranges::range_value_t<source_type>;
		using source_iterator = std::ranges::iterator_t<source_type>;

		source_type source;
		source_iterator position;

		generate_from(source_type&& from) :
			source(from),
			position(std::cbegin(from))
		{}

		constexpr auto operator()()
		{
			if (ended()) {
				return value_type{};
			}

			return *(position++);
		}

		constexpr auto skip(size_t count = 1)
		{
			std::ranges::advance(position, count, std::ranges::end(source));
			return *this;
		}
		
		auto ended() const {
			return position == std::end(source);
		}

		auto consumed() const {
			return static_cast<std::size_t>(std::ranges::distance(std::ranges::begin(source), position));
		}
	};

	namespace test
	{
		static_assert(simple_generator<constexpr_random<size_t, 0, 10>>);
		static_assert(simple_generator<generate_from<std::array<int, 3>>>);
		static_assert(simple_generator<generator<std::identity>>);
	}

	template <std::size_t SIZE, simple_generator GENERATOR_TYPE>
	constexpr auto generate_array(GENERATOR_TYPE generator, std::size_t skip = 0)
	{
		auto result = std::array<typename GENERATOR_TYPE::value_type, SIZE>{};
		generator.skip(skip);
		std::generate(std::begin(result), std::end(result), generator);
		return result;
	}
}

#endif

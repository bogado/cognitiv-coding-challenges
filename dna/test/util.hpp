#ifndef UTIL_HPP_INCLUDED
#define UTIL_HPP_INCLUDED

#include <concepts>
#include <utility>
#include <limits>
#include <functional>

namespace util {

	template<typename GENERATOR, typename VALUE>
	concept simple_generator = requires (GENERATOR gen, const GENERATOR cgen) {
		std::invocable<GENERATOR>;
		std::same_as<std::invoke_result_t<GENERATOR>, VALUE>;
		{ gen.skip(size_t(0)) } -> std::same_as<GENERATOR>;
		{ cgen.ended() } -> std::convertible_to<bool>;
		{ cgen.consumed() } -> std::same_as<size_t>;
	};

	template <typename DOMAIN, DOMAIN MIN, DOMAIN MAX, std::unsigned_integral auto START_SEED = 0ULL, typename SEED_TYPE = decltype(START_SEED)>
	requires(std::constructible_from<DOMAIN, SEED_TYPE> && std::convertible_to<DOMAIN, SEED_TYPE>)
	struct constexpr_random {
		using value_type = DOMAIN;
		using seed_type  = SEED_TYPE;

		static constexpr auto min_seed = static_cast<SEED_TYPE>(MIN);
		static constexpr auto max_seed = static_cast<SEED_TYPE>(MAX);

		seed_type seed = START_SEED;
		size_t step = 0;

		// Constants used by the posix random: (see https://en.wikipedia.org/wiki/Linear_congruential_generator#Parameters_in_common_use)
		static constexpr auto multiplier = 0x5DEECE66DLL;
		static constexpr auto increment  = 11LL;
		static constexpr auto modulo     = 1LL << 48;

		static constexpr auto bit_range  = std::pair{15, 47};

		constexpr auto skip(size_t size) {
			for (int i = 0; i < size; i++) {
				update_seed();
			}
			return *this;
		}

		constexpr void update_seed() {
			step++;
			seed = (seed * multiplier + increment) % modulo;
		}

		[[nodiscard]]
		constexpr auto raw_value() const {
			return (seed && ((1 << bit_range.second) -1)) >> bit_range.first;
		}

		[[nodiscard]]
		constexpr value_type operator()() {
			update_seed();
			if constexpr ((MAX - MIN) <= ((1 << bit_range.second) - 1) >> bit_range.first)
			return static_cast<value_type>((raw_value() + MIN) % (MAX - MIN)); // TODO: this is not uniform distributed but good enouth
		}

		auto ended() const {
			return false;
		}

		auto consumed() const {
			return step;
		}
	};

	static_assert(simple_generator<constexpr_random<size_t, 0, 10>, size_t>);

	template <std::invocable<size_t> FUNCTION, size_t MAX = std::numeric_limits<size_t>::max()>
	requires(!std::same_as<void, std::invoke_result_t<FUNCTION, size_t>>)
	struct generator
	{
		using value_type = std::invoke_result_t<FUNCTION, size_t>;
		size_t position = 0;
		FUNCTION generator_method;

		generator()
		requires(std::default_initializable<FUNCTION>) :
			generator_method{}
		{}

		template <typename FUNC_FOWARD>
		generator(FUNC_FOWARD&& func) :
			generator_method(std::forward<FUNC_FOWARD>(func))
		{}

		auto skip(size_t value) {
			position = std::min(MAX, position + value);
			return (*this);
		}

		constexpr value_type operator()() {
			if (ended()) {
				return value_type{};
			}
			return generator_method(position++);
		}

		auto ended() const {
			return position == MAX;
		}

		auto consumed() const {
			return position;
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

		constexpr auto skip(size_t count)
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

	namespace test {
		static_assert(simple_generator<constexpr_random<size_t, 0, 10>, size_t>);
		static_assert(simple_generator<generate_from<std::array<int, 3>>, int>);
		static_assert(simple_generator<generator<std::identity>, size_t>);
	}
}

#endif

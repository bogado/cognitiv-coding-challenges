#ifndef FAKE_PERSON_FIXTURE_HPP_INCLUDED
#define FAKE_PERSON_FIXTURE_HPP_INCLUDED

#include "base.hpp"
#include "person.hpp"
#include "util.hpp"
#include "sequence_buffer.hpp"

#include "./fake_telomere_fixture.hpp"

#include <concepts>
#include <array>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <algorithm>
#include <span>
#include <ranges>
#include <utility>

// NOLINT: cppcoreguidelines-avoid-magic-numbers
// This don't make sense for tests.

namespace fixtures {

template <typename DNA_BASE_GENERATOR>
concept dna_base_generator = requires {
	util::simple_generator<DNA_BASE_GENERATOR>;
	std::same_as<typename DNA_BASE_GENERATOR::value_type, dna::base>;
};

template <size_t SEED>
using base_random_generator_type = util::constexpr_random<dna::base, dna::A, dna::T, SEED>;

template <std::integral auto START_SEED = 0LLU>
const constexpr auto random_sequence_generator = base_random_generator_type<START_SEED>{};

template <std::integral auto START_SEED>
constexpr auto random_sequence_generator_with_mutations(std::convertible_to<std::size_t> auto ...positions) {
	return util::generator(
    [original = random_sequence_generator<START_SEED>, positions...](size_t pos) mutable {
        auto result = original();
        if (((pos == positions) || ... || false)) {
            switch (result) {
			case dna::A:
                return dna::T;
			case dna::G:
                return dna::A;
			case dna::T:
                return dna::C;
			case dna::C:
                return dna::G;
            }
		}
		return result;
    });
}

namespace test {
    enum class op {
        equal,
        not_equal
    };


	static_assert([a = random_sequence_generator<2000>]() mutable { return a.skip(100)(); }() == dna::C);
	static_assert(
		skiped_copy(random_sequence_generator<2000>, 99)()
		== skiped_copy(random_sequence_generator_with_mutations<2000>(100),99)()
	);
	static_assert(
		skiped_copy(random_sequence_generator<2000>, 100)()
		!= skiped_copy(random_sequence_generator_with_mutations<2000>(100),100)()
	);
}

// source: https://en.wikipedia.org/wiki/Human_genome#Molecular_organization_and_gene_content
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

template <int NUM, auto DIVIDER =10'000>
requires (NUM < std::size(chromossome_sizes))
auto const constexpr_size_chromossome = chromossome_sizes[NUM]/DIVIDER;

static constexpr auto chromossome_X = 24;
static constexpr auto chromossome_Y = 23;


template <size_t NUM>
requires (NUM < std::size(chromossome_sizes))
constexpr auto make_base_fake_chromossome_generator(const std::convertible_to<size_t> auto ... mutations) noexcept
{
    return std::invoke([=]() {
        if constexpr (sizeof...(mutations) == 0) {
            return base_random_generator_type<NUM>();
        } else {
            return random_sequence_generator_with_mutations<NUM>(mutations...);
        }
    });
}

template <int                        NUM,
    std::convertible_to<size_t> auto TELOMERE_START_COUNT = 10,
    std::convertible_to<size_t> auto TELOMERE_END_COUNT   = TELOMERE_START_COUNT>
constexpr auto fake_chromossome_size = std::size(fake_telomeres<TELOMERE_START_COUNT>)
                                     + std::size(fake_telomeres<TELOMERE_END_COUNT>)
                                     + constexpr_size_chromossome<NUM>;

template <int NUM,
		 std::convertible_to<size_t> auto TELOMERE_START_COUNT = 10,
		 std::convertible_to<size_t> auto TELOMERE_END_COUNT = TELOMERE_START_COUNT>
requires (NUM < std::size(chromossome_sizes))
constexpr auto make_fake_chromossome_generator(std::convertible_to<size_t> auto... mutations) noexcept
{
	auto start_telomere = fake_telomeres<TELOMERE_START_COUNT>;
	auto end_telomere = fake_telomeres<TELOMERE_END_COUNT>;
	auto base_generator = make_base_fake_chromossome_generator<NUM>(mutations...);

	return util::generator([start_telomere, base_generator, end_telomere](std::size_t pos) mutable {
		const auto start_sequence     = std::size(start_telomere);
		const auto start_end_telomere = start_sequence + constexpr_size_chromossome<NUM>;
	
		if (pos > fake_chromossome_size<NUM, TELOMERE_START_COUNT, TELOMERE_END_COUNT>) { return dna::A; }
		if (pos < std::size(start_telomere)) {
			return start_telomere[pos];
		}

		if (pos >= start_end_telomere && (pos - start_end_telomere) < std::size(end_telomere)) {
			return end_telomere[pos - start_end_telomere];
		}

		return base_generator();
	});
}

template <int NUM,
		 std::convertible_to<size_t> auto TELOMERE_START_COUNT = 10,
		 std::convertible_to<size_t> auto TELOMERE_END_COUNT = TELOMERE_START_COUNT>
requires (NUM < std::size(chromossome_sizes))
constexpr auto make_sequence_buffer_storage(std::convertible_to<size_t> auto... mutations) noexcept {
	return util::generate_array<fake_chromossome_size<NUM, TELOMERE_START_COUNT, TELOMERE_END_COUNT>>(make_fake_chromossome_generator<NUM, TELOMERE_START_COUNT, TELOMERE_END_COUNT>(mutations...));
}

template <size_t SIZE>
constexpr auto to_sequence_array(std::array<dna::base, SIZE> sequence) noexcept
{
	auto result = std::array<std::byte, SIZE/4 + ((SIZE % 4 != 0)?1:0)>{};
	pack_sequence(sequence, std::begin(result));
	return result;
}

template <int NUM, std::convertible_to<std::size_t> auto ... MUTATIONS>
requires (NUM < std::size(chromossome_sizes))
constinit auto fake_sequence_buffer_storage = to_sequence_array(make_sequence_buffer_storage<NUM>(MUTATIONS...));

using fake_sequence_buffer_type = dna::sequence_buffer<std::span<std::byte>>;

static_assert(dna::is_sequence_buffer<fake_sequence_buffer_type>);

template <int NUM, std::convertible_to<std::size_t> auto ... MUTATIONS>
requires (NUM < std::size(chromossome_sizes))
constexpr auto fake_sequence_buffer = fake_sequence_buffer_type(fake_sequence_buffer_storage<NUM, MUTATIONS...>);

struct fake_helix_stream_type {
	fake_sequence_buffer_type buffer;

	void seek(long pos)
	{
		buffer.buffer() = buffer.buffer().subspan(pos);
	}

	auto read()
	{
		return buffer;
	}

	std::size_t size() const
	{
		return std::size(buffer.buffer());
	}
};

static_assert(dna::HelixStream<fake_helix_stream_type>);

template <int NUM, std::convertible_to<std::size_t> auto ... MUTATIONS>
requires(NUM < std::size(chromossome_sizes)) constexpr auto fake_helix_stream = fake_helix_stream_type {
    fake_sequence_buffer<NUM, MUTATIONS...>
};

struct fake_person {
	std::array<fake_helix_stream_type, std::size(chromossome_sizes)> data;

	constexpr auto chromosome(int p) {
		return data[p];
	}

	constexpr size_t chromosomes() noexcept
	{
		return data.size();
	}
};

template<std::convertible_to<size_t> auto ... MUTATIONS>
consteval auto make_fake_person()
{
	return []<size_t... NUMs>(std::index_sequence<NUMs...>) {
		return fake_person{  {fake_helix_stream<NUMs, MUTATIONS...>... } };
	}(std::make_index_sequence<std::size(chromossome_sizes)>());
}

template<auto ARRAY>
requires (std::tuple_size_v<decltype(ARRAY)> > 0)
consteval auto make_fake_person()
{
	return []<size_t... NUMs>(std::index_sequence<NUMs...>) {
		return make_fake_person<std::get<NUMs>(ARRAY)...>();
	}(std::make_index_sequence<std::tuple_size_v<decltype(ARRAY)>>());
}

constexpr auto mutations = std::array{
	50, 60, 70, 90
};

extern const fake_person fake_person_fixture_std;
extern const fake_person fake_person_fixture_mut;

}

#endif

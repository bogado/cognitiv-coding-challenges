#include "catch.hpp"
#include "fake_person_fixture.hpp"
#include "telomere.hpp"
#include "test/fake_person.hpp"

#include <person.hpp>
#include <iostream>
#include <variant>
#include <string_view>
#include <concepts>

struct test_people {
	fixtures::fake_person standard = fixtures::fake_person_fixture_std;
	fixtures::fake_person test_target = fixtures::fake_person_fixture_mut;

	auto test_each_chromossome(
		std::invocable<fixtures::fake_sequence_buffer_type,
					   fixtures::fake_sequence_buffer_type, int
		> auto test) {
		for (auto num = 0; num != standard.chromosomes(); num++) {
			INFO("Chormossome " << num);
			test(standard.chromosome(num).read(), test_target.chromosome(num).read(), num);
		}
	}
};

TEST_CASE_METHOD(test_people, "Fake person skip telomere", "[telomere]")
{
    test_each_chromossome([&](fixtures::fake_sequence_buffer_type source, fixtures::fake_sequence_buffer_type target, int num) {
        auto [src_telomeres, skipped_source] = dna::skip_telomere(source);
        REQUIRE(std::ranges::size(src_telomeres) + std::ranges::size(skipped_source) == std::ranges::size(target));
        REQUIRE(std::ranges::size(src_telomeres) != 0);

        auto [target_telomeres, skipped_target] = dna::skip_telomere(target);
        REQUIRE(std::ranges::size(target_telomeres) + std::ranges::size(skipped_target) == std::ranges::size(source));
        REQUIRE(std::ranges::size(target_telomeres) != 0);

        REQUIRE(std::ranges::size(src_telomeres) == std::ranges::size(target_telomeres));
    });
}

TEST_CASE_METHOD(test_people, "Fake person Chormossome test", "[interesting]")
{
    REQUIRE(standard.chromosomes() == test_target.chromosomes());

	test_each_chromossome([&](fixtures::fake_sequence_buffer_type source, fixtures::fake_sequence_buffer_type target, int num) {
		auto [telomere_source, skipped_source] = dna::skip_telomere(source);
		auto [telomore_target, skipped_target] = dna::skip_telomere(target);
		REQUIRE(telomere_source.begin() != telomore_target.begin());

        auto findings = dna::locate_interesting(skipped_source, skipped_target, num);

        REQUIRE(std::size(findings) == std::size(fixtures::mutations));

        auto expected_loc = std::begin(fixtures::mutations);
        for (auto found: findings) {
            INFO("found : " << found);
            REQUIRE(std::holds_alternative<dna::Snp>(found.what));
            REQUIRE(found.loc.pos == *expected_loc);

            std::cout << found;

			expected_loc++;
        }
    });
}


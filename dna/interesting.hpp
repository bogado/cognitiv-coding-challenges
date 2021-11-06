#ifndef INTERESSING_HPP_INCLUDED
#define INTERESSING_HPP_INCLUDED

#include "base.hpp"
#include "sequence_buffer.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <iterator>
#include <optional>
#include <ranges>
#include <span>
#include <sstream>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace dna {

constexpr auto look_ahead_equal = 2 * 3; // minimum size of an equal range to mark the end of a diff.

auto limit(is_sequence_view auto sequence)
{
    return sequence | std::views::take(look_ahead_equal);
}

template <typename MUTATION_TYPE>
concept mutation_type = requires(const MUTATION_TYPE mutation)
{
    { mutation.id() } -> std::convertible_to<std::string>;
	{ mutation.size() } -> std::same_as<std::size_t>;
};

template <typename MUTATION_TYPE, is_sequence_view SOURCE_VIEW, is_sequence_view TARGET_VIEW>
struct check_result {
    SOURCE_VIEW source;
    TARGET_VIEW target;

    std::optional<MUTATION_TYPE> found;

    void advance_source(int n)
    {
        source.advance(n);
    }

    void advance_target(int n)
    {
        target.advance(n);
    }

    void advance(int n)
    {
        advance_source(n);
        advance_target(n);
    }
};

template <typename MUTATION_TYPE, is_sequence_view SOURCE_VIEW, is_sequence_view TARGET_VIEW>
auto make_result(SOURCE_VIEW source, TARGET_VIEW target, std::optional<MUTATION_TYPE> result = {})
{
    auto source_r = std::ranges::subrange(std::ranges::begin(source), std::ranges::end(source));
    auto target_r = std::ranges::subrange(std::ranges::begin(target), std::ranges::end(target));
    return check_result<MUTATION_TYPE, decltype(source_r), decltype(target_r)> {
        source_r,
        target_r,
        result
    };
}

/** This is a catch all difference
 *
 * Should be the last option on the variant type of interesting things.
 */
struct GenericDiff {
    auto id()
    {
        return "Generic diff";
    }

    static auto check(is_sequence_view auto source, is_sequence_view auto target)
    {
        return make_result<GenericDiff>(source, target);
    }
};

/** Snp "Single Nucleotide Polymorphism"
 *
 */
struct Snp {
    base expected;
    base found;

    std::size_t size() const noexcept
    {
        return 1;
    }

    std::string id() const noexcept
    {
        auto out = std::stringstream {};
        out << found << ">" << expected;
        return out.str();
    }

    static auto check(is_sequence_view auto source, is_sequence_view auto target) noexcept
    {
        auto result = make_result<Snp>(source, target);

        if (std::ranges::empty(source)
            || std::ranges::empty(target)
            || *std::ranges::begin(source) == *std::ranges::begin(target)) {
            return result;
        }

        result.advance(1);

        auto limited_source = limit(result.source);
        if (auto [source_it, target_it] = std::ranges::mismatch(limited_source, result.target);
            source_it == std::ranges::end(limited_source)) {
            result.found = Snp { *std::begin(source), *std::begin(target) };
        }

        return result;
    }
};

static_assert(mutation_type<Snp>);

struct InDel {
    enum category {
        Insertion,
        Deletion
    };

    std::vector<base> data;
    category          type;

    std::size_t size() const
    {
        return std::size(data);
    }

    std::string id() const
    {
        auto out = std::stringstream {};
        if (type == Insertion) {
            out << " Insertion: ";
        } else {
            out << " Deletion: ";
        }
        std::copy(std::begin(data), std::end(data), std::ostream_iterator<base>(out));

        return out.str();
    }

    static auto check(is_sequence_view auto source, is_sequence_view auto target)
    {
        /*
		TODO: IMPLEMENTATION
		*/
        return make_result<InDel>(source, target);
    }
};

std::ostream& operator<<(std::ostream& out, mutation_type auto mutation)
{
    return out << mutation.id() << "\n";
}

using all_interesting_t = std::variant<Snp, InDel, GenericDiff>;

inline std::ostream& operator<<(std::ostream& out, all_interesting_t interesting)
{
    std::visit([&out](const auto& detail) {
        out << detail;
    },
        interesting);
    return out;
}

auto check(is_sequence_view auto source, is_sequence_view auto target)
{
    auto result = make_result<all_interesting_t>(source, target);

    std::invoke(
        [&result]<auto... INDEX>(std::index_sequence<INDEX...>) {
            (std::invoke([&result]() {
                if (result.found.has_value()) {
                    return true;
                }
                auto partial_result = std::variant_alternative_t<INDEX, all_interesting_t>::check(result.source, result.target);
                if (partial_result.found.has_value()) {
                    result.found  = partial_result.found.value();
                    result.source = partial_result.source;
                    result.target = partial_result.target;
                    return true;
                }
                return false;
            }) || ...);
        },
        std::make_index_sequence<std::variant_size_v<all_interesting_t>> {});

    return result;
}
}

#endif

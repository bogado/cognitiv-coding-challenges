#pragma once

#include "sequence_buffer.hpp"

#include <string>
#include <iostream>
#include <sstream>
#include <ranges>

namespace dna
{

template<typename T>
concept HelixStream = requires(T a) {
	{ a.seek(long{}) };
	{ a.read() } -> is_sequence_buffer;
	{ a.size() } -> std::convertible_to<std::size_t>;
};

template<typename T>
concept Person = requires(T a) {
	{ a.chromosome(1) } -> HelixStream;
	{ a.chromosomes() } -> std::convertible_to<std::size_t>;
};

struct location {
	std::size_t chromosome;
	std::size_t position;

	friend std::ostream& operator <<(std::ostream& out, const location& me)
	{
		return out << me.chromosome << ":" << me.position;
	}
};

template <typename MUTATION_TYPE>
concept mutation_type = requires(const MUTATION_TYPE mutation, std::ostream& out) {
	{ mutation.id() } -> std::convertible_to<std::string>;
	{ out << mutation };
	{ mutation.chomossome_location() } -> std::same_as<location>;
	{ mutation.size() } -> std::same_as<std::size_t>;
};

struct snp {
	location loc;
	base expected;
	base found;

	location chomossome_location() {
		return loc;
	}

	std::string id() {
		auto out = std::stringstream{};
		out << *this;
		return out.str();
	}

	friend std::ostream& operator <<(std::ostream& out, const snp& me)
	{
		return out << me.loc << " Alleles: " << me.found << ">" << me.expected;
	}
};

}


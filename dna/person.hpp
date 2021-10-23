#pragma once

#include "sequence_buffer.hpp"

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

}


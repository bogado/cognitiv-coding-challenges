#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>
#include <atomic>
#include <sequence_buffer.hpp>

namespace detail
{

// XXX: I'm not sure that this class is adding any value.
//
// `to_upper`:: is not specified as part of a "char_traits" specification.
// `length`:: This function is specified as returning the location of the first position where the value compares equal to the default constructed value (usually 0).
//
// see (http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/n4861.pdf#subsection.21.2.1)[char.traits.require]
/// Character traits used on the `std::string` container on the `fake_stream` class.
class binary_traits : public std::char_traits<std::byte>
{
public:
	static constexpr std::byte to_upper(std::byte c) noexcept {
		return c;
	}

	static std::size_t length(const std::byte* s) {
		throw std::logic_error("length of a binary string can't be determined. It must be explicitly supplied");
	}
};

}

class fake_stream
{
	std::vector<std::byte> data_;
	std::size_t chunksize_;
	std::atomic<long> offset_;
public:
	using byte_view = std::basic_string_view<std::byte, detail::binary_traits>;

	fake_stream();
	fake_stream(const fake_stream& other);
	fake_stream(fake_stream&& other) noexcept;
	fake_stream(std::vector<std::byte> data, std::size_t chunksize);

	fake_stream& operator=(const fake_stream& other);
	fake_stream& operator=(fake_stream&& other) noexcept;

	void seek(long offset);
	long size() const;
	dna::sequence_buffer<byte_view> read();
};



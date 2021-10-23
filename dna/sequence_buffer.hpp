#pragma once

#include <cstddef>
#include "base.hpp"

namespace dna
{

template<typename T>
concept bool ByteBuffer = requires(T a) {
	{ static_cast<std::size_t>(a.size()) } -> std::size_t;
	{ a[0] } -> std::byte;
};

template<ByteBuffer T>
class sequence_buffer;

template<ByteBuffer T>
class sequence_buffer_iterator
{
	const sequence_buffer<T>* buf_;
	std::size_t index_;
public:
	using iterator_category = std::bidirectional_iterator_tag;
	using value_type = base;
	using difference_type = long;

	constexpr sequence_buffer_iterator() noexcept :
			buf_(nullptr),
			index_(0)
	{ }

	constexpr sequence_buffer_iterator(const sequence_buffer<T>* buffer, std::size_t index = 0) noexcept :
			buf_(buffer),
			index_(index)
	{ }

	constexpr sequence_buffer_iterator(const sequence_buffer_iterator& other) noexcept :
			buf_(other.buf_),
			index_(other.index_)
	{ }

	constexpr sequence_buffer_iterator& operator=(const sequence_buffer_iterator& other) noexcept
	{
		buf_ = other.buf_;
		index_ = other.index_;

		return *this;
	}

	constexpr value_type operator*() const;

	constexpr sequence_buffer_iterator& operator++() noexcept
	{
		++index_;
		return *this;
	}

	constexpr sequence_buffer_iterator operator++(int) noexcept
	{
		sequence_buffer_iterator result = *this;
		++index_;
		return result;
	}

	constexpr sequence_buffer_iterator& operator+=(std::size_t diff) noexcept
	{
		index_ += diff;
		return *this;
	}

	constexpr sequence_buffer_iterator operator+(std::size_t diff) const noexcept
	{
		return sequence_buffer_iterator(buf_, index_ + diff);
	}

	constexpr sequence_buffer_iterator& operator--() noexcept
	{
		--index_;
		return *this;
	}

	constexpr sequence_buffer_iterator operator--(int) noexcept
	{
		sequence_buffer_iterator result = *this;
		--index_;
		return result;
	}

	constexpr long operator-(const sequence_buffer_iterator& other) const noexcept
	{
		return static_cast<long>(index_ - other.index_);
	}

	constexpr sequence_buffer_iterator& operator-=(std::size_t diff) noexcept
	{
		index_ -= diff;
		return *this;
	}

	constexpr sequence_buffer_iterator operator-(std::size_t diff) const noexcept
	{
		return sequence_buffer_iterator(buf_, index_ - diff);
	}

	constexpr bool operator==(const sequence_buffer_iterator& other) noexcept
	{
		return buf_ == other.buf_ && index_ == other.index_;
	}

	constexpr bool operator!=(const sequence_buffer_iterator& other) noexcept
	{
		return !operator==(other);
	}

};

template<ByteBuffer T>
class sequence_buffer
{
	T buffer_;
	std::size_t size_;
public:
	using iterator = sequence_buffer_iterator<T>;

	constexpr sequence_buffer(T buffer, std::size_t size = 0) :
			buffer_(std::forward<T>(buffer)),
			size_(size)
	{
		if (size_ == 0)
			size_ = static_cast<std::size_t>(buffer_.size() * packed_size::value);
	}

	constexpr base at(std::size_t index) const
	{
		auto boffset = index / packed_size::value;
		auto tidx = index - (boffset * packed_size::value);

		return unpack(buffer_[boffset])[tidx];
	}

	constexpr base operator[](std::size_t index) const
	{
		return at(index);
	}

	constexpr std::size_t size() const noexcept
	{
		return size_;
	}

	constexpr iterator begin() const noexcept
	{
		return iterator(this, 0);
	}

	constexpr iterator end() const noexcept
	{
		return iterator(this, size_);
	}

	constexpr const T& buffer() const noexcept
	{
		return buffer_;
	}

	constexpr T& buffer() noexcept
	{
		return buffer_;
	}
};


template<ByteBuffer T>
constexpr typename sequence_buffer_iterator<T>::value_type sequence_buffer_iterator<T>::operator*() const
{
	if (buf_ != nullptr)
		return buf_->at(index_);
	return A;
}

template<ByteBuffer T>
std::ostream& operator<<(std::ostream& os, const sequence_buffer<T>& buf)
{
	for (auto i : buf)
		os << i;
	return os;
}

}


#include "ByteBuffer.h"

namespace Network
{
	uint8_t* ByteBuffer::data() noexcept
	{
		return _dataBuffer.data();
	}

	const uint8_t* ByteBuffer::data() const noexcept
	{
		return _dataBuffer.data();
	}

	size_t ByteBuffer::size() const noexcept
	{
		return _dataSize;
	}

	size_t ByteBuffer::capacity() const noexcept
	{
		return _dataBuffer.size();
	}

	void ByteBuffer::clear() noexcept
	{
		_dataSize = 0; _readPos = 0;
	}

	void ByteBuffer::reserve(size_t capacity)
	{
		if (capacity > _dataBuffer.size()) {
			_dataBuffer.resize(capacity);
		}
	}

	void ByteBuffer::resize(size_t newSize)
	{
		if (newSize > _dataBuffer.size()) {
			_dataBuffer.resize(newSize);
		}
		_dataSize = std::min(_dataSize, newSize);
	}

	void ByteBuffer::append(const uint8_t* data, size_t size) {
		ensureCapacity(_dataSize + size);
		std::memcpy(_dataBuffer.data() + _dataSize, data, size);
		_dataSize += size;
	}

	const uint8_t* ByteBuffer::readBytes(size_t size) {
		if (_readPos + size > _dataSize) {
			throw std::out_of_range("Read bytes beyond buffer size");
		}
		const uint8_t* result = _dataBuffer.data() + _readPos;
		_readPos += size;
		return result;
	}

	size_t ByteBuffer::getReadPosition() const noexcept
	{
		return _readPos;
	}

	void ByteBuffer::setReadPosition(size_t pos) {
		if (pos > _dataSize) {
			throw std::out_of_range("Invalid read position");
		}
		_readPos = pos;
	}

	size_t ByteBuffer::getWritePosition() const noexcept
	{
		return _dataSize;
	}

	void ByteBuffer::setWritePosition(size_t pos) {
		if (pos > _dataBuffer.size()) {
			throw std::out_of_range("Invalid write position");
		}
		_dataSize = pos;
	}

	void ByteBuffer::ensureCapacity(size_t requiredCapacity) {
		if (requiredCapacity > _dataBuffer.size()) {
			// 按需扩容（类似 vector 的增长策略）
			_dataBuffer.resize(std::max(requiredCapacity, _dataBuffer.size() * 2));
		}
	}

}


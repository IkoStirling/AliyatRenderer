#pragma once
#include "STPacketHeader.h"
#include <vector>

namespace Network
{
	class ByteBuffer {
	public:
		// 构造函数
		ByteBuffer() = default;
		explicit ByteBuffer(size_t initialCapacity) : _dataBuffer(initialCapacity) {}

	private:
		std::vector<uint8_t> _dataBuffer; // 底层存储
		size_t _dataSize = 0;             // 有效数据大小
		size_t _readPos = 0;              // 读取位置
	public:
		// 基础功能
		uint8_t* data() noexcept;
		const uint8_t* data() const noexcept;
		size_t size() const noexcept;
		size_t capacity() const noexcept;
		void clear() noexcept;

		// 扩容与调整
		void reserve(size_t capacity);
		void resize(size_t newSize);

		// 写入数据（支持任意类型）
		template<typename T>
		void write(const T& value);

		// 读取数据（支持任意类型）
		template<typename T>
		T read();

		// 流式写入（支持链式调用）
		template<typename T>
		ByteBuffer& operator<<(const T& data);

		// 流式读取（支持链式调用）
		template<typename T>
		ByteBuffer& operator>>(T& data);

		// 模板特化
		template<>
		void write<STPacketHeader>(const STPacketHeader& header);




		// 原始块操作
		void append(const uint8_t* data, size_t size);

		const uint8_t* readBytes(size_t size);

		// 读写位置控制
		size_t getReadPosition() const noexcept;
		void setReadPosition(size_t pos);

		size_t getWritePosition() const noexcept;
		void setWritePosition(size_t pos);

		// 字节序转换（大端序）
		template<typename T>
		T hostToNetwork(T value);

		template<typename T>
		T networkToHost(T value);

		// 确保容量足够
		void ensureCapacity(size_t requiredCapacity);

	};

	//****************************************************************************

	template<>
	inline void ByteBuffer::write<STPacketHeader>(const STPacketHeader& header)
	{
		write(header.packetId);
		write(header.payloadSize);
		write(header.checksum);
	}




	//****************************************************************************

	template<typename T>
	inline void ByteBuffer::write(const T& value)
	{
		static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value,
			"Write requires arithmetic or enum type");

		const size_t typeSize = sizeof(T);
		ensureCapacity(_dataSize + typeSize);

		// 字节序转换
		T netValue = hostToNetwork(value);
		std::memcpy(_dataBuffer.data() + _dataSize, &netValue, typeSize);
		_dataSize += typeSize;
	}

	template<typename T>
	inline T ByteBuffer::read()
	{
		static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value,
			"Read requires arithmetic or enum type");

		if (_readPos + sizeof(T) > _dataSize) {
			throw std::out_of_range("Read beyond buffer size");
		}

		T value;
		std::memcpy(&value, _dataBuffer.data() + _readPos, sizeof(T));
		_readPos += sizeof(T);

		// 字节序转换
		return networkToHost(value);
	}

	template<typename T>
	inline ByteBuffer& ByteBuffer::operator<<(const T& data)
	{
		write(data);
		return *this;
	}

	// 流式读取（支持链式调用）
	template<typename T>
	inline ByteBuffer& ByteBuffer::operator>>(T& data) {
		data = read<T>();
		return *this;
	}

	template<typename T>
	inline T ByteBuffer::hostToNetwork(T value) {
		if constexpr (std::is_same_v<T, uint16_t>) {
			return htons(value);  // 主机序 → 网络序（大端）
		}
		else if constexpr (std::is_same_v<T, uint32_t>) {
			return htonl(value);  // 主机序 → 网络序（大端）
		}
		else {
			return value;         // 其他类型不转换（如 uint8_t）
		}
	}

	template<typename T>
	inline T ByteBuffer::networkToHost(T value) {
		if constexpr (std::is_same_v<T, uint16_t>) {
			return ntohs(value);  // 网络序 → 主机序（大端）
		}
		else if constexpr (std::is_same_v<T, uint32_t>) {
			return ntohl(value);  // 网络序 → 主机序（大端）
		}
		else {
			return value;         // 其他类型不转换（如 uint8_t）
		}
	}

}


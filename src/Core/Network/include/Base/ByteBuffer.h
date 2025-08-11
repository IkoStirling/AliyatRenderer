#pragma once
#include "STPacketHeader.h"
#include <vector>

namespace Network
{
	class ByteBuffer {
	public:
		// ���캯��
		ByteBuffer() = default;
		explicit ByteBuffer(size_t initialCapacity) : _dataBuffer(initialCapacity) {}

	private:
		std::vector<uint8_t> _dataBuffer; // �ײ�洢
		size_t _dataSize = 0;             // ��Ч���ݴ�С
		size_t _readPos = 0;              // ��ȡλ��
	public:
		// ��������
		uint8_t* data() noexcept;
		const uint8_t* data() const noexcept;
		size_t size() const noexcept;
		size_t capacity() const noexcept;
		void clear() noexcept;

		// ���������
		void reserve(size_t capacity);
		void resize(size_t newSize);

		// д�����ݣ�֧���������ͣ�
		template<typename T>
		void write(const T& value);

		// ��ȡ���ݣ�֧���������ͣ�
		template<typename T>
		T read();

		// ��ʽд�루֧����ʽ���ã�
		template<typename T>
		ByteBuffer& operator<<(const T& data);

		// ��ʽ��ȡ��֧����ʽ���ã�
		template<typename T>
		ByteBuffer& operator>>(T& data);

		// ģ���ػ�
		template<>
		void write<STPacketHeader>(const STPacketHeader& header);




		// ԭʼ�����
		void append(const uint8_t* data, size_t size);

		const uint8_t* readBytes(size_t size);

		// ��дλ�ÿ���
		size_t getReadPosition() const noexcept;
		void setReadPosition(size_t pos);

		size_t getWritePosition() const noexcept;
		void setWritePosition(size_t pos);

		// �ֽ���ת���������
		template<typename T>
		T hostToNetwork(T value);

		template<typename T>
		T networkToHost(T value);

		// ȷ�������㹻
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

		// �ֽ���ת��
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

		// �ֽ���ת��
		return networkToHost(value);
	}

	template<typename T>
	inline ByteBuffer& ByteBuffer::operator<<(const T& data)
	{
		write(data);
		return *this;
	}

	// ��ʽ��ȡ��֧����ʽ���ã�
	template<typename T>
	inline ByteBuffer& ByteBuffer::operator>>(T& data) {
		data = read<T>();
		return *this;
	}

	template<typename T>
	inline T ByteBuffer::hostToNetwork(T value) {
		if constexpr (std::is_same_v<T, uint16_t>) {
			return htons(value);  // ������ �� �����򣨴�ˣ�
		}
		else if constexpr (std::is_same_v<T, uint32_t>) {
			return htonl(value);  // ������ �� �����򣨴�ˣ�
		}
		else {
			return value;         // �������Ͳ�ת������ uint8_t��
		}
	}

	template<typename T>
	inline T ByteBuffer::networkToHost(T value) {
		if constexpr (std::is_same_v<T, uint16_t>) {
			return ntohs(value);  // ������ �� �����򣨴�ˣ�
		}
		else if constexpr (std::is_same_v<T, uint32_t>) {
			return ntohl(value);  // ������ �� �����򣨴�ˣ�
		}
		else {
			return value;         // �������Ͳ�ת������ uint8_t��
		}
	}

}


#ifndef __SHARED_SERIALIZATION_BYTEBUFFER_H__
#define __SHARED_SERIALIZATION_BYTEBUFFER_H__

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <string>
#include <vector>
#include <map>

class ByteBuffer
{

public:
	ByteBuffer(const unsigned char* const data, size_t size);
	ByteBuffer(size_t buf_size = 512);
	~ByteBuffer();

public:
	void Kill(void);

	unsigned char* Buf(void);
	const unsigned char* Buf(void) const;

	unsigned char* RdPos(void);
	void AddRdPos(size_t bytes);

	void Length(int length);

	//���ص�ǰ�����ݳ���
	size_t Length(void);

	//��ȡʣ����ÿռ��С
	size_t Space(void);

	//��ȡ���ݴ洢�ռ��С
	size_t Size(void);

	//�ӵ�ǰ��Ч���������ڴ���ʼ��ַɾ��ָ������bytes�ֽ�����
	char* Drain(size_t bytes);

	template<typename T>
	bool Peek(T& rhs)
	{
		size_t bytes = sizeof(T);
		size_t data_len = this->wr_pos_ - this->rd_pos_;

		if (bytes > data_len)
		{
			return false;
		}
		else
		{
			rhs = (*((T*)(this->buf_ + this->rd_pos_)));
			return true;
		}
	}

public:
	ByteBuffer& operator <<(int8_t value);
	ByteBuffer& operator <<(int16_t value);
	ByteBuffer& operator <<(int32_t value);
	ByteBuffer& operator <<(int64_t value);

	ByteBuffer& operator <<(uint8_t value);
	ByteBuffer& operator <<(uint16_t value);
	ByteBuffer& operator <<(uint32_t value);
	ByteBuffer& operator <<(uint64_t value);

	ByteBuffer& operator <<(float value);
	ByteBuffer& operator <<(double value);

	ByteBuffer& operator <<(bool value);

	ByteBuffer& operator <<(const std::string& text)
	{
		uint16_t string_len = text.size();
		this->operator <<(string_len);
		memcpy(Grow(string_len), text.c_str(), string_len);
		return *this;
	}

	template<typename element>
	ByteBuffer& operator <<(const std::vector<element>& vec)
	{
		uint32_t count = vec.size();
		this->operator <<(count);

		for (typename std::vector<element>::const_iterator ite = vec.begin(); ite
			!= vec.end(); ++ite)
		{
			*this << *ite;
		}
		return *this;
	}

	template<typename T>
	ByteBuffer& Append(T* t, size_t size)
	{
		memcpy(Grow(size), (char*) t, size);
		return *this;
	}

public:
	ByteBuffer& operator >> (int8_t& value);
	ByteBuffer& operator >> (int16_t& value);
	ByteBuffer& operator >> (int32_t& value);
	ByteBuffer& operator >> (int64_t& value);

	ByteBuffer& operator >> (uint8_t& value);
	ByteBuffer& operator >> (uint16_t& value);
	ByteBuffer& operator >> (uint32_t& value);
	ByteBuffer& operator >> (uint64_t& value);

	ByteBuffer& operator >> (float& value);
	ByteBuffer& operator >> (double& value);

	ByteBuffer& operator >> (bool& value);

	ByteBuffer& operator >> (std::string& text)
	{
		uint16_t string_len = 0;
		this->operator >>(string_len);

		if (string_len > this->Length())
		{
			throw std::exception();
		}

		text = std::string((const char*)(this->buf_ + this->rd_pos_), string_len);
		this->Skip(string_len);

		return *this;
	};

	template <typename element>
	ByteBuffer& operator >> (std::vector<element>& vec)
	{
		uint32_t size = 0;
		this->operator >>(size);

		vec.clear();

		for (uint32_t index = 0; index < size; index++)
		{
			element items;
			*this >> items;
			vec.push_back(items);
		}

		return *this;
	};

private:
	void Skip(size_t bytes);
	unsigned char* Read(size_t bytes);

private:
	void Align(void);
	unsigned char* Grow(size_t bytes);
	void Expand(size_t bytes);

	template<typename T>
	ByteBuffer& Append(const T& t)
	{
		*((T*) Grow(sizeof(T))) = t;
		return *this;
	}

	template <typename T>
	ByteBuffer& Extract(T& t)
	{
		t = (*((T*)Read(sizeof(T))));
		return *this;
	};

protected:
	unsigned char *buf_; //��������(realloc)����������ڴ���ʼ��ַ
	size_t buf_size_; //������������������ڴ��ֽ���

	size_t rd_pos_;
	size_t wr_pos_;

private:
	ByteBuffer(const ByteBuffer&);
	const ByteBuffer& operator =(const ByteBuffer&);
};

#endif
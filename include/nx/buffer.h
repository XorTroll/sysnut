#pragma once

/** @file nx/buffer.h
*
*  @author Blake Warner
*
*  @license GPLv3
*/

#include "nut.h"
#include "nx/sptr.h"
#include "log.h"
#include "nx/integer.h"
#include <new>

#ifdef __SWITCH__
#include <string.h>
#endif

#define Array Buffer

template<class T>
T swapEndian(T s)
{
	T result;
	u8* dest = (u8*)&result;
	u8* src = (u8*)&s;
	for (unsigned int i = 0; i < sizeof(s); i++)
	{
		dest[i] = src[sizeof(s) - i - 1];
	}
	return result;
}

template<class T=u8, unsigned int BUFFER_ALIGN = 0xFF>
class Buffer
{
public:
	Buffer()
	{
		buffer() = NULL;
		size() = 0;
		bufferSize() = 0;
		resize(0);
	}

	~Buffer()
	{
		close();
	}

	Buffer(const Buffer<T>& src)
	{
		operator=(src);
	}

	Buffer(const void* src, u64 sz)
	{
		set(src, sz);
	}

	bool operator==(const Buffer<T>& src)
	{
		if (size() != src.size())
		{
			return false;
		}

		return memcmp(buffer(), src.buffer(), size()) == 0;
	}

	Buffer<T>& operator=(const Buffer<T>& src)
	{
		resize(src.size());
		resize(0);
		writeBuffer(src);
		//set(src.buffer(), src.size());
		return *this;
	}

	const T& operator[](u32 i) const
	{
		return m_buffer[i];
	}

	T& operator[](u32 i)
	{
		return m_buffer[i];
	}

	virtual u64 writeBuffer(const Buffer<T>& v)
	{
		return writeBuffer(v.buffer(), v.size() * sizeof(T));

		u64 originalSize = size();

		resize(originalSize + v.size());
		memcpy((void*)(buffer() + originalSize), v.buffer(), v.size() * sizeof(T));
		return v.size();

		/*const T* p = v.buffer();
		for (size_t i = 0; i < v.size(); i++)
		{
			push(p[i]);
		}
		return v.size();*/
	}

	virtual u64 writeBuffer(const void* p, u64 sz)
	{
		u64 originalSize = size();

		resize(originalSize + sz);
		memcpy((void*)(buffer() + originalSize), p, sz * sizeof(T));
		return sz;

		/*const T* p = v.buffer();
		for (size_t i = 0; i < v.size(); i++)
		{
		push(p[i]);
		}
		return v.size();*/
	}

	virtual void setEOF()
	{
	}

	Buffer<T>& operator+=(const Buffer<T>& v)
	{
		writeBuffer(v);
		return *this;
	}

	template<class K>
	Buffer<T>& operator+=(const K& v)
	{
		write<K>(v);
		return *this;
	}

	template<class K>
	u64 write(const K& v)
	{
		const u8* p = reinterpret_cast<const u8*>(&v);
		for (size_t i = 0; i < sizeof(v); i++)
		{
			push(p[i]);
		}
		return sizeof(v);
	}

	bool contains(const T& v)
	{
		for (u64 i = 0; i < size(); i++)
		{
			if (m_buffer[i] == v)
			{
				return true;
			}
		}
		return false;
	}

	T& first()
	{
		return (*this)[0];
	}

	T& last()
	{
		if (!size())
		{
			return (*this)[0];
		}
		return (*this)[size() - 1];
	}

	T& push(T n)
	{
		u64 sz = size();
		if (!resize(sz + 1))
		{
			fatal("failed to resize buffer!\n");
			return (*this)[sz-1];
		}
		m_buffer[sz] = n;
		return (*this)[sz];
	}

	T pop()
	{
		if (!size())
		{
			return T();
		}

		T n = m_buffer[size() - 1];
		resize(size() - 1);
		return n;
	}

	sptr<Buffer<T>> slice(s64 start, s64 end) const
	{
		sptr<Buffer<T>> out(new Buffer<T>());
		slice(*out, start, end);
		return out;
	}

	bool slice(Buffer<T>& out, s64 start, s64 end) const
	{
		u64 sz = end - start;

		if (!out.resize(sz))
		{
			return false;
		}

		memcpy(out.buffer(), buffer() + (start * sizeof(T)), (size_t)sz * sizeof(T));

		return true;
	}

	void initializeItems(u64 start, u64 end)
	{
		if (start == end)
		{
			return;
		}

		if (start < end)
		{
			for (u64 i = start; i < end; i++)
			{
				new((T*)&m_buffer[i]) T();
			}
		}
		else
		{
			for (u64 i = end; i < start; i++)
			{
				m_buffer[i].~T();
			}
		}
	}

	virtual bool resize(u64 newSize)
	{
		u64 originalSize = size();

		if (buffer() && newSize <= bufferSize())
		{
			size() = newSize;
			initializeItems(originalSize, newSize);
			return true;
		}

		u64 newBufferSize = (newSize / BUFFER_ALIGN) * BUFFER_ALIGN + BUFFER_ALIGN;

		T* newBuffer =  (T*)malloc((size_t)newBufferSize * sizeof(T));

		if (!newBuffer)
		{
			fatal("out of memory, tried to alloc %d bytes\n", (int)newSize);
			return false;
		}

		if (buffer())
		{
			memcpy((void*)newBuffer, (void*)buffer(), (size_t)size() * sizeof(T));
			free(buffer());
		}

		buffer() = newBuffer;
		size() = newSize;
		bufferSize() = newBufferSize;

		initializeItems(originalSize, newSize);
		return true;
	}

	const T* buffer(u64 i)
	{
		return (T*)buffer() + i;
	}

	const T* buffer() const { return m_buffer; }
	T*& buffer() { return m_buffer; }

	u64 size() const { return m_size; }
	u64& size() { return m_size;  }

	u64 sizeBytes() const { return size() * sizeof(T); }

	const u64& bufferSize() const { return m_bufferSize; }
	u64& bufferSize() { return m_bufferSize; }

	bool set(const void* src, u64 sz)
	{
		if (!resize(sz))
		{
			return false;
		}
		memcpy(buffer(), src, (size_t)sz);
		return true;
	}

	sptr<Buffer<T>> where(bool(*callback)(T& s))
	{
		sptr<Buffer<T>> results(new Buffer<T>());

		for (auto& f : *this)
		{
			if (callback(f))
			{
				results->push(f);
			}
		}

		return results;
	}

	bool close()
	{
		if (!buffer())
		{
			return false;
		}

		free((void*)buffer());
		size() = 0;
		bufferSize() = 0;
		buffer() = NULL;
		return true;
	}

	const char* c_str(u64 i = 0) const
	{
		if (!buffer())
		{
			return "";
		}

		return (const char*)buffer() + i;
	}

	void dump(int sz = 0, int offset = 0) const
	{
		if (!sz)
		{
			sz = size() - offset;
		}
		else
		{
			sz += offset;
		}

		for (unsigned long i = offset; i < sz; i++)
		{
			if ((i + 1) % 32 == 0)
			{
				debug("%2.2X\n", (unsigned int)c_str()[i]);
			}
			else if ((i + 1) % 4 == 0)
			{
				debug("%2.2X ", (unsigned int)c_str()[i]);
			}
			else
			{
				debug("%2.2X", (unsigned int)c_str()[i]);
			}
		}
		debug("\n");
	}

	const T* begin() const
	{
		return &m_buffer[0];
	}

	const T* end() const
	{
		return &m_buffer[size()];
	}

	T* begin()
	{
		return &m_buffer[0];
	}

	T* end()
	{
		return &m_buffer[size()];
	}

	/*
	integer<256> sha256()
	{
		integer<256> result(0);

		mbedtls_sha256_context ctx;
		mbedtls_sha256_init(&ctx);

		mbedtls_sha256_update(&ctx, buffer(), sizeBytes());

		mbedtls_sha256_finish(&ctx, (u8*)&result);

		mbedtls_sha256_free(&ctx);
		return result;
	}
	*/

protected:

	T* m_buffer = NULL;
	u64 m_size = 0;
	u64 m_bufferSize = 0;
};


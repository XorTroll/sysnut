#pragma once

#include "nx/file.h"
#include "nx/url.h"
#include "nx/buffer.h"
#include "nx/crypto.h"
#include "nx/integer.h"

#define PAGE_ALIGNMENT 0x10000

class BufferedFile;

class Page : public Buffer<u8>
{
public:
	bool& dirty() { return m_dirty; }
	u64& id() { return m_id; }
	bool& active() { return m_active; }

	bool contains(u64 offset, u64 size);
	u64 pageOffset() { return id() * PAGE_ALIGNMENT; }
	bool load(BufferedFile* f, u64 offset, u64 sz);

private:
	bool m_active = false;
	u64 m_id = 0;
	bool m_dirty = false;
};

class FileCrypto : public Crypto
{
public:
	FileCrypto();
	crypt_type_t& type() { return m_type; }
	integer<128>& key() { return m_key; }
private:
	crypt_type_t m_type;
	integer<128> m_key;
};

class BufferedFile : public File
{
public:
	BufferedFile();
	virtual ~BufferedFile();

	virtual bool open(Url path, const char* mode = "rb");
	virtual bool close();
	bool seek(u64 offset, int whence = 0);
	bool seekThrough(u64 offset, int whence = 0);
	bool rewind();
	u64 tell();
	virtual u64 read(Buffer<u8>& buffer, u64 sz = 0);
	u64 readThrough(Buffer<u8>& buffer, u64 sz = 0);

	const u64& currentPosition() const { return m_currentPosition; }
	u64& currentPosition() { return m_currentPosition; }

	FileCrypto& crypto() { return m_crypto; }
	bool setCrypto(crypt_type_t cryptoType, integer<128>& key);
	Page& page() { return m_page; }
private:
	Page m_page;
	u64 m_currentPosition = 0;
	FileCrypto m_crypto;
};
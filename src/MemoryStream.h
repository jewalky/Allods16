#pragma once

#include "Stream.h"

// a Stream backed by std::vector
class MemoryStream : public Stream
{
public:
	MemoryStream();

	virtual bool IsValid() { return true; }
	virtual bool IsEOF();
	virtual bool IsWritable() { return true; }
	virtual bool IsReadable() { return true; }
	virtual uint64_t GetLength();
	virtual uint64_t GetPosition();
	virtual uint64_t SetPosition(uint64_t position);

	// generic i/o
	virtual uint64_t ReadBytes(void* buffer, uint64_t count);
	virtual uint64_t WriteBytes(const void* buffer, uint64_t count);

	//
	void Clear();
	void SetBuffer(const std::vector<uint8_t>& buffer);
	void SetBuffer(const uint8_t* buffer, uint64_t count);
	const std::vector<uint8_t>& GetBuffer();

private:
	std::vector<uint8_t> mBuffer;
	uint64_t mPosition;

	MemoryStream(const MemoryStream& s) {};
};
#pragma once

#include <cstdint>
#include <string>
#include <vector>

class Stream
{
public:

	// base methods
	virtual bool IsValid() { return false; }
	operator bool() { return IsValid(); }

	virtual bool IsWritable() = 0;
	virtual bool IsReadable() = 0;
	virtual uint64_t GetLength() = 0;
	virtual uint64_t GetPosition() = 0;
	virtual uint64_t SetPosition(uint64_t position) = 0;
	virtual bool IsEOF() = 0;

	// generic i/o
	virtual uint64_t ReadBytes(void* buffer, uint64_t count) = 0;
	virtual uint64_t WriteBytes(const void* buffer, uint64_t count) = 0;
	
	uint64_t ReadBytes(std::vector<uint8_t>& vector, uint64_t count);
	uint64_t WriteBytes(const std::vector<uint8_t>& vector, uint64_t count);
	
	// binary i/o
	// read
	uint64_t ReadUInt64();
	int64_t ReadInt64() { return int64_t(ReadUInt64()); }
	uint32_t ReadUInt32();
	int32_t ReadInt32() { return int32_t(ReadUInt32()); }
	uint16_t ReadUInt16();
	int16_t ReadInt16() { return int16_t(ReadUInt16()); }
	uint8_t ReadUInt8();
	int8_t ReadInt8() { return int8_t(ReadUInt8()); }
	double_t ReadDouble();
	float_t ReadFloat();
	// write
	uint64_t WriteUInt64(uint64_t v);
	int64_t WriteInt64(int64_t v) { return int64_t(WriteUInt64(uint64_t(v))); }
	uint32_t WriteUInt32(uint32_t v);
	int32_t WriteInt32(int32_t v) { return int32_t(WriteUInt32(uint32_t(v))); }
	uint16_t WriteUInt16(uint16_t v);
	int16_t WriteInt16(int16_t v) { return int16_t(WriteUInt16(uint16_t(v))); }
	uint8_t WriteUInt8(uint8_t v);
	int8_t WriteInt8(int8_t v) { return int8_t(WriteUInt8(uint8_t(v))); }
	double_t WriteDouble(double_t v);
	float_t WriteFloat(float_t v);

	// text i/o
	char ReadChar() { return char(ReadUInt8()); }
	char WriteChar(char c) { return char(WriteUInt8(uint8_t(c))); }
	std::string ReadString(uint64_t len);
	std::string WriteString(const std::string& s, uint64_t len);

	// pure text i/o
	std::string ReadLine();
	std::string WriteLine(const std::string& line);

protected:
	Stream() {}
	virtual ~Stream() {}

private:
	Stream(const Stream& s) {};
};
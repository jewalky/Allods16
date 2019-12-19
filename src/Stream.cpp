#include "Stream.h"

uint64_t Stream::ReadBytes(std::vector<uint8_t>& vector, uint64_t count)
{
	vector.resize(count);
	return ReadBytes(vector.data(), count);
}

uint64_t Stream::WriteBytes(const std::vector<uint8_t>& vector, uint64_t count)
{
	return WriteBytes(vector.data(), count);
}

uint64_t Stream::ReadUInt64()
{
	uint64_t o;
	ReadBytes(&o, sizeof(uint64_t));
	return o;
}

uint32_t Stream::ReadUInt32()
{
	uint32_t o;
	ReadBytes(&o, sizeof(uint32_t));
	return o;
}

uint16_t Stream::ReadUInt16()
{
	uint16_t o;
	ReadBytes(&o, sizeof(uint16_t));
	return o;
}

uint8_t Stream::ReadUInt8()
{
	uint8_t o;
	ReadBytes(&o, sizeof(uint8_t));
	return o;
}

double_t Stream::ReadDouble()
{
	double_t o;
	ReadBytes(&o, sizeof(double_t));
	return o;
}

float_t Stream::ReadFloat()
{
	float_t o;
	ReadBytes(&o, sizeof(float_t));
	return o;
}

uint64_t Stream::WriteUInt64(uint64_t v)
{
	WriteBytes(&v, sizeof(uint64_t));
	return v;
}

uint32_t Stream::WriteUInt32(uint32_t v)
{
	WriteBytes(&v, sizeof(uint32_t));
	return v;
}

uint16_t Stream::WriteUInt16(uint16_t v)
{
	WriteBytes(&v, sizeof(uint16_t));
	return v;
}

uint8_t Stream::WriteUInt8(uint8_t v)
{
	WriteBytes(&v, sizeof(uint8_t));
	return v;
}

double_t Stream::WriteDouble(double_t v)
{
	WriteBytes(&v, sizeof(double_t));
	return v;
}

float_t Stream::WriteFloat(float_t v)
{
	WriteBytes(&v, sizeof(float_t));
	return v;
}

std::string Stream::ReadString(uint64_t len)
{
	char* buf = new char[len + 1];
	buf[len] = 0;
	ReadBytes(buf, len);
	std::string o(buf);
	delete buf;
	return o;
}

std::string Stream::WriteString(const std::string& s, uint64_t len)
{
	std::string w = s;
	w.reserve(len);
	WriteBytes(w.data(), len);
	w.shrink_to_fit();
	return w;
}

std::string Stream::ReadLine()
{
	std::string o;
	while (char c = ReadChar())
	{
		if (IsEOF()) break;
		if (c == '\n') break;
		if (c == '\r') continue;
		o += c;
	}
	return o;
}

std::string Stream::WriteLine(const std::string& line)
{
	WriteBytes(line.data(), line.length());
	WriteChar('\n');
	return line;
}

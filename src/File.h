#pragma once

#include "Stream.h"

enum class FileOpenFlags : uint16_t
{
	NoFlags = 0x0000,
	Read = 0x0001,
	Write = 0x0002,
	Truncate = 0x0004,
	Text = 0x0008
};

inline FileOpenFlags operator|(FileOpenFlags lhs, FileOpenFlags rhs)
{
	return static_cast<FileOpenFlags>(static_cast<uint16_t>(lhs) | static_cast<uint16_t>(rhs));
}

inline FileOpenFlags operator&(FileOpenFlags lhs, FileOpenFlags rhs)
{
	return static_cast<FileOpenFlags>(static_cast<uint16_t>(lhs) & static_cast<uint16_t>(rhs));
}

// a Stream backed by C file API
class File : public Stream
{
public:
	File(const std::string& path, FileOpenFlags flags);
	virtual ~File();

	bool Open();
	void Close();

	virtual bool IsValid();
	virtual bool IsEOF();
	virtual bool IsWritable();
	virtual bool IsReadable();
	virtual uint64_t GetLength();
	virtual uint64_t GetPosition();
	virtual uint64_t SetPosition(uint64_t position);
	virtual uint64_t SkipBytes(uint64_t num);

	// generic i/o
	virtual uint64_t ReadBytes(void* buffer, uint64_t count);
	virtual uint64_t WriteBytes(const void* buffer, uint64_t count);

private:
	std::string mPath;
	FileOpenFlags mFlags;
	void* mFile;

	File(const File& f) {};
};
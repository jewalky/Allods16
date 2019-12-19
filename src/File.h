#pragma once

#include "Stream.h"

enum FileOpenFlags
{
	File_OpenRead = 0x0001,
	File_OpenWrite = 0x0002,
	File_OpenTruncate = 0x0004,
	File_OpenText = 0x0008
};

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

	// generic i/o
	virtual uint64_t ReadBytes(void* buffer, uint64_t count);
	virtual uint64_t WriteBytes(const void* buffer, uint64_t count);

private:
	std::string mPath;
	FileOpenFlags mFlags;
	void* mFile;

	File(const File& f) {};
};
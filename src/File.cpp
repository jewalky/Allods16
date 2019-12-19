#include "File.h"
#include <cstdio>

File::File(const std::string& path, FileOpenFlags flags)
{
	mPath = path;
	mFlags = flags;
}

File::~File()
{
	Close();
}

bool File::Open()
{

	std::string fopenFlags = "";
	bool havePlus = false;
	
	if ((mFlags & (FileOpenFlags::Truncate|FileOpenFlags::Write)) == (FileOpenFlags::Truncate|FileOpenFlags::Write))
	{
		fopenFlags += "w";
		havePlus = (mFlags & FileOpenFlags::Read) != FileOpenFlags::NoFlags;
	}
	else
	{
		fopenFlags += "r";
		havePlus = true;
	}

	if ((mFlags & FileOpenFlags::Text) == FileOpenFlags::NoFlags)
		fopenFlags += "b";

	if (havePlus)
		fopenFlags += "+";

	fopen_s((FILE**)&mFile, mPath.c_str(), fopenFlags.c_str());
	return (mFile != nullptr);
		
}

void File::Close()
{
	if (mFile != nullptr)
	{
		fclose((FILE*)mFile);
		mFile = nullptr;
	}
}

bool File::IsValid()
{
	return (mFile != nullptr);
}

bool File::IsEOF()
{
	return feof((FILE*)mFile);
}

bool File::IsWritable()
{
	return (mFlags & FileOpenFlags::Write) != FileOpenFlags::NoFlags;
}

bool File::IsReadable()
{
	return (mFlags & FileOpenFlags::Read) != FileOpenFlags::NoFlags;
}

uint64_t File::GetLength()
{

	if (mFile == nullptr)
		return 0;
	
	uint64_t curPos = ftell((FILE*)mFile);
	fseek((FILE*)mFile, 0, SEEK_END);
	uint64_t len = ftell((FILE*)mFile);
	fseek((FILE*)mFile, long(curPos), SEEK_SET);
	return len;

}

uint64_t File::GetPosition()
{
	
	if (mFile == nullptr)
		return 0;

	return ftell((FILE*)mFile);

}

uint64_t File::SetPosition(uint64_t position)
{
	
	if (mFile == nullptr)
		return 0;

	fseek((FILE*)mFile, long(position), SEEK_SET);
	return ftell((FILE*)mFile);

}

uint64_t File::ReadBytes(void* buffer, uint64_t count)
{

	if (mFile == nullptr)
		return 0;

	if ((mFlags & FileOpenFlags::Read) == FileOpenFlags::NoFlags)
		return 0;

	return fread(buffer, 1, size_t(count), (FILE*)mFile);

}

uint64_t File::WriteBytes(const void* buffer, uint64_t count)
{
	
	if (mFile == nullptr)
		return 0;

	if ((mFlags & FileOpenFlags::Write) == FileOpenFlags::NoFlags)
		return 0;

	return fwrite(buffer, 1, size_t(count), (FILE*)mFile);

}

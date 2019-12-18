#include "MemoryStream.h"
#include <algorithm>
#include <cstring>

MemoryStream::MemoryStream()
{
	mPosition = 0;
}

bool MemoryStream::IsEOF()
{
	return mPosition >= mBuffer.size();
}

uint64_t MemoryStream::GetLength()
{
	return mBuffer.size();
}

uint64_t MemoryStream::GetPosition()
{
	return mPosition;
}

uint64_t MemoryStream::SetPosition(uint64_t position)
{
	return mPosition = position;
}

uint64_t MemoryStream::ReadBytes(void* buffer, uint64_t count)
{
	if (mPosition >= mBuffer.size())
		return 0;
	count = std::min(mBuffer.size() - mPosition, count);
	memcpy(buffer, mBuffer.data() + mPosition, count);
	mPosition += count;
	return count;
}

uint64_t MemoryStream::WriteBytes(const void* buffer, uint64_t count)
{
	uint64_t needSize = mPosition + count;
	if (needSize >= mBuffer.size())
		mBuffer.resize(needSize);
	memcpy(mBuffer.data() + mPosition, buffer, count);
	return count;
}

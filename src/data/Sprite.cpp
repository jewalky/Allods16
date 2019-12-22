#include "Sprite.h"

int Sprite::GetWidth(uint32_t index)
{
	if (index >= mFrames.size())
		return 0;
	return mFrames[index].mWidth;
}

int Sprite::GetHeight(uint32_t index)
{
	if (index >= mFrames.size())
		return 0;
	return mFrames[index].mHeight;
}

uint32_t Sprite::GetSize()
{
	return mFrames.size();
}

const Color* Sprite::GetPalette()
{
	if (mPalette.size() == 0)
		return nullptr;
	return mPalette.data();
}

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

const std::vector<Color>& Sprite::GetPalette()
{
	return mPalette;
}

#pragma once

#include <cstdint>
#include <vector>
#include "../draw/DrawingContext.h"

#define SPRITE_PALETTE_FLAG 0x80000000

class Sprite
{
public:
	virtual ~Sprite() {}

	int GetWidth(uint32_t index);
	int GetHeight(uint32_t index);
	uint32_t GetSize();

	virtual void Draw(DrawingContext& ctx, int32_t x, int32_t y, uint32_t index, const Color* palette) = 0;
	const Color* GetPalette();

protected:
	Sprite() {}

	struct SpriteFrame
	{
		uint32_t mWidth;
		uint32_t mHeight;
		std::vector<uint8_t> mData;
	};

	std::vector<SpriteFrame> mFrames;
	std::vector<Color> mPalette;

};
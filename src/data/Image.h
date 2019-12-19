#pragma once

#include <cstdint>
#include "../draw/DrawingContext.h"

#define IMAGE_NO_COLORKEY -1

class Image
{
public:
	virtual ~Image() {}

	virtual uint32_t GetWidth() = 0;
	virtual uint32_t GetHeight() = 0;

	virtual void Draw(DrawingContext& ctx, int32_t x, int32_t y, int32_t colorkey = IMAGE_NO_COLORKEY) = 0;
	virtual void DrawPartial(DrawingContext& ctx, int32_t x, int32_t y, const Rect& innerRect, int32_t colorkey = IMAGE_NO_COLORKEY) = 0;
	virtual void Blit(DrawingContext& ctx, int32_t x, int32_t y) = 0;
	virtual void BlitPartial(DrawingContext& ctx, int32_t x, int32_t y, const Rect& innerRect) = 0;

protected:
	Image() {}
};
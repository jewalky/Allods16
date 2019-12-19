#pragma once

#include "Image.h"
#include <string>
#include <vector>

class ImageTruecolor : public Image
{
public:
	ImageTruecolor(const std::string& path);
	ImageTruecolor(uint32_t w, uint32_t h);

	virtual uint32_t GetWidth();
	virtual uint32_t GetHeight();

	virtual void Draw(DrawingContext& ctx, int32_t x, int32_t y, int32_t colorkey = IMAGE_NO_COLORKEY) { DrawPartial(ctx, x, y, Rect::FromXYWH(0, 0, mWidth, mHeight), colorkey); }
	virtual void DrawPartial(DrawingContext& ctx, int32_t x, int32_t y, const Rect& innerRect, int32_t colorkey = IMAGE_NO_COLORKEY);
	virtual void Blit(DrawingContext& ctx, int32_t x, int32_t y) { BlitPartial(ctx, x, y, Rect::FromXYWH(0, 0, mWidth, mHeight)); }
	virtual void BlitPartial(DrawingContext& ctx, int32_t x, int32_t y, const Rect& innerRect);

	void SetSize(uint32_t w, uint32_t h);
	void FromScreen(const Rect& screenRect);
	Color* GetBuffer();

private:
	uint32_t mWidth;
	uint32_t mHeight;
	std::vector<Color> mPixels;
};

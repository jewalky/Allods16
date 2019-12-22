#pragma once

#include "Image.h"
#include <string>
#include <vector>
#include "../screen/Color.h"

class ImagePaletted : public Image
{
public:
	ImagePaletted(const std::string& path);
	ImagePaletted(uint32_t w, uint32_t h);
	ImagePaletted(uint32_t w, uint32_t h, const std::vector<Color>& palette);

	virtual uint32_t GetWidth();
	virtual uint32_t GetHeight();

	virtual void Draw(DrawingContext& ctx, int32_t x, int32_t y, int32_t colorkey = IMAGE_NO_COLORKEY) { DrawPartial(ctx, x, y, Rect::FromXYWH(0, 0, mWidth, mHeight), colorkey); }
	virtual void DrawPartial(DrawingContext& ctx, int32_t x, int32_t y, const Rect& innerRect, int32_t colorkey = IMAGE_NO_COLORKEY);
	virtual void Blit(DrawingContext& ctx, int32_t x, int32_t y) { BlitPartial(ctx, x, y, Rect::FromXYWH(0, 0, mWidth, mHeight)); }
	virtual void BlitPartial(DrawingContext& ctx, int32_t x, int32_t y, const Rect& innerRect);

	uint8_t GetPixelAt(uint32_t x, uint32_t y);
	uint8_t* GetBuffer();
	const Color* GetPalette();

	void SetSize(uint32_t w, uint32_t h);
	void MoveInPlace(int32_t offsX, int32_t offsY);

private:
	uint32_t mWidth;
	uint32_t mHeight;
	std::vector<uint8_t> mPixels;
	std::vector<Color> mPalette;
};
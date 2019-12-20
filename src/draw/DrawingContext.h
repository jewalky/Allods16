#pragma once

#include "../screen/Screen.h"

class ImageTruecolor;
class DrawingContext
{
public:
	DrawingContext(Screen* screen);
	DrawingContext(Screen* screen, const Rect& subrect);

	DrawingContext(ImageTruecolor* image);
	DrawingContext(ImageTruecolor* image, const Rect& subrect);

	Color* GetBuffer();
	// this is NOT in bytes, this is in pixels
	int32_t GetPitch();
	Rect GetViewport();

	// primitive drawing
	void DrawPoint(const Point& p, Color c);
	void DrawLine(const Point& v1, const Point& v2, Color c, uint32_t mask = 0xFFFFFFFF);
	void DrawRect(const Rect& rec, Color c);
	// does not do alpha blending
	void ClearRect(const Rect& rec, Color c);

	//
	inline static void AlphaBlend(const Color& c, Color& background)
	{
		background.components.r = c.components.r * c.components.a / 255 + background.components.r * (255 - c.components.a) / 255;
		background.components.g = c.components.g * c.components.a / 255 + background.components.g * (255 - c.components.a) / 255;
		background.components.b = c.components.b * c.components.a / 255 + background.components.b * (255 - c.components.a) / 255;
		background.components.a = 255;
	}

	inline static void AlphaBlendHalf(const Color& c, Color& background)
	{
		background.value = ((background.value & 0xFEFEFE) >> 1) + ((c.value & 0xFEFEFE) >> 1);
	}

	inline static void AlphaBlend16(const Color& c, Color& background)
	{
		background.components.r = c.components.r * c.components.a / 15 + background.components.r * (15 - c.components.a) / 15;
		background.components.g = c.components.g * c.components.a / 15 + background.components.g * (15 - c.components.a) / 15;
		background.components.b = c.components.b * c.components.a / 15 + background.components.b * (15 - c.components.a) / 15;
		background.components.a = 255;
	}

private:
	Color* mBuffer;
	int32_t mPitch;
	Rect mViewport;
};
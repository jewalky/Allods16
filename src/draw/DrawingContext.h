#pragma once

#include "../screen/Screen.h"

class DrawingContext
{
public:
	DrawingContext(Screen* screen);
	DrawingContext(Screen* screen, Rect subrect);

	Color* GetBuffer();
	// this is NOT in bytes, this is in pixels
	int GetPitch();
	Rect GetViewport();

	// primitive drawing
	void DrawPoint(const Point& p, Color c);
	void DrawLine(const Point& v1, const Point& v2, Color c);
	void DrawRect(const Rect& rec, Color c);
	// does not do alpha blending
	void ClearRect(const Rect& rec, Color c);

	//
	inline static void AlphaBlend(Color& c, Color& background)
	{
		background.components.r = c.components.r * c.components.a / 255 + background.components.r * (255 - c.components.a) / 255;
		background.components.g = c.components.g * c.components.a / 255 + background.components.g * (255 - c.components.a) / 255;
		background.components.b = c.components.b * c.components.a / 255 + background.components.b * (255 - c.components.a) / 255;
		background.components.a = 255;
	}

private:
	Screen* mScreen;
	int mPitch;
	Rect mViewport;
};
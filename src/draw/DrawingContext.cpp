#include "DrawingContext.h"
#include "../logging.h"
#include "../data/ImageTruecolor.h"
#include <cstdlib>
#include <algorithm>

DrawingContext::DrawingContext(Screen* s)
{
	mViewport = s->GetViewport();
	mPitch = mViewport.w;
    mBuffer = s->GetBuffer();
}

DrawingContext::DrawingContext(Screen* s, const Rect& viewport)
{
	Rect screenViewport = s->GetViewport();
	mViewport = screenViewport.GetIntersection(viewport);
	mPitch = screenViewport.w;
    mBuffer = s->GetBuffer();
}

DrawingContext::DrawingContext(ImageTruecolor* image)
{
    mViewport = Rect::FromXYWH(0, 0, image->GetWidth(), image->GetHeight());
    mPitch = mViewport.w;
    mBuffer = image->GetBuffer();
}

DrawingContext::DrawingContext(ImageTruecolor* image, const Rect& viewport)
{
    Rect imageViewport = Rect::FromXYWH(0, 0, image->GetWidth(), image->GetHeight());
    mViewport = imageViewport.GetIntersection(viewport);
    mPitch = mViewport.w;
    mBuffer = image->GetBuffer();
}

Color* DrawingContext::GetBuffer()
{
    return mBuffer;
}

int32_t DrawingContext::GetPitch()
{
	return mPitch;
}

Rect DrawingContext::GetViewport()
{
	return mViewport;
}

void DrawingContext::DrawPoint(const Point& p, Color c)
{
	if (c.components.a == 0)
		return;
	if (!mViewport.Contains(p))
		return;

	Color* buf = GetBuffer() + p.y * mPitch + p.x;
	
	if (c.components.a == 255)
	{
		*buf = c;
		return;
	}

    AlphaBlend(c, *buf);
}

void DrawingContext::DrawLine(const Point& v1, const Point& v2, Color c)
{

	int dx = abs(v1.x - v2.x), dy = abs(v1.y - v2.y);
	int p = 2 * dy - dx;
	int twoDy = 2 * dy, twoDyDx = 2 * (dy - dx);
	int x, y, xEnd;
	/*Determine which points to start and End */
	if (v1.x > v2.x) {
		x = v2.x;
		y = v2.y;
		xEnd = v1.x;
	}
	else {
		x = v1.x; y = v1.y; xEnd = v2.x;
	}

	int ydiff = 1;
	if (v2.y < v1.y)
		ydiff = -1;
	
	Color* buffer = GetBuffer() + y * mPitch + x;

	if (v1.x == v2.x)
	{
		if (v1.x < mViewport.GetLeft() || v1.x >= mViewport.GetRight())
			return;
		int32_t yMin = std::min(v1.y, v2.y);
		int32_t yMax = std::max(v1.y, v2.y);
		int32_t from = std::max(mViewport.GetTop(), yMin);
		int32_t to = std::min(mViewport.GetBottom(), yMax);
		buffer += (from - yMin) * mPitch;
		for (int32_t y = from; y < to; y++)
		{
			AlphaBlend(c, *buffer);
			buffer += mPitch;
		}
		return;
	}
	else if (v1.y == v2.y)
	{
		if (v1.y < mViewport.GetTop() || v1.y >= mViewport.GetBottom())
			return;
		int32_t xMin = std::min(v1.x, v2.x);
		int32_t xMax = std::max(v1.x, v2.x);
		int32_t from = std::max(mViewport.GetLeft(), xMin);
		int32_t to = std::min(mViewport.GetRight(), xMax);
		buffer += (from - xMin);
		for (int32_t x = from; x < to; x++)
			AlphaBlend(c, *buffer++);
		return;
	}

	if (mViewport.Contains(Point(x, y)))
		AlphaBlend(c, *buffer);

	while (x < xEnd) {
		x++;
		buffer++;
		if (p < 0) {
			p = p + twoDy;
		}
		else {
			y += ydiff;
			buffer += mPitch * ydiff;
			p = p + twoDyDx;
		}
		if (mViewport.Contains(Point(x, y)))
			AlphaBlend(c, *buffer);
	}


}

void DrawingContext::DrawRect(const Rect& rec, Color c)
{
    Rect drawRec = mViewport.GetIntersection(rec);
    Color* buf = GetBuffer() + drawRec.y * mPitch + drawRec.y;
    for (int y = drawRec.GetTop(); y < drawRec.GetBottom(); y++)
    {
        for (int x = 0; x < drawRec.w; x++)
            AlphaBlend(c, *buf++);
        buf += mPitch - drawRec.w;
    }
}

void DrawingContext::ClearRect(const Rect& rec, Color c)
{
	Rect drawRec = mViewport.GetIntersection(rec);
	Color* buf = GetBuffer() + drawRec.y * mPitch + drawRec.y;
	for (int y = drawRec.GetTop(); y < drawRec.GetBottom(); y++)
	{
		for (int x = 0; x < drawRec.w; x++)
			*buf++ = c;
		buf += mPitch - drawRec.w;
	}
}
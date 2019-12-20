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

static int _sign(int x)
{
    if (x < 0) return -1;
    if (x > 0) return 1;
    return 0;
}

void DrawingContext::DrawLine(const Point& v1, const Point& v2, Color c, uint32_t mask)
{

	int x1 = v1.x, y1 = v1.y;
	int x2 = v2.x, y2 = v2.y;

    // Distance of the line
    int dx = x2 - x1;
    int dy = y2 - y1;

    // Positive (absolute) distance
    int dxabs = abs(dx);
    int dyabs = abs(dy);

    // Half distance
    int x = dyabs >> 1;
    int y = dxabs >> 1;

    // Direction
    int sdx = _sign(dx);
    int sdy = _sign(dy);

    // Start position
    int px = x1;
    int py = y1;

    // Draw first pixel
    if ((px >= mViewport.GetLeft()) && (px < mViewport.GetRight()) && (py >= mViewport.GetTop()) && (py < mViewport.GetBottom()))
        AlphaBlend(c, *(mBuffer + py * mPitch + px));

    // Check if the line is more horizontal than vertical
    if (dxabs >= dyabs)
    {
        for (int i = 0; i < dxabs; i++)
        {
            y += dyabs;
            if (y >= dxabs)
            {
                y -= dxabs;
                py += sdy;
            }
            px += sdx;

            // Draw pixel
            if ((mask & (1 << (i & 0x7))) != 0)
            {
                if ((px >= mViewport.GetLeft()) && (px < mViewport.GetRight()) && (py >= mViewport.GetTop()) && (py < mViewport.GetBottom()))
                    AlphaBlend(c, *(mBuffer + py * mPitch + px));
            }
        }
    }
    // Else the line is more vertical than horizontal
    else
    {
        for (int i = 0; i < dyabs; i++)
        {
            x += dxabs;
            if (x >= dyabs)
            {
                x -= dyabs;
                px += sdx;
            }
            py += sdy;

            // Draw pixel
            if ((mask & (1 << (i & 0x7))) != 0)
            {
                if ((px >= mViewport.GetLeft()) && (px < mViewport.GetRight()) && (py >= mViewport.GetTop()) && (py < mViewport.GetBottom()))
                    AlphaBlend(c, *(mBuffer + py * mPitch + px));
            }
        }
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
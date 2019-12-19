#include "DrawingContext.h"
#include "../logging.h"
#include "../data/ImageTruecolor.h"

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

    int dx = v2.x - v1.x;
    int dy = v2.y - v1.y;
    float rateXY = 0.0;
    if (v2.y != v1.y)
        rateXY = float(dx) / dy;

    int x1 = v1.x, y1 = v1.y;
    int x2 = v2.x, y2 = v2.y;

    for (int i = 0; i < 2; i++)
    {
        // check x1,y1
        if (y1 < mViewport.GetTop())
        {
            x1 += (mViewport.GetTop() - y1) * rateXY;
            y1 = mViewport.GetTop();
        }
        if (x1 < mViewport.GetLeft())
        {
            if (rateXY != 0) y1 += (mViewport.GetLeft() - x1) / rateXY;
            x1 = mViewport.GetLeft();
        }
        // check x2,y2
        if (y2 < mViewport.GetTop())
        {
            x2 += (mViewport.GetTop() - y2) * rateXY;
            y2 = mViewport.GetTop();
        }
        if (x2 < mViewport.GetLeft())
        {
            if (rateXY != 0) y2 += (mViewport.GetLeft() - x2) / rateXY;
            x2 = mViewport.GetLeft();
        }
        // check x1,y1
        if (y1 >= mViewport.GetBottom())
        {
            x1 -= (y1 - (mViewport.GetBottom())) * rateXY;
            y1 = mViewport.GetBottom()-1;
        }
        if (x1 >= mViewport.GetRight())
        {
            if (rateXY != 0) y1 -= (x1 - (mViewport.GetRight())) / rateXY;
            x1 = mViewport.GetRight()-1;
        }
        // check x2,y2
        if (y2 >= mViewport.GetBottom())
        {
            x2 -= (y2 - (mViewport.GetBottom())) * rateXY;
            y2 = mViewport.GetBottom()-1;
        }
        if (x2 >= mViewport.GetRight())
        {
            if (rateXY != 0) y2 -= (x2 - (mViewport.GetRight())) / rateXY;
            x2 = mViewport.GetRight()-1;
        }
    }

    if ((x1 == x2 && (x1 == mViewport.GetLeft() || x1 == mViewport.GetRight())) ||
        (y1 == y2 && (y1 == mViewport.GetLeft() || y1 == mViewport.GetRight())))
    {
        return;
    }

    dx = x2 - x1;
    dy = y2 - y1;

    //
    // brezenham
    Color* buf = GetBuffer() + y1 * mPitch + x1;
    if (y1 == y2)
    {
        for (int x = x1; x <= x2; x++)
            AlphaBlend(c, *buf++);
    }
    else if (x1 == x2)
    {
        for (int y = y1; y <= y2; y++)
        {
            AlphaBlend(c, *buf);
            buf += mPitch;
        }
    }
    else
    {
        int pitchOffset = mPitch;
        if (y2 < y1)
            pitchOffset = -pitchOffset;
        int xOffset = 1;
        if (x2 < x1)
            xOffset = -1;

        int countX = dx;
        if (countX < 0)
            countX = -countX;

        int p = 2 * dy - dx;

        for (int x = 0; x < countX; x++)
        {
            if ((p >= 0) == (xOffset > 0))
            {
                AlphaBlend(c, *buf);
                buf += pitchOffset;
                p = p + 2 * dy - 2 * dx;
            }
            else
            {
                AlphaBlend(c, *buf);
                p = p + 2 * dy;
            }
            buf += xOffset;
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
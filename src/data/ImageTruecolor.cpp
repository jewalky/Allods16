#include "ImageTruecolor.h"
#include "../Application.h"
#include "../logging.h"

ImageTruecolor::ImageTruecolor(const std::string& path)
{

	MemoryStream ms;
	if (!Application::GetInstance()->GetResources()->ReadFile(ms, path))
		Application::GetInstance()->Abort(Format("Critical: couldn't load \"%s\"", path));
	
	SDL_RWops* rw = SDL_RWFromConstMem(ms.GetBuffer().data(), ms.GetLength());
	SDL_Surface* bmpUnprocessed = SDL_LoadBMP_RW(rw, 1);
	
	if (bmpUnprocessed == nullptr)
		Application::GetInstance()->Abort(Format("Critical: couldn't load \"%s\": %s", path, SDL_GetError()));

	SDL_PixelFormat pfd;
	pfd.palette = NULL;
	pfd.BitsPerPixel = 32;
	pfd.BytesPerPixel = 4;
	pfd.Rmask = 0x00FF0000;
	pfd.Gmask = 0x0000FF00;
	pfd.Bmask = 0x000000FF;
	pfd.Amask = 0xFF000000;
	pfd.Rshift = 0;
	pfd.Gshift = 8;
	pfd.Bshift = 16;
	pfd.Ashift = 24;
	pfd.Rloss = 0;
	pfd.Gloss = 0;
	pfd.Bloss = 0;
	pfd.Aloss = 0;

	SDL_Surface* bmpConverted = SDL_ConvertSurface(bmpUnprocessed, &pfd, 0);
	SDL_FreeSurface(bmpUnprocessed);
	if (bmpConverted == nullptr)
		Application::GetInstance()->Abort(Format("Critical: couldn't load \"%s\": %s", path, SDL_GetError()));

	mWidth = bmpConverted->w;
	mHeight = bmpConverted->h;
	mPixels.resize(mWidth * mHeight);
	memcpy(mPixels.data(), bmpConverted->pixels, mWidth * mHeight * sizeof(Color));

	SDL_FreeSurface(bmpConverted);
	
}

ImageTruecolor::ImageTruecolor(uint32_t w, uint32_t h)
{
	mWidth = w;
	mHeight = h;
}

uint32_t ImageTruecolor::GetWidth()
{
	return mWidth;
}

uint32_t ImageTruecolor::GetHeight()
{
	return mHeight;
}

void ImageTruecolor::DrawPartial(DrawingContext& ctx, int32_t x, int32_t y, const Rect& innerRect, int32_t colorkey)
{
	// first, take rect in screen space and clip it to viewport
	Rect viewRec = ctx.GetViewport();
	Rect screenRec = Rect::FromXYWH(x, y, innerRect.w, innerRect.h).GetIntersection(viewRec);
	if (screenRec.w > mWidth) screenRec.w = mWidth;
	if (screenRec.h > mHeight) screenRec.h = mHeight;
	Rect clipRec = Rect::FromXYWH(screenRec.x - x + innerRect.x, screenRec.y - y + innerRect.y, screenRec.w, screenRec.h).GetIntersection(Rect::FromXYWH(0, 0, mWidth, mHeight));
	
	Color* screenBuffer = ctx.GetBuffer() + screenRec.y * ctx.GetPitch() + screenRec.x;
	Color* buffer = mPixels.data() + clipRec.y * mWidth + clipRec.x;

	for (int y = clipRec.GetTop(); y < clipRec.GetBottom(); y++)
	{
		for (int x = clipRec.GetLeft(); x < clipRec.GetRight(); x++)
			DrawingContext::AlphaBlend(*buffer++, *screenBuffer++);
		screenBuffer += ctx.GetPitch() - clipRec.w;
		buffer += mWidth - clipRec.w;
	}
}

void ImageTruecolor::BlitPartial(DrawingContext& ctx, int32_t x, int32_t y, const Rect& innerRect)
{
	// first, take rect in screen space and clip it to viewport
	Rect viewRec = ctx.GetViewport();
	Rect screenRec = Rect::FromXYWH(x, y, innerRect.w, innerRect.h).GetIntersection(viewRec);
	if (screenRec.w > mWidth) screenRec.w = mWidth;
	if (screenRec.h > mHeight) screenRec.h = mHeight;
	Rect clipRec = Rect::FromXYWH(screenRec.x - x + innerRect.x, screenRec.y - y + innerRect.y, screenRec.w, screenRec.h).GetIntersection(Rect::FromXYWH(0, 0, mWidth, mHeight));

	Color* screenBuffer = ctx.GetBuffer() + screenRec.y * ctx.GetPitch() + screenRec.x;
	Color* buffer = mPixels.data() + clipRec.y * mWidth + clipRec.x;

	for (int y = clipRec.GetTop(); y < clipRec.GetBottom(); y++)
	{
		for (int x = clipRec.GetLeft(); x < clipRec.GetRight(); x++)
			*screenBuffer++ = *buffer++;
		screenBuffer += ctx.GetPitch() - clipRec.w;
		buffer += mWidth - clipRec.w;
	}
}

void ImageTruecolor::FromScreen(const Rect& screenRect)
{

	Screen* screen = Application::GetInstance()->GetScreen();
	Rect viewport = screen->GetViewport();
	Rect clipRect = viewport.GetIntersection(screenRect);
	mWidth = clipRect.w;
	mHeight = clipRect.h;
	mPixels.resize(clipRect.w * clipRect.h);
	Color* screenBuffer = screen->GetBuffer() + clipRect.y * viewport.w + clipRect.x;
	Color* buffer = mPixels.data();
	for (int y = clipRect.GetTop(); y < clipRect.GetBottom(); y++)
	{
		for (int x = clipRect.GetLeft(); x < clipRect.GetRight(); x++)
			*buffer++ = *screenBuffer++;
		screenBuffer += viewport.w - clipRect.w;
	}

}

void ImageTruecolor::SetSize(uint32_t w, uint32_t h)
{
	mWidth = w;
	mHeight = h;
	mPixels.resize(w * h);
}

Color* ImageTruecolor::GetBuffer()
{
	return mPixels.data();
}

void ImageTruecolor::MoveInPlace(int32_t offsX, int32_t offsY)
{
	if (offsX == 0 && offsY == 0)
		return;

	if (offsX + int32_t(mWidth) <= 0 || offsY + int32_t(mHeight) <= 0 || offsX > int32_t(mWidth) || offsY > int32_t(mHeight))
		return;

	Color* buffer = mPixels.data();
	bool isBackwards = int32_t(mWidth) * offsY + offsX >= 0;
	Rect copyRect = Rect::FromXYWH(offsX, offsY, mWidth, mHeight).GetIntersection(Rect::FromXYWH(0, 0, mWidth, mHeight));

	if (!isBackwards)
	{
		Color* copyTo = buffer + copyRect.y * mWidth + copyRect.x;
		buffer = buffer + (copyRect.y-offsY) * mWidth + (copyRect.x-offsX);
		for (int y = copyRect.GetTop(); y < copyRect.GetBottom(); y++)
		{
			for (int x = copyRect.GetLeft(); x < copyRect.GetRight(); x++)
				*copyTo++ = *buffer++;
			copyTo += mWidth - copyRect.w;
			buffer += mWidth - copyRect.w;
		}
	}
	else
	{
		Color* copyTo = buffer + mWidth * mHeight - (copyRect.y-offsY) * mWidth - (copyRect.x-offsX);
		buffer = buffer + mWidth * mHeight - copyRect.y * mWidth - copyRect.x;
		for (int y = copyRect.GetBottom() - 1; y >= copyRect.GetTop(); y--)
		{
			for (int x = copyRect.GetRight() - 1; x >= copyRect.GetLeft(); x--)
				*copyTo-- = *buffer--;
			copyTo -= mWidth - copyRect.w;
			buffer -= mWidth - copyRect.w;
		}
	}
}

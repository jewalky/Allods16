#include "ImagePaletted.h"
#include "../Application.h"
#include "../MemoryStream.h"

ImagePaletted::ImagePaletted(const std::string& path)
{

	MemoryStream ms;
	if (!Application::GetInstance()->GetResources()->ReadFile(ms, path))
		Application::GetInstance()->Abort(Format("Critical: couldn't load \"%s\"", path));

	SDL_RWops* rw = SDL_RWFromConstMem(ms.GetBuffer().data(), ms.GetLength());
	SDL_Surface* bmpUnprocessed = SDL_LoadBMP_RW(rw, 1);

	if (!bmpUnprocessed)
		Application::GetInstance()->Abort(Format("Critical: couldn't load \"%s\": %s", path, SDL_GetError()));

	if (bmpUnprocessed->format->BitsPerPixel != 8 ||
		bmpUnprocessed->format->BytesPerPixel != 1 ||
		bmpUnprocessed->format->palette == nullptr)
	{
		SDL_FreeSurface(bmpUnprocessed);
		Application::GetInstance()->Abort(Format("Critical: couldn't load \"%s\": Image is not paletted", path));
	}

	mPalette.resize(256);
	SDL_Palette* sdlPal = bmpUnprocessed->format->palette;
	for (int i = 0; i < sdlPal->ncolors; i++)
		mPalette[i] = Color(sdlPal->colors[i].r, sdlPal->colors[i].g, sdlPal->colors[i].b, sdlPal->colors[i].a);

	mWidth = bmpUnprocessed->w;
	mHeight = bmpUnprocessed->h;
	mPixels.resize(mWidth * mHeight);

	memcpy(mPixels.data(), bmpUnprocessed->pixels, mWidth * mHeight);

	SDL_FreeSurface(bmpUnprocessed);

}

ImagePaletted::ImagePaletted(uint32_t w, uint32_t h)
{
	SetSize(w, h);
	mPalette.resize(256);
	for (int i = 0; i < 255; i++)
		mPalette[i] = Color(i, i, i, 255);
}

ImagePaletted::ImagePaletted(uint32_t w, uint32_t h, const std::vector<Color>& palette)
{
	SetSize(w, h);
	mPalette = palette;
	mPalette.resize(256);
}

uint32_t ImagePaletted::GetWidth()
{
	return mWidth;
}

uint32_t ImagePaletted::GetHeight()
{
	return mHeight;
}

void ImagePaletted::DrawPartial(DrawingContext& ctx, int32_t x, int32_t y, const Rect& innerRect, int32_t colorkey)
{

	// first, take rect in screen space and clip it to viewport
	Rect viewRec = ctx.GetViewport();
	Rect screenRec = Rect::FromXYWH(x, y, innerRect.w, innerRect.h).GetIntersection(viewRec);
	if (screenRec.w > mWidth) screenRec.w = mWidth;
	if (screenRec.h > mHeight) screenRec.h = mHeight;
	Rect clipRec = Rect::FromXYWH(screenRec.x - x + innerRect.x, screenRec.y - y + innerRect.y, screenRec.w, screenRec.h).GetIntersection(Rect::FromXYWH(0, 0, mWidth, mHeight));

	Color* screenBuffer = ctx.GetBuffer() + screenRec.y * ctx.GetPitch() + screenRec.x;
	uint8_t* buffer = mPixels.data() + clipRec.y * mWidth + clipRec.x;

	if (colorkey < 0)
	{
		for (int y = clipRec.GetTop(); y < clipRec.GetBottom(); y++)
		{
			for (int x = clipRec.GetLeft(); x < clipRec.GetRight(); x++)
				*screenBuffer++ = mPalette[*buffer++];
			screenBuffer += ctx.GetPitch() - clipRec.w;
			buffer += mWidth - clipRec.w;
		}
	}
	else
	{
		for (int y = clipRec.GetTop(); y < clipRec.GetBottom(); y++)
		{
			for (int x = clipRec.GetLeft(); x < clipRec.GetRight(); x++)
			{
				Color c = mPalette[*buffer];
				if ((c.value & 0xF0F0F0) != (colorkey & 0xF0F0F0))
					*screenBuffer = c;
				buffer++;
				screenBuffer++;
			}
			screenBuffer += ctx.GetPitch() - clipRec.w;
			buffer += mWidth - clipRec.w;
		}
	}

}

void ImagePaletted::BlitPartial(DrawingContext& ctx, int32_t x, int32_t y, const Rect& innerRect)
{
	DrawPartial(ctx, x, y, innerRect, -1);
}

uint8_t ImagePaletted::GetPixelAt(uint32_t x, uint32_t y)
{
	if (x >= mWidth || y >= mHeight)
		return 0;
	return mPixels[y * mWidth + x];
}

uint8_t* ImagePaletted::GetBuffer()
{
	return mPixels.data();
}

const std::vector<Color>& ImagePaletted::GetPalette()
{
	return mPalette;
}

void ImagePaletted::SetSize(uint32_t w, uint32_t h)
{
	mWidth = w;
	mHeight = h;
	mPixels.resize(w * h);
}

void ImagePaletted::MoveInPlace(int32_t offsX, int32_t offsY)
{
	if (offsX == 0 && offsY == 0)
		return;

	if (offsX + int32_t(mWidth) <= 0 || offsY + int32_t(mHeight) <= 0 || offsX > int32_t(mWidth) || offsY > int32_t(mHeight))
		return;

	uint8_t* buffer = mPixels.data();
	bool isBackwards = int32_t(mWidth) * offsY + offsX >= 0;
	Rect copyRect = Rect::FromXYWH(offsX, offsY, mWidth, mHeight).GetIntersection(Rect::FromXYWH(0, 0, mWidth, mHeight));

	if (!isBackwards)
	{
		uint8_t* copyTo = buffer + copyRect.y * mWidth + copyRect.x;
		buffer = buffer + (copyRect.y - offsY) * mWidth + (copyRect.x - offsX);
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
		uint8_t* copyTo = buffer + mWidth * mHeight - (copyRect.y - offsY) * mWidth - (copyRect.x - offsX) - 1;
		buffer = buffer + mWidth * mHeight - copyRect.y * mWidth - copyRect.x - 1;
		for (int y = copyRect.GetBottom() - 1; y >= copyRect.GetTop(); y--)
		{
			for (int x = copyRect.GetRight() - 1; x >= copyRect.GetLeft(); x--)
				*copyTo-- = *buffer--;
			copyTo -= mWidth - copyRect.w;
			buffer -= mWidth - copyRect.w;
		}
	}
}

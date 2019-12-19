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

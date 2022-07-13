#include "Sprite16A.h"
#include "../Application.h"
#include "../utils.h"
#include "../logging.h"

Sprite16A::Sprite16A(const std::string& path)
{

	MemoryStream ms;
	if (!Application::GetInstance()->GetResources()->ReadFile(ms, path))
		Application::GetInstance()->Abort(Format("Critical: couldn't load \"%s\"", path));
	
	// count of sprites
	ms.SetPosition(ms.GetLength() - 4);
	uint32_t countOfSprites = ms.ReadUInt32();
	bool hasPalette = countOfSprites & SPRITE_PALETTE_FLAG;
	countOfSprites &= ~SPRITE_PALETTE_FLAG;

	// read palette if any
	ms.SetPosition(0);
	if (hasPalette)
	{
		mPalette.resize(256);
		if (ms.ReadBytes(mPalette.data(), sizeof(Color) * 256) != sizeof(Color) * 256)
			Application::GetInstance()->Abort(Format("Critical: couldn't load \"%s\"", path));
	}

	// read frames
	for (uint32_t i = 0; i < countOfSprites; i++)
	{
		mFrames.push_back(SpriteFrame());
		SpriteFrame& frame = mFrames.back();
		frame.mWidth = ms.ReadUInt32();
		frame.mHeight = ms.ReadUInt32();
		uint32_t dataLen = ms.ReadUInt32();
		frame.mData.resize(dataLen);
		if (ms.ReadBytes(frame.mData.data(), dataLen) != dataLen)
			Application::GetInstance()->Abort(Format("Critical: couldn't load \"%s\"", path));
	}

}

void Sprite16A::Draw(DrawingContext& ctx, int32_t x, int32_t y, uint32_t index, const Color* palette)
{

	if (index >= mFrames.size())
		return;

	SpriteFrame& frame = mFrames[index];
	const Color* paletteData = palette;
	uint16_t* spriteData = (uint16_t*)frame.mData.data();

	Rect frameRec = Rect::FromXYWH(x, y, frame.mWidth, frame.mHeight);
	Rect viewRec = ctx.GetViewport();
	if (!viewRec.Intersects(frameRec))
		return;

	Color* buffer = ctx.GetBuffer() + ctx.GetPitch() * y + x;

	int32_t inX = x;
	int32_t inY = y;
	uint16_t* maxSpriteData = (uint16_t*)(frame.mData.data() + frame.mData.size());
	while (spriteData < maxSpriteData)
	{
		uint16_t rleType = *spriteData++;

		if (rleType & 0xC000)
		{
			if (rleType & 0x4000)
			{
				buffer += ctx.GetPitch() * (rleType & 0xFF);
				inY += rleType & 0xFF;
			}
			else
			{
				buffer += rleType & 0xFF;
				inX += rleType & 0xFF;
				if (inX >= static_cast<int32_t>(frame.mWidth) + x)
				{
					inX = x + (inX - (frame.mWidth+x));
					inY++;
					buffer += ctx.GetPitch() - frame.mWidth;
				}
			}
		}
		else
		{
			rleType &= 0xFF;

			if (inY < viewRec.GetTop() || inY >= viewRec.GetBottom())
			{
				spriteData += rleType;
				buffer += rleType;
				inX += rleType;
				continue;
			}

			// some pixels
			for (uint8_t i = 0; i < rleType; i++)
			{
				if (inX >= viewRec.GetLeft() && inX < viewRec.GetRight())
				{
					uint16_t px = *spriteData;
					px >>= 1;
					uint8_t alpha = ((px & 0x0F00) >> 8);
					uint8_t palIndex = px & 0xFF;
					DrawingContext::AlphaBlend16(Color(paletteData[palIndex], alpha), *buffer);
				}

				spriteData++;
				buffer++;
				inX++;
			}
		}
	}

}

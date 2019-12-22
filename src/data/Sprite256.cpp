#include "Sprite256.h"
#include "../Application.h"
#include "../utils.h"
#include "../logging.h"

Sprite256::Sprite256(const std::string& path)
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

void Sprite256::Draw(DrawingContext& ctx, int32_t x, int32_t y, uint32_t index, const Color* palette)
{

	if (index >= mFrames.size())
		return;

	SpriteFrame& frame = mFrames[index];
	const Color* paletteData = palette;
	uint8_t* spriteData = (uint8_t*)frame.mData.data();

	Rect frameRec = Rect::FromXYWH(x, y, frame.mWidth, frame.mHeight);
	Rect viewRec = ctx.GetViewport();
	if (!viewRec.Intersects(frameRec))
		return;

	bool isWhole = (viewRec.GetIntersection(frameRec) == frameRec);

	Color* buffer = ctx.GetBuffer() + ctx.GetPitch() * y + x;

	int32_t inX = x;
	int32_t inY = y;
	uint8_t* maxSpriteData = (uint8_t*)(frame.mData.data() + frame.mData.size());
	while (spriteData < maxSpriteData)
	{
		uint8_t rleType = *spriteData++;

		if (rleType & 0xC0)
		{
			if (rleType & 0x40)
			{
				buffer += ctx.GetPitch() * (rleType & 0x3F);
				inY += rleType & 0x3F;
			}
			else
			{
				buffer += rleType & 0x3F;
				inX += rleType & 0x3F;
				if (inX == frame.mWidth + x)
				{
					inX = x;
					inY++;
					buffer += ctx.GetPitch() - frame.mWidth;
				}
			}
		}
		else
		{
			rleType &= 0x3F;

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
					uint8_t palIndex = *spriteData;
					*buffer = Color(paletteData[palIndex], 255);
				}

				spriteData++;
				buffer++;
				inX++;
			}
		}
	}

}

#include "MapLogic.h"
#include "../MemoryStream.h"
#include "../Application.h"
#include "../data/AlmLevel.h"
#include "../mapview/MapView.h"

MapLogic::MapLogic(const std::string& path)
{

	mIsValid = false;

	MemoryStream ms;
	if (!Application::GetInstance()->GetResources()->ReadFile(ms, path))
	{
		Printf("Couldn't load \"%s\": couldn't open file", path);
		return;
	}

	AlmLevel alm;
	if (!alm.LoadFromStream(ms))
	{
		Printf("Couldn't load \"%s\": invalid map", path);
		return;
	}

	mWidth = alm.mInfo.mWidth;
	mHeight = alm.mInfo.mHeight;
	mNodes.resize(mWidth * mHeight);
	MapNode* nodes = GetNodes();
	uint16_t* tiles = alm.mTiles.data();
	int8_t* heights = alm.mHeights.data();
	for (int y = 0; y < mHeight; y++)
	{
		for (int x = 0; x < mWidth; x++)
		{
			nodes->mTile = *tiles++;
			nodes->mHeight = *heights++;
			nodes->mFlags = 0;
			nodes++;
		}
	}

	mIsValid = true;

	SetDefaults();

}

MapLogic::~MapLogic()
{

	// for now do-nothing, but later we will need to deinitialize all objects

}

bool MapLogic::IsValid()
{
	return mIsValid;
}

MapNode* MapLogic::GetNodes()
{
	return mNodes.data();
}

uint8_t MapLogic::GetSpeed()
{
	return mSpeed;
}

uint8_t MapLogic::SetSpeed(uint8_t speed)
{
	// to-do: emit event of speed changed
	if (speed < 0) speed = 0;
	if (speed > 8) speed = 8;
	mSpeed = speed;
	return speed;
}

void MapLogic::AttachView(MapView* view)
{
	// check if not attached already
	for (auto& v : mViews)
	{
		if (v == view)
			return;
	}
	mViews.push_back(view);
}

void MapLogic::DetachView(MapView* view)
{
	for (std::vector<MapView*>::iterator it = mViews.begin();
		it != mViews.end(); ++it)
	{
		std::vector<MapView*>::iterator toErase = it;
		it--;
		mViews.erase(toErase);
	}
}

void MapLogic::Tick()
{
	if (mLastTime == 0)
		mLastTime = Application::GetTicks();

	int64_t speedMult = 5 * (mSpeed + 1); // how many ticks in a single second
	int64_t oneTick = 1000 / speedMult;
	int64_t timePassed = (Application::GetTicks() - mLastTime) * speedMult;
	if (timePassed > oneTick)
	{
		while (timePassed > oneTick)
		{
			FixedTick();
			timePassed -= oneTick;
		}
		mLastTime = Application::GetTicks() + timePassed;
	}

}

void MapLogic::SetDefaults()
{
	mSpeed = 5;
}

void MapLogic::FixedTick()
{
	for (auto& v : mViews)
		v->FixedTick();
}

uint32_t MapLogic::GetWidth()
{
	return mWidth;
}

uint32_t MapLogic::GetHeight()
{
	return mHeight;
}

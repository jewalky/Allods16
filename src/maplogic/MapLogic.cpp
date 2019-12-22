#include "MapLogic.h"
#include "../MemoryStream.h"
#include "../Application.h"
#include "../data/AlmLevel.h"
#include "../mapview/MapView.h"
#include "MapObstacle.h"

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

	nodes = GetNodes();
	uint8_t* obstacles = alm.mObstacles.data();
	for (int y = 0; y < mHeight; y++)
	{
		for (int x = 0; x < mWidth; x++)
		{
			uint8_t obstacleID = *obstacles++;
			if (obstacleID > 0)
			{
				MapObstacle* ob = new MapObstacle(this, obstacleID - 1);
				if (!ob->IsValid())
				{
					Printf("Warning: invalid Obstacle ID %u", obstacleID);
					continue;
				}
				AddObject(ob);
				ob->SetPosition(x, y);
			}
		}
	}

	mIsValid = true;

	SetDefaults();

}

MapLogic::~MapLogic()
{

	// for now do-nothing, but later we will need to deinitialize all objects
	std::forward_list<MapObject*> objects = mAllObjects;
	mAllObjects.clear();
	for (auto& obj : objects)
		delete obj;

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
		if ((*it) == view)
		{
			mViews.erase(it);
			return;
		}
	}
}

void MapLogic::AddObject(MapObject* obj)
{
	if (obj->mIsAdded)
		return;
	mObjects.push_front(obj);
	obj->mIsAdded = true;
}

void MapLogic::RemoveObject(MapObject* obj)
{
	if (!obj->mIsAdded)
		return;
	mObjects.remove(obj);
	obj->mIsAdded = false;
}

int8_t MapLogic::GetHeightAt(float_t x, float_t y)
{
	
	int32_t xI = x;
	int32_t yI = y;
	
	if (xI < 0 || xI >= mWidth)
		return 0;
	if (yI < 0 || yI >= mHeight)
		return 0;

	float xF = x - xI;
	float yF = y - yI;

	MapNode* nodes = GetNodes() + yI * mWidth + xI;

	if (xF > 0 || yF > 0)
	{

		if (xI + 1 >= mWidth || yI + 1 >= mHeight)
			return 0;

		MapNode& node1 = *nodes;
		MapNode& node2 = *(nodes+1);
		MapNode& node3 = *(nodes+mWidth);
		MapNode& node4 = *(nodes+mWidth+1);

		float lerpY1 = node3.mHeight * yF + node1.mHeight * (1 - yF);
		float lerpY2 = node4.mHeight * yF + node2.mHeight * (1 - yF);

		float lerpX = lerpY2 * xF + lerpY1 * (1 - xF);

		return int8_t(lerpX);

	}
	else return nodes->mHeight;

}

void MapLogic::Tick()
{
	if (mLastTime == 0)
		mLastTime = Application::GetTicks();

	int64_t speedMult = 5 * (mSpeed + 1); // how many ticks in a single second
	int64_t oneTick = 1000 / speedMult;
	int64_t timePassed = (Application::GetTicks() - mLastTime) * speedMult;
	if (timePassed > 1000)
	{
		while (timePassed > 1000)
		{
			FixedTick();
			timePassed -= 1000;
			mLastTime += oneTick;
		}
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
	
	std::forward_list<MapObject*> toErase;
	
	for (auto& obj : mObjects)
	{
		if (!obj->Tick())
			toErase.push_front(obj);
	}

	for (auto& obj : toErase)
		delete obj;
}

uint32_t MapLogic::GetWidth()
{
	return mWidth;
}

uint32_t MapLogic::GetHeight()
{
	return mHeight;
}

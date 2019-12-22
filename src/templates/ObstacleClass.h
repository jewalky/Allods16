#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include "../mapview/CompoundPalette.h"
#include "../data/Sprite256.h"

class MapView;

class ObstacleFile
{
public:

	std::string mPath;
	Sprite256* mSprite = nullptr;
	Sprite256* mSpriteB = nullptr;

	void CheckLoad(MapView* view);
	const CompoundPalette* GetPalette(MapView* view);

private:

	std::unordered_map<MapView*, const CompoundPalette*> mPalettes;

};

class ObstacleClass
{
public:

	struct AnimationFrame
	{
		uint32_t mTime;
		uint32_t mFrame;
	};

	std::string mDescText;
	uint32_t mID;
	float_t mCenterX;
	float_t mCenterY;
	std::vector<AnimationFrame> mFrames;
	int32_t mDeadObject;
	ObstacleFile mFile;

};

class ObstacleClassManager
{
public:
	
	static void Load();
	static ObstacleClass* GetByID(uint32_t id);

private:

	static std::vector<ObstacleClass> mObjects;

};
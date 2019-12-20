#pragma once

#include <cstdint>
#include "../Stream.h"

#define ALM_SIGNATURE 0x0052374D 

struct AlmInfo
{

	uint32_t mWidth;
	uint32_t mHeight;
	
	float_t mSolarAngle;
	uint32_t mTimeOfDay;
	uint32_t mDarkness;
	uint32_t mContrast;

	uint32_t mUseTiles;

	uint32_t mCountPlayers;
    uint32_t mCountStructures;
    uint32_t mCountUnits;
    uint32_t mCountTriggers;
    uint32_t mCountSacks;
    uint32_t mCountGroups;
    uint32_t mCountInns;
    uint32_t mCountShops;
    uint32_t mCountPointers;
    uint32_t mCountMusic;
    std::string mName;
    uint32_t mRecPlayers;
    uint32_t mLevel;
    uint32_t mJunk1;
    uint32_t mJunk2;
    std::string mAuthor;

    bool LoadFromStream(Stream& stream);

};

struct AlmLevel
{

    AlmInfo mInfo;
    std::vector<uint16_t> mTiles;
    std::vector<int8_t> mHeights;
    std::vector<uint8_t> mObstacles;

	bool LoadFromStream(Stream& stream);

};
#include "AlmLevel.h"

#include "../logging.h"

bool AlmInfo::LoadFromStream(Stream& stream)
{

	mWidth = stream.ReadUInt32();
	mHeight = stream.ReadUInt32();
	mSolarAngle = stream.ReadFloat();
	mTimeOfDay = stream.ReadUInt32();
	mDarkness = stream.ReadUInt32();
	mContrast = stream.ReadUInt32();
	mUseTiles = stream.ReadUInt32();
	mCountPlayers = stream.ReadUInt32();
	mCountStructures = stream.ReadUInt32();
	mCountUnits = stream.ReadUInt32();
	mCountTriggers = stream.ReadUInt32();
	mCountSacks = stream.ReadUInt32();
	mCountGroups = stream.ReadUInt32();
	mCountInns = stream.ReadUInt32();
	mCountShops = stream.ReadUInt32();
	mCountPointers = stream.ReadUInt32();
	mCountMusic = stream.ReadUInt32();
	mName = stream.ReadString(0x40);
	mRecPlayers = stream.ReadUInt32();
	mLevel = stream.ReadUInt32();
	mJunk1 = stream.ReadUInt32();
	mJunk2 = stream.ReadUInt32();
	mAuthor = stream.ReadString(0x200);

	return true;
}

bool AlmLevel::LoadFromStream(Stream& stream)
{
	
	uint32_t alm_signature = stream.ReadUInt32();
	uint32_t alm_headersize = stream.ReadUInt32();
	stream.SkipBytes(4);
	uint32_t alm_sectioncount = stream.ReadUInt32();
	stream.SkipBytes(4);

	if (alm_signature != ALM_SIGNATURE)
	{
		Printf("Invalid ALM signature (0x%08X)", alm_signature);
		return false;
	}

	if (alm_headersize != 0x14)
	{
		Printf("Invalid ALM header size (0x%08X)", alm_headersize);
		return false;
	}

	if (alm_sectioncount < 3)
	{
		Printf("Invalid ALM section count (%d)", alm_sectioncount);
		return false;
	}

	bool infoLoaded = false;

	for (uint32_t i = 0; i < alm_sectioncount; i++)
	{

		stream.SkipBytes(8);
		uint32_t sec_size = stream.ReadUInt32();
		uint32_t sec_id = stream.ReadUInt32();
		stream.SkipBytes(4);

		switch (sec_id)
		{
		case 0: // info
			if (!mInfo.LoadFromStream(stream))
				return false;
			infoLoaded = true;
			break;

		case 1: // tiles
			if (!infoLoaded)
			{
				Printf("Invalid ALM: tiles before info");
				return false;
			}

			mTiles.resize(mInfo.mWidth * mInfo.mHeight);
			stream.ReadBytes(mTiles.data(), mTiles.size() * sizeof(uint16_t));
			break;

		case 2: // heights
			if (!infoLoaded)
			{
				Printf("Invalid ALM: tiles before info");
				return false;
			}

			mHeights.resize(mInfo.mWidth * mInfo.mHeight);
			stream.ReadBytes(mHeights.data(), mHeights.size() * sizeof(int8_t));
			break;

		case 3: // obstacles
			if (!infoLoaded)
			{
				Printf("Invalid ALM: tiles before info");
				return false;
			}

			mObstacles.resize(mInfo.mWidth * mInfo.mHeight);
			stream.ReadBytes(mObstacles.data(), mObstacles.size() * sizeof(uint8_t));
			break;

		default:
			stream.SkipBytes(sec_size);
			break;
		}

	}

	return true;

}
#pragma once

#include <string>
#include <vector>
#include <forward_list>
#include "MapObject.h"

struct MapNode
{

	// no not class enum thanks
	enum MapNodeFlags
	{
		BlockedGround		= 0x0001,
		BlockedAir			= 0x0002,
		Discovered			= 0x0004,
		Visible				= 0x0008,
		Unblocked			= 0x0010,
		DynamicGround		= 0x0020,
		DynamicAir			= 0x0040,
		BlockedTerrain		= 0x0080,
		NeedRedraw			= 0x0100,
		NeedRedrawFOW		= 0x0200
	};

	uint16_t mTile;
	int8_t mHeight;
	uint16_t mFlags;

	std::forward_list<MapObject*> mObjects;

};

class MapView;
class MapLogic
{
public:

	MapLogic(const std::string& path);
	~MapLogic();

	bool IsValid();
	void Tick();
	void FixedTick();

	uint32_t GetWidth();
	uint32_t GetHeight();
	MapNode* GetNodes();

	uint8_t GetSpeed();
	uint8_t SetSpeed(uint8_t speed);

	void AttachView(MapView* view);
	void DetachView(MapView* view);

	void AddObject(MapObject* obj);
	void RemoveObject(MapObject* obj);

	int8_t GetHeightAt(float_t x, float_t y);

private:

	//
	void SetDefaults();

	// internal
	bool mIsValid;
	uint64_t mLastTime = 0;
	std::vector<MapView*> mViews;
	
	// map data
	uint32_t mWidth;
	uint32_t mHeight;
	std::vector<MapNode> mNodes;
	// objects processed during Tick()
	std::forward_list<MapObject*> mObjects;
	// objects used for garbage collection
	std::forward_list<MapObject*> mAllObjects;

	friend class MapObject;

	// playsim data
	uint8_t mSpeed;

};
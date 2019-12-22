#pragma once

#include "../screen/Rect.h"
#include "../screen/Point.h"
#include <unordered_map>

class MapView;
class MapLogic;
class MapObject
{
public:

	MapObject(MapLogic* logic);
	virtual ~MapObject();

	// logic
	MapLogic* GetLogic();
	void LinkToWorld();
	void UnlinkFromWorld();
	bool IsLinked();
	bool IsAdded();
	void SetPosition(int32_t x, int32_t y);
	const Rect& GetPosition();
	uint32_t GetWidth();
	uint32_t GetHeight();

	// happens when some object is gone. we need to clear all references to this object
	virtual void CheckPointers(MapObject* object);

	// returns flags to set and clear. this is MapNode::MapNodeFlags
	virtual uint16_t GetNodeLinkFlags();
	virtual bool Tick();

	// drawing
	void CheckDraw(MapView* view);
	virtual bool HandleEvent(const SDL_Event* ev);
	virtual void Draw(MapView* view);

private:

	MapLogic* mLogic;
	bool mIsLinked = false;
	bool mIsAdded = false;
	Rect mPosition;

	uint32_t mWidth = 1;
	uint32_t mHeight = 1;

	void SetPosition(const Rect& pos);

	std::unordered_map<MapView*, uint32_t> mRenderIDs;

	friend class MapLogic;

};
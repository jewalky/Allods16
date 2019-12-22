#include "MapObject.h"
#include "MapLogic.h"
#include "../mapview/MapView.h"
#include "../logging.h"

MapObject::MapObject(MapLogic* logic)
{
	mLogic = logic;
	mLogic->mAllObjects.push_front(this);
}

MapObject::~MapObject()
{

	// perform "garbage collection"
	for (std::forward_list<MapObject*>::iterator it = mLogic->mAllObjects.begin();
		it != mLogic->mAllObjects.end(); ++it)
	{
		MapObject* obj = *it;
		if (obj == this)
			continue;
		obj->CheckPointers(this);
	}
	mLogic->mAllObjects.remove(this);

	UnlinkFromWorld();
	mLogic->RemoveObject(this);

}

MapLogic* MapObject::GetLogic()
{
	return mLogic;
}

void MapObject::LinkToWorld()
{

	if (mIsLinked)
		return;

	uint16_t flags = GetNodeLinkFlags();
	MapNode* nodes = mLogic->GetNodes() + mLogic->GetWidth() * mPosition.y + mPosition.x;
	for (int32_t y = mPosition.y; y < mPosition.GetBottom(); y++)
	{
		for (int32_t x = mPosition.x; x < mPosition.GetRight(); x++)
		{
			nodes->mFlags |= flags;
			nodes->mObjects.push_front(this);
			nodes++;
		}
		nodes += mLogic->GetWidth() - mPosition.w;
	}

	mIsLinked = true;

}

void MapObject::UnlinkFromWorld()
{

	if (!mIsLinked)
		return;

	uint16_t flags = GetNodeLinkFlags();
	MapNode* nodes = mLogic->GetNodes() + mLogic->GetWidth() * mPosition.y + mPosition.x;
	for (int32_t y = mPosition.y; y < mPosition.GetBottom(); y++)
	{
		for (int32_t x = mPosition.x; x < mPosition.GetRight(); x++)
		{
			nodes->mFlags &= ~flags;
			nodes->mObjects.remove(this);
			nodes++;
		}
		nodes += mLogic->GetWidth() - mPosition.w;
	}

	mIsLinked = false;

}

bool MapObject::IsLinked()
{
	return mIsLinked;
}

bool MapObject::IsAdded()
{
	return mIsAdded;
}

void MapObject::SetPosition(int32_t x, int32_t y)
{
	SetPosition(Rect::FromXYWH(x, y, mWidth, mHeight));
}

void MapObject::SetPosition(const Rect& pos)
{
	UnlinkFromWorld();
	Rect maxAllowed = Rect::FromXYWH(0, 0, mLogic->GetWidth(), mLogic->GetHeight());
	mPosition = pos.GetIntersection(maxAllowed);
	LinkToWorld();
}

const Rect& MapObject::GetPosition()
{
	return mPosition;
}

uint32_t MapObject::GetWidth()
{
	return mWidth;
}

uint32_t MapObject::GetHeight()
{
	return mHeight;
}

void MapObject::CheckPointers(MapObject* object)
{
}

uint16_t MapObject::GetNodeLinkFlags()
{
	return 0;
}

bool MapObject::Tick()
{
	return false;
}

void MapObject::CheckDraw(MapView* view)
{

	// draws something once (until next frame)
	uint32_t rid = view->GetRenderID();
	std::unordered_map<MapView*, uint32_t>::iterator rids = mRenderIDs.find(view);
	if (rids == mRenderIDs.end() || rids->second != rid)
	{
		Draw(view);
		mRenderIDs[view] = rid;
	}

}

bool MapObject::HandleEvent(const SDL_Event* ev)
{
	return false;
}

void MapObject::Draw(MapView* view)
{

}

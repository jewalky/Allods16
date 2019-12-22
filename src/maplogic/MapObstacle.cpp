#include "MapObstacle.h"
#include "MapLogic.h"
#include "../mapview/MapView.h"
#include "../Application.h"

MapObstacle::MapObstacle(MapLogic* logic, uint32_t id) : MapObject(logic)
{
	mClass = ObstacleClassManager::GetByID(id);
	mFrame = 0;
	mTime = 0;
}

MapObstacle::~MapObstacle()
{
}

bool MapObstacle::IsValid()
{
	return mClass != nullptr;
}

uint16_t MapObstacle::GetNodeLinkFlags()
{
	return MapNode::BlockedGround;
}

bool MapObstacle::Tick()
{

	if (mClass == nullptr)
		return false;

	if (mClass->mFrames.size() <= 1)
		return true;

	const Rect& pos = GetPosition();
	MapLogic* logic = GetLogic();
	MapNode* nodes = logic->GetNodes() + logic->GetWidth() * pos.y + pos.x;
	uint16_t anyVisible = 0;
	for (int yCheck = 0; yCheck < 3; yCheck++)
	{
		anyVisible |= nodes->mFlags & MapNode::Visible;
		nodes -= logic->GetWidth();
	}
	if (!anyVisible) return true;

	mTime++;
	if (mTime > mClass->mFrames[mFrame].mTime)
	{
		mFrame = (mFrame + 1) % (mClass->mFrames.size());
		mTime = 0;
	}

	return true;

}

void MapObstacle::Draw(MapView* view)
{
	
	if (mClass == nullptr)
		return;

	const Rect& pos = GetPosition();
	// build x/y coordinates
	int x = (pos.x - view->GetScrollX()) * 32 + 16;
	int y = (pos.y - view->GetScrollY()) * 32 + 16 - GetLogic()->GetHeightAt(pos.x + 0.5, pos.y + 0.5);

	mClass->mFile.CheckLoad(view);

	Sprite256* sprite = mClass->mFile.mSprite;
	if (sprite == nullptr)
		return;

	const CompoundPalette* pal = mClass->mFile.GetPalette(view);
	uint32_t realFrame = mClass->mFrames[mFrame].mFrame;

	int fw = sprite->GetWidth(realFrame);
	int fh = sprite->GetHeight(realFrame);

	int drawX = x - mClass->mCenterX * fw;
	int drawY = y - mClass->mCenterY * fh;
	float shadowOffs = 0.3;
	int shadowOffsReal = shadowOffs * fh;
	int shadowDrawX = x - mClass->mCenterX * fw + (-shadowOffsReal) * (1 - mClass->mCenterY);

	const Color* palette = mClass->mFile.GetPalette(view)->GetPalette(16);
	uint8_t r = palette[0].components.r;

	// draw sprite
	view->EnqueueDrawCall(y, [sprite, drawX, drawY, realFrame, palette](MapView* view){
		
		DrawingContext ctx(Application::GetInstance()->GetScreen(), view->GetClipRect());
		sprite->Draw(ctx, drawX, drawY, realFrame, palette);

	});

	// draw shadow
	view->EnqueueDrawCall(y-32, [sprite, shadowDrawX, drawY, realFrame](MapView* view) {

		DrawingContext ctx(Application::GetInstance()->GetScreen(), view->GetClipRect());
		sprite->DrawShadow(ctx, shadowDrawX, drawY, realFrame, 32, 2);

	});
	

}

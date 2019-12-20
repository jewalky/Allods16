#include "MapView.h"
#include "../Application.h"
#include <algorithm>

MapView::MapView(UIElement* parent, MapLogic* logic) : LoadingElement(parent)
{
	if (parent == nullptr)
		Application::GetInstance()->Abort("Attempted to create MapView without parent");
	if (logic == nullptr)
		Application::GetInstance()->Abort("Attempted to create MapView without MapLogic");
	SetClientRect(parent->GetClientRect());
	mLogic = logic;
	mOwnLogic = false; // ticked externally
	mLogic->AttachView(this);
	SetDefaults();
}

MapView::MapView(UIElement* parent, const std::string& path) : LoadingElement(parent)
{
	if (parent == nullptr)
		Application::GetInstance()->Abort("Attempted to create MapView without parent");
	SetClientRect(parent->GetClientRect());
	mLogic = nullptr; // will be loaded in loading thread
	mOwnLogic = true;
	mOwnMapPath = path;
}

MapView::~MapView()
{
	for (auto& image : mTiles)
		delete image;

	if (mLogic != nullptr)
	{
		mLogic->DetachView(this);
		if (mOwnLogic)
			delete mLogic;
	}

}

void MapView::LoadingThread()
{
	// load map
	if (mOwnLogic)
	{
		mLogic = new MapLogic(mOwnMapPath);
		mLogic->AttachView(this);
		SetDefaults();
	}
	// load images
	mTiles.resize(0x34);
	for (uint32_t i = 0; i < mTiles.size(); i++)
	{
		int tile1 = ((i & 0xF0) >> 4) + 1;
		int tile2 = i & 0x0F;
		mTiles[i] = new ImagePaletted(Format("graphics/terrain/tile%d-%02d.bmp", tile1, tile2));
	}
	// load palettes
	mTilePalettes.resize(4);
	for (int i = 0; i < 4; i++)
	{
		ImagePaletted* imageWithBasePalette = mTiles[i << 4];
		mTilePalettes[i].SetBasePalette(imageWithBasePalette->GetPalette());
		mTilePalettes[i].UpdatePalettes(Color(255, 255, 255, 255), 255, 255);
	}
}

void MapView::Tick()
{
	LoadingElement::Tick();

	if (IsLoading())
	{
		Application::GetInstance()->GetMouse()->SetCursor(Mouse::Wait);
		return;
	}

	mUIScrollX = mUIScrollY = 0;

	Application::GetInstance()->GetMouse()->SetCursor(Mouse::Default);

	// if own map logic, tick it
	if (mOwnLogic && mLogic != nullptr)
		mLogic->Tick();
}

bool MapView::HandleEvent(const SDL_Event* ev)
{
	if (IsLoading())
		return false;

	if (ev->type == SDL_KEYDOWN)
	{
		switch (ev->key.keysym.sym)
		{
		case SDLK_LEFT:
			mUIScrollX--;
			return true;
		case SDLK_RIGHT:
			mUIScrollX++;
			return true;
		case SDLK_UP:
			mUIScrollY--;
			return true;
		case SDLK_DOWN:
			mUIScrollY++;
			return true;
		case SDLK_c:
			{
				DrawingContext ctx(mTerrain);
				ctx.ClearRect(Rect::FromXYWH(0, 0, mTerrain->GetWidth(), mTerrain->GetHeight()), Color(0, 0, 0, 0));
				if (ev->key.keysym.mod & KMOD_SHIFT) // forces terrain redraw
					mLastDrawnRect = mLastVisibleRect = Rect::FromXYWH(0, 0, 0, 0);
				return true;
			}
		}
	}

	return false;
}

// warning: may be threaded!!! only operations inside MapView and MapLogic are safe
void MapView::SetDefaults()
{
	const Rect& clientRect = GetClientRect();
	mTerrain = new ImageTruecolor(clientRect.w, clientRect.h);
	SetScroll(8, 8);
}

void MapView::SetScroll(int32_t x, int32_t y)
{
	if (x < 8) x = 8;
	if (y < 8) y = 8;
	// calculate visible rect. we need only w&h here
	int nodesWidth = mTerrain->GetWidth() / 32;
	int nodesHeight = mTerrain->GetHeight() / 32;
	if (x + nodesWidth > mLogic->GetWidth() - 8)
		x = (mLogic->GetWidth() - 8) - nodesWidth;
	if (y + nodesHeight > mLogic->GetHeight() - 8)
		y = (mLogic->GetHeight() - 8) - nodesHeight;
	mScrollX = x;
	mScrollY = y;
	UpdateVisibleRect();
}

int32_t MapView::GetScrollX()
{
	return mScrollX;
}

int32_t MapView::GetScrollY()
{
	return mScrollY;
}

void MapView::UpdateVisibleRect()
{
	Rect allowedViewRect = Rect::FromXYWH(0, 0, mLogic->GetWidth()-1, mLogic->GetHeight()-1);
	Rect viewRect = Rect::FromXYWH(mScrollX, mScrollY, mTerrain->GetWidth() / 32, mTerrain->GetHeight() / 32).GetPadded(4).GetIntersection(allowedViewRect); // +4 nodes in every direction
	mVisibleRect = viewRect;
}

const Rect& MapView::GetVisibleRect()
{
	return mVisibleRect;
}

void MapView::DrawTerrain()
{
	// draw terrain from x/y
	// check if visible rect changed (expanded or moved)
	bool doRecordNewVisible = false;
	if (mLastVisibleRect != mVisibleRect)
	{
		// mark all new tiles as NeedRedraw
		MapNode* nodes = mLogic->GetNodes() + mVisibleRect.y * mLogic->GetWidth() + mVisibleRect.x;
		Rect unpaddedLastVisible = mLastDrawnRect;
		for (int y = mVisibleRect.y; y < mVisibleRect.GetBottom(); y++)
		{
			for (int x = mVisibleRect.x; x < mVisibleRect.GetRight(); x++)
			{
				if (!unpaddedLastVisible.Contains(Point(x, y)))
					nodes->mFlags |= MapNode::NeedRedraw;
				nodes++;
			}
			nodes += mLogic->GetWidth() - mVisibleRect.w;
		}
		mLastVisibleRect = mVisibleRect;
		doRecordNewVisible = true;

		// check if we can reuse some existing cells in the bitmap
		if (mLastScrollX != mScrollX || mLastScrollY != mScrollY)
		{
			if (mLastScrollX >= 0 && mLastScrollY >= 0)
			{
				int deltaX = (mLastScrollX - mScrollX) * 32;
				int deltaY = (mLastScrollY - mScrollY) * 32;
				mTerrain->MoveInPlace(deltaX, deltaY);
			}
			mLastScrollX = mScrollX;
			mLastScrollY = mScrollY;
		}
	}

	// save which nodes are fully seen
	if (doRecordNewVisible)
		mLastDrawnRect = Rect::FromLTRB(mVisibleRect.GetLeft()+4, mVisibleRect.GetBottom(), mVisibleRect.GetRight()-4, mVisibleRect.GetTop());

	MapNode* nodes = mLogic->GetNodes() + mVisibleRect.y * mLogic->GetWidth() + mVisibleRect.x;
	for (int32_t y = mVisibleRect.y; y < mVisibleRect.GetBottom(); y++)
	{
		bool allYDrawn = true;
		int anyDrawn = false;
		for (int32_t x = mVisibleRect.x; x < mVisibleRect.GetRight(); x++)
		{
			if (nodes->mFlags & MapNode::NeedRedraw)
			{
				allYDrawn &= DrawTerrainNode(x, y, *nodes, *(nodes+1), *(nodes+mLogic->GetWidth()), *(nodes+mLogic->GetWidth()+1));
				nodes->mFlags &= ~MapNode::NeedRedraw;
			}
			nodes++;
		}
		if (doRecordNewVisible && allYDrawn)
		{
			mLastDrawnRect.SetBottom(std::max(mLastDrawnRect.GetBottom(), y));
			mLastDrawnRect.SetTop(std::min(mLastDrawnRect.GetTop(), y));
		}
		nodes += mLogic->GetWidth() - mVisibleRect.w;
	}
}

bool MapView::DrawTerrainNode(int32_t x, int32_t y, MapNode& node1, MapNode& node2, MapNode& node3, MapNode& node4)
{

	// nodes:
	// node1 node2
	// node3 node4

	DrawingContext ctx(mTerrain);

	int x1 = (x - mScrollX) * 32;
	int x2 = (x - mScrollX) * 32 + 32;
	// for convenience
	int x3 = x1, x4 = x2;

	int y1 = (y - mScrollY) * 32;
	int y2 = (y - mScrollY) * 32;
	int y3 = (y - mScrollY) * 32 + 32;
	int y4 = (y - mScrollY) * 32 + 32;

	int y1h = y1 - node1.mHeight;
	int y2h = y2 - node2.mHeight;
	int y3h = y3 - node3.mHeight;
	int y4h = y4 - node4.mHeight;
	int minDrawY = std::min(std::min(y1h, y2h), std::min(y3h, y4h));
	int maxDrawY = std::max(std::max(y1h, y2h), std::max(y3h, y4h));

	if (maxDrawY <= 0 || minDrawY >= int32_t(mTerrain->GetHeight()))
		return false;

	bool wouldFitInY = (minDrawY >= 0 && maxDrawY < mTerrain->GetHeight());

	//  check if we don't need to draw this
	if (x2 <= 0 || x1 >= mTerrain->GetWidth())
		return wouldFitInY;

	// lerp brightness
	uint8_t brightness[32 * 32];
	uint8_t brightness1 = 16;
	uint8_t brightness2 = 32;
	uint8_t brightness3 = 16;
	uint8_t brightness4 = 32;
	uint8_t* brightnessTmp = brightness;
	for (int ly = 0; ly < 32; ly++)
	{
		uint8_t lerpY1 = brightness2 * ly / 31 + brightness1 * (31 - ly) / 31;
		uint8_t lerpY2 = brightness4 * ly / 31 + brightness3 * (31 - ly) / 31;
		for (int lx = 0; lx < 32; lx++)
		{
			uint8_t lerpX = lerpY2 * lx / 31 + lerpY1 * (31 - lx) / 31;
			*brightnessTmp++ = lerpX;
		}
	}

	// draw tile
	Color* buffer = mTerrain->GetBuffer();
	ImagePaletted* tileImage = mTiles[(node1.mTile & 0xFF0) >> 4];
	uint8_t* tileBuffer = tileImage->GetBuffer() + tileImage->GetWidth() * ((node1.mTile & 0x00F) * 32);
	const CompoundPalette& paletteBuffer = mTilePalettes[(node1.mTile & 0xF00) >> 8];
	for (int lx = 0; lx < 32; lx++)
	{
		int hMin = node2.mHeight * lx / 31 + node1.mHeight * (31 - lx) / 31;
		int hMax = node4.mHeight * lx / 31 + node3.mHeight * (31 - lx) / 31;
		int yMin = y1 - hMin;
		int yMax = y3 - hMax;
		if (yMax < yMin)
			continue;

		// precalculate
		int yCount = yMax - yMin;

		uint8_t* tilePost = tileBuffer + lx;
		Color* post = buffer + yMin * mTerrain->GetWidth() + lx + x1;
		uint8_t* brightnessPost = brightness + lx;
		for (int ly = yMin; ly < yMax; ly++)
		{
			if (ly >= 0 && ly < mTerrain->GetHeight())
			{
				int inY = (ly - yMin) * 31 / yCount;
				if (inY > 31) inY = 31;
				if (inY < 0) inY = 0;
				uint8_t palColor = *(tilePost + inY * tileImage->GetWidth());
				*post = paletteBuffer.GetPalette(*(brightnessPost+inY*32))[palColor];
			}
			post += mTerrain->GetWidth();
		}
	}

	ctx.DrawLine(Point(x1, y1 - node1.mHeight), Point(x2, y2 - node2.mHeight), Color(255, 0, 0, 255));
	ctx.DrawLine(Point(x1, y1 - node1.mHeight), Point(x3, y3 - node3.mHeight), Color(255, 0, 0, 255));

	return wouldFitInY;

}

void MapView::Draw()
{

	if (IsLoading())
		return;

	// update scrolling if UI wanted it
	if (mUIScrollX != 0 || mUIScrollY != 0)
		SetScroll(mScrollX + mUIScrollX, mScrollY + mUIScrollY);

	// update new cells in terrain
	DrawTerrain();

	// blit terrain
	const Rect& rec = GetClientRect();
	DrawingContext ctx(Application::GetInstance()->GetScreen(), rec);
	mTerrain->Blit(ctx, 0, 0);
	
}

void MapView::FixedTick()
{
	
	// check water time and animate
	mWaterAnimTime++;
	if (mWaterAnimTime > 8)
	{
		mWaterAnimFrame = (mWaterAnimFrame + 1 % 4);
		mWaterAnimTime = 0;
		MapNode* nodes = mLogic->GetNodes() + mVisibleRect.y * mLogic->GetWidth() + mVisibleRect.x;
		for (int32_t y = mVisibleRect.y; y < mVisibleRect.GetBottom(); y++)
		{
			for (int32_t x = mVisibleRect.x; x < mVisibleRect.GetRight(); x++)
			{
				// animate water
				uint16_t tile = nodes->mTile;
				uint16_t tilenum = (tile & 0xFF0) >> 4; // base rect
				uint16_t tilein = tile & 0x00F; // number of picture inside rect
				if (tilenum >= 0x20 && tilenum <= 0x2F)
				{
					tilenum -= 0x20;
					uint16_t tilewi = tilenum / 4;
					uint16_t tilew = tilenum % 4;
					uint16_t waflocal = tilewi;
					if (mWaterAnimFrame != tilewi)
						waflocal = ++waflocal % 4;
					tilenum = 0x20 + (4 * waflocal) + tilew;
					nodes->mTile = (uint16_t)((tilenum << 4) | tilein);
					nodes->mFlags |= MapNode::NeedRedraw;
				}
				nodes++;
			}
			nodes += mLogic->GetWidth() - mVisibleRect.w;
		}
	}

}
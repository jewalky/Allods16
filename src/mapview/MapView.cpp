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
	else if (ev->type == SDL_MOUSEMOTION)
	{
		mHoverX = -1;
		mHoverY = -1;
		if (ev->motion.x >= 0 && ev->motion.x < mTerrain->GetWidth() &&
			ev->motion.y >= 0 && ev->motion.y < mTerrain->GetHeight())
		{
			float fX = float(ev->motion.x % 32) / 31;
			int32_t cx = ev->motion.x / 32 + mScrollX;
			int32_t cy = -1;
			MapNode* nodes = mLogic->GetNodes() + cx + mVisibleRect.GetTop() * mLogic->GetWidth();
			for (int32_t i = mVisibleRect.GetTop(); i <= mVisibleRect.GetBottom(); i++)
			{
				MapNode* node1 = nodes;
				MapNode* node2 = nodes + 1;
				
				int32_t y1 = (i - mScrollY) * 32 - node1->mHeight;
				int32_t y2 = (i - mScrollY) * 32 - node2->mHeight;
				int32_t y = y2 * fX + y1 * (1 - fX);
				if (y > ev->motion.y)
				{
					cy = i-1;
					break;
				}

				nodes += mLogic->GetWidth();
			}

			if (cx > 0 && cy > 0 && cx < mLogic->GetWidth() && cy < mLogic->GetHeight())
			{
				mHoverX = cx;
				mHoverY = cy;

				// set visible
				MapNode* nodes = mLogic->GetNodes();
				for (int y = 0; y < mLogic->GetHeight(); y++)
				{
					for (int x = 0; x < mLogic->GetWidth(); x++)
					{

						uint16_t flags = nodes->mFlags;
						if (abs(x - cx) < 4 && abs(y - cy) < 4)
						{
							flags |= MapNode::Discovered | MapNode::Visible;
						}
						else
						{
							flags &= ~MapNode::Visible;
						}
						
						if (flags != nodes->mFlags)
						{
							if ((flags & MapNode::Discovered) != (nodes->mFlags & MapNode::Discovered))
								flags |= MapNode::NeedRedraw;
							nodes->mFlags = flags | MapNode::NeedRedrawFOW;
							int pitch = mLogic->GetWidth();
							(nodes - pitch - 1)->mFlags |= MapNode::NeedRedrawFOW;
							(nodes - pitch)->mFlags |= MapNode::NeedRedrawFOW;
							(nodes - pitch + 1)->mFlags |= MapNode::NeedRedrawFOW;
							(nodes - 1)->mFlags |= MapNode::NeedRedrawFOW;
							(nodes + 1)->mFlags |= MapNode::NeedRedrawFOW;
							(nodes + pitch - 1)->mFlags |= MapNode::NeedRedrawFOW;
							(nodes + pitch)->mFlags |= MapNode::NeedRedrawFOW;
							(nodes + pitch + 1)->mFlags |= MapNode::NeedRedrawFOW;
						}

						nodes++;

					}
				}
			}
			else
			{
				mHoverX = mHoverY = -1;
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
	mTerrainFOW = new ImagePaletted(mTerrain->GetWidth(), mTerrain->GetHeight());
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

const CompoundPalette* MapView::AllocateCompoundPalette(const Color* basePalette)
{
	mObjectPalettes.resize(mObjectPalettes.size() + 1);
	CompoundPalette* pal = &mObjectPalettes[mObjectPalettes.size() - 1];
	pal->SetBasePalette(basePalette);
	pal->UpdatePalettes(Color(255, 255, 255, 255), 255, 255);
	return pal;
}

void MapView::EnqueueDrawCall(int order, MapViewDrawCall callback)
{

	DrawCallEntry ent;
	ent.mWeight = order;
	ent.mCallback = callback;

	if (mDrawQueue.empty() || (!mDrawQueue.empty() > 0 && mDrawQueue.front().mWeight > order))
	{
		mDrawQueue.push_front(ent);
		return;
	}

	std::forward_list<DrawCallEntry>::iterator last = mDrawQueue.begin();
	for (std::forward_list<DrawCallEntry>::iterator it = ++mDrawQueue.begin();
		it != mDrawQueue.end(); ++it)
	{
		if (it->mWeight > order)
		{
			mDrawQueue.insert_after(last, ent);
			return;
		}

		last++;
	}

	mDrawQueue.insert_after(last, ent);

}

uint32_t MapView::GetRenderID()
{
	return mRenderID;
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
					nodes->mFlags |= MapNode::NeedRedraw|MapNode::NeedRedrawFOW;
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
			if (nodes->mFlags & (MapNode::NeedRedraw | MapNode::NeedRedrawFOW))
			{
				allYDrawn &= DrawTerrainNode(x, y, *nodes);
				nodes->mFlags &= ~(MapNode::NeedRedraw|MapNode::NeedRedrawFOW);
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

// produce alpha value from fog of war flag
static uint8_t alphaFromVisFlags(uint16_t flags)
{
	if (flags & MapNode::Visible)
		return 255;
	if (flags & MapNode::Discovered)
		return 127;
	return 0;
}

bool MapView::DrawTerrainNode(int32_t x, int32_t y, MapNode& node1)
{

	// nodes:
	// node1 node2
	// node3 node4

	DrawingContext ctx(mTerrain);

	int nodesPitch = mLogic->GetWidth();
	MapNode* nodes = mLogic->GetNodes() + nodesPitch * y + x;
	MapNode& node2 = *(nodes + 1);
	MapNode& node3 = *(nodes + nodesPitch);
	MapNode& node4 = *(nodes + nodesPitch + 1);

	int x1 = (x - mScrollX) * 32;
	int x2 = (x - mScrollX) * 32 + 32;
	// for convenience
	int x3 = x1, x4 = x2;

	if (x2 <= 0 || x1 >= mTerrain->GetWidth())
		return false;

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

	// lerp brightness
	uint8_t brightness1 = 16;
	uint8_t brightness2 = 16;
	uint8_t brightness3 = 16;
	uint8_t brightness4 = 16;

	// fog of war
	static uint8_t fow[32 * 32];
	// if we are going to draw with a single color, just set it here
	int singleFOWColor = -1;
	if (node1.mFlags & MapNode::NeedRedrawFOW)
	{
		// get more nodes
		// we have center and one to the right and bottom
		// we also need up and left
		// rename nodes a bit.. so that there is no confusion
		MapNode& aux1 = *(nodes - nodesPitch - 1);
		MapNode& aux2 = *(nodes - nodesPitch);
		MapNode& aux3 = *(nodes - nodesPitch + 1);
		MapNode& aux4 = *(nodes - 1);
		MapNode& aux5 = node1;
		MapNode& aux6 = node2;
		MapNode& aux7 = *(nodes + nodesPitch - 1);
		MapNode& aux8 = node3;
		MapNode& aux9 = node4;

		uint16_t f1 = aux1.mFlags & (MapNode::Discovered | MapNode::Visible);
		uint16_t f2 = aux2.mFlags & (MapNode::Discovered | MapNode::Visible);
		uint16_t f3 = aux3.mFlags & (MapNode::Discovered | MapNode::Visible);
		uint16_t f4 = aux4.mFlags & (MapNode::Discovered | MapNode::Visible);
		uint16_t f5 = aux5.mFlags & (MapNode::Discovered | MapNode::Visible);
		uint16_t f6 = aux6.mFlags & (MapNode::Discovered | MapNode::Visible);
		uint16_t f7 = aux7.mFlags & (MapNode::Discovered | MapNode::Visible);
		uint16_t f8 = aux8.mFlags & (MapNode::Discovered | MapNode::Visible);
		uint16_t f9 = aux9.mFlags & (MapNode::Discovered | MapNode::Visible);

		bool differentFlags = f5 != f1 || f5 != f2 || f5 != f3 || f5 != f4 || f5 != f6 || f5 != f7 || f5 != f8 || f5 != f9;

		if (!differentFlags)
		{
			singleFOWColor = alphaFromVisFlags(f5);
		}
		else
		{

			int np1 = alphaFromVisFlags(f1);
			int np2 = alphaFromVisFlags(f2);
			int np3 = alphaFromVisFlags(f3);
			int np4 = alphaFromVisFlags(f4);
			int np5 = alphaFromVisFlags(f5);
			int np6 = alphaFromVisFlags(f6);
			int np7 = alphaFromVisFlags(f7);
			int np8 = alphaFromVisFlags(f8);
			int np9 = alphaFromVisFlags(f9);

			float topContrib = 0.5;
			float yContrib = 0.5;
			float bottomContrib = 0;
			float fDelta = 1.0 / 32;
			uint8_t* cbuf = fow;
			for (int y = 0; y < 32; y++)
			{
				float leftContrib = 0.5;
				float xContrib = 0.5;
				float rightContrib = 0;

				for (int x = 0; x < 32; x++)
				{
					int c1 = np1 * leftContrib + np2 * xContrib + np3 * rightContrib;
					int c2 = np4 * leftContrib + np5 * xContrib + np6 * rightContrib;
					int c3 = np7 * leftContrib + np8 * xContrib + np9 * rightContrib;
					int c4 = c1 * topContrib + c2 * yContrib + c3 * bottomContrib;

					*cbuf++ = c4;

					if (x > 15)
					{
						leftContrib = 0;
						rightContrib += fDelta;
						xContrib -= fDelta;
					}
					else
					{
						leftContrib -= fDelta;
						xContrib += fDelta;
					}
				}

				if (y > 15)
				{
					topContrib = 0;
					bottomContrib += fDelta;
					yContrib -= fDelta;
				}
				else
				{
					topContrib -= fDelta;
					yContrib += fDelta;
				}
			}

		}
	}

	// draw tile
	Color* buffer = ctx.GetBuffer();
	ImagePaletted* tileImage = mTiles[(node1.mTile & 0xFF0) >> 4];
	uint8_t* tileBuffer = tileImage->GetBuffer() + tileImage->GetWidth() * ((node1.mTile & 0x00F) * 32);
	uint8_t* fowBuffer = mTerrainFOW->GetBuffer();
	const CompoundPalette& paletteBuffer = mTilePalettes[(node1.mTile & 0xF00) >> 8];
	int terrainPitch = mTerrain->GetWidth();
	for (int lx = 0; lx < 32; lx++)
	{
		float fX = float(lx) / 31;
		int hMin = node2.mHeight * fX + node1.mHeight * (1 - fX);
		int hMax = node4.mHeight * fX + node3.mHeight * (1 - fX);
		int yMin = y1 - hMin;
		int yMax = y3 - hMax;
		if (yMax < yMin)
			continue;

		// precalculate
		int yCount = yMax - yMin;
		float fScale = float(32) / yCount;
		
		float brightnessX1, brightnessX2;
		if (node1.mFlags & MapNode::NeedRedraw)
		{
			brightnessX1 = brightness1 * fX + brightness2 * (1 - fX);
			brightnessX2 = brightness3 * fX + brightness4 * (1 - fX);
		}

		uint8_t* tilePost = tileBuffer + lx;
		Color* post = buffer + yMin * terrainPitch + lx + x1;
		uint8_t* fowBufferPost = fow + lx;
		uint8_t* fowPost = fowBuffer + yMin * terrainPitch + lx + x1;
		for (int ly = yMin; ly < yMax; ly++)
		{
			if (ly >= 0 && ly < mTerrain->GetHeight())
			{
				int inY = (ly - yMin) * fScale;
				if (inY > 31) inY = 31;
				if (inY < 0) inY = 0;
				float fY = float(inY) / 31;
				int brightnessY;
				if (node1.mFlags & MapNode::NeedRedraw)
				{
					brightnessY = float(brightnessX1 * fY + brightnessX2 * (1 - fY));
					uint8_t palColor = *(tilePost + inY * tileImage->GetWidth());
					*post = paletteBuffer.GetPalette(brightnessY)[palColor];
				}
				if (node1.mFlags & MapNode::NeedRedrawFOW)
				{
					uint8_t fowByte = (singleFOWColor >= 0) ? singleFOWColor : *(fowBufferPost + inY * 32);
					*fowPost = fowByte;
				}
			}
			post += terrainPitch;
			fowPost += terrainPitch;
		}
	}

	if (node1.mFlags & MapNode::NeedRedraw)
	{
		ctx.DrawLine(Point(x1, y1 - node1.mHeight), Point(x2, y2 - node2.mHeight), Color(128, 0, 0, 255));
		ctx.DrawLine(Point(x1, y1 - node1.mHeight), Point(x3, y3 - node3.mHeight), Color(128, 0, 0, 255));
	}

	return wouldFitInY;

}

void MapView::DrawVisibility()
{

	// this logic is pretty much from DrawPartial of the image, except it draws shadows instead of alpha
	// first, take rect in screen space and clip it to viewport
	const Rect& rec = GetClipRect();
	DrawingContext ctx(Application::GetInstance()->GetScreen(), rec);

	int terrainW = mTerrainFOW->GetWidth();
	int terrainH = mTerrainFOW->GetHeight();

	Rect innerRect = Rect::FromXYWH(0, 0, terrainW, terrainH);

	Rect viewRec = ctx.GetViewport();
	Rect screenRec = Rect::FromXYWH(rec.x, rec.y, innerRect.w, innerRect.h).GetIntersection(viewRec);
	if (screenRec.w > terrainW) screenRec.w = terrainW;
	if (screenRec.h > terrainH) screenRec.h = terrainH;
	Rect clipRec = Rect::FromXYWH(screenRec.x - rec.x + innerRect.x, screenRec.y - rec.y + innerRect.y, screenRec.w, screenRec.h).GetIntersection(Rect::FromXYWH(0, 0, terrainW, terrainH));

	Color* screenBuffer = ctx.GetBuffer() + screenRec.y * ctx.GetPitch() + screenRec.x;
	uint8_t* buffer = mTerrainFOW->GetBuffer() + clipRec.y * terrainW + clipRec.x;

	for (int y = clipRec.GetTop(); y < clipRec.GetBottom(); y++)
	{
		for (int x = clipRec.GetLeft(); x < clipRec.GetRight(); x++)
		{
			screenBuffer->components.r = screenBuffer->components.r * (*buffer) / 255;
			screenBuffer->components.g = screenBuffer->components.g * (*buffer) / 255;
			screenBuffer->components.b = screenBuffer->components.b * (*buffer) / 255;
			screenBuffer++;
			buffer++;
		}
		screenBuffer += ctx.GetPitch() - clipRec.w;
		buffer += terrainW - clipRec.w;
	}

}

void MapView::Draw()
{

	if (IsLoading())
		return;

	// begin frame
	mRenderID++;
	mDrawQueue.clear();

	// update scrolling if UI wanted it
	if (mUIScrollX != 0 || mUIScrollY != 0)
		SetScroll(mScrollX + mUIScrollX, mScrollY + mUIScrollY);

	// update new cells in terrain
	DrawTerrain();

	// blit terrain
	const Rect& rec = GetClipRect();
	DrawingContext ctx(Application::GetInstance()->GetScreen(), rec);
	mTerrain->Blit(ctx, rec.x, rec.y);

	// enqueue objects
	MapNode* nodes = mLogic->GetNodes() + mVisibleRect.y * mLogic->GetWidth() + mVisibleRect.x;
	for (int32_t y = mVisibleRect.y; y < mVisibleRect.GetBottom(); y++)
	{
		for (int32_t x = mVisibleRect.x; x < mVisibleRect.GetRight(); x++)
		{
			for (auto& object : nodes->mObjects)
				object->CheckDraw(this);
			nodes++;
		}
		nodes += mLogic->GetWidth() - mVisibleRect.w;
	}

	// draw objects
	for (auto& ent : mDrawQueue)
		ent.mCallback(this);

	// draw fog of war
	DrawVisibility();
	
}

void MapView::FixedTick()
{
	
	// check water time and animate
	mWaterAnimTime++;
	if (mWaterAnimTime > 5)
	{
		mWaterAnimTime = 0;
		MapNode* nodes = mLogic->GetNodes() + mVisibleRect.y * mLogic->GetWidth() + mVisibleRect.x;
		for (int32_t y = mVisibleRect.y; y < mVisibleRect.GetBottom(); y++)
		{
			for (int32_t x = mVisibleRect.x; x < mVisibleRect.GetRight(); x++)
			{
				if (!(nodes->mFlags & MapNode::Visible))
				{
					nodes++;
					continue;
				}

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

	// update non-active objects (trees...)
	std::forward_list<MapObject*> toErase;
	MapNode* nodes = mLogic->GetNodes() + mVisibleRect.y * mLogic->GetWidth() + mVisibleRect.x;
	for (int32_t y = mVisibleRect.GetBottom()-1; y >= mVisibleRect.y; y--)
	{
		for (int32_t x = mVisibleRect.GetRight()-1; x >= mVisibleRect.x; x--)
		{
			for (auto& object : nodes->mObjects)
			{
				if (!object->IsAdded())
				{
					if (!object->Tick())
						toErase.push_front(object);
				}
			}
			nodes++;
		}
		nodes += mLogic->GetWidth() - mVisibleRect.w;
	}

	for (auto& obj : toErase)
		delete obj;

}
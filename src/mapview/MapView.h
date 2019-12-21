#pragma once

#include "../ui/LoadingElement.h"
#include "../maplogic/MapLogic.h"
#include "../data/ImagePaletted.h"
#include "../data/ImageTruecolor.h"
#include "CompoundPalette.h"
#include "../screen/Rect.h"

class MapView : public LoadingElement
{
public:
	explicit MapView(UIElement* parent, MapLogic* logic);
	explicit MapView(UIElement* parent, const std::string& path);
	virtual ~MapView();

	virtual void Tick();
	virtual void Draw();
	virtual bool HandleEvent(const SDL_Event* ev);

	// on map load
	void SetDefaults();

	// handles one playsim tick in MapLogic
	void FixedTick();

	//
	void SetScroll(int32_t x, int32_t y);
	int32_t GetScrollX();
	int32_t GetScrollY();
	const Rect& GetVisibleRect();

private:

	virtual void LoadingThread();
	void UpdateVisibleRect();
	
	// terrain drawing
	void DrawTerrain();
	bool DrawTerrainNode(int32_t x, int32_t y, MapNode& node1);

	// visibility drawing
	void DrawVisibility();

	bool mOwnLogic = false;
	std::string mOwnMapPath;
	MapLogic* mLogic;

	// tile images
	std::vector<ImagePaletted*> mTiles;
	std::vector<CompoundPalette> mTilePalettes;

	// terrain image
	ImageTruecolor* mTerrain;
	ImagePaletted* mTerrainFOW;

	//
	int32_t mLastScrollX = -1;
	int32_t mLastScrollY = -1;
	int32_t mScrollX = 8;
	int32_t mScrollY = 8;
	Rect mLastDrawnRect;
	Rect mLastVisibleRect;
	Rect mVisibleRect;

	// ui variables
	uint64_t mLastScrollTime;
	int32_t mUIScrollX;
	int32_t mUIScrollY;
	int32_t mHoverX;
	int32_t mHoverY;

	// global terrain animation
	uint64_t mWaterAnimTime;
};
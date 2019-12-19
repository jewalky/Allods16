#pragma once

#include "../data/Sprite.h"
#include "../screen/Point.h"
#include "../screen/Rect.h"

#include "../logging.h"

class Mouse
{
public:

	Mouse();

	// draw mouse
	void PreApply();
	// undraw mouse
	void PostApply();
	// set pos
	void SetPosition(const Point& p);

	enum InternalCursor
	{
		Default,
		Select,
		SelectStructure,
		Move,
		Attack,
		MoveAttack,
		Wait,
		CantPut,
		Pickup,
		Cast,
		SmallDefault,

		NUM_CURSORS,
		NoCursor = -1
	};

	// for items
	void SetCursor(Sprite* sprite);
	// for regular cursor
	void SetCursor(InternalCursor cursor);


private:
	//
	struct MouseCursorInfo
	{
		Sprite* mSprite;
		int32_t mOffsetX;
		int32_t mOffsetY;
		int32_t mAnimDelay;
		uint32_t mAnimFrame;
		bool mTransient;

		MouseCursorInfo()
		{
			mSprite = nullptr;
			mOffsetX = mOffsetY = 0;
			mAnimDelay = -1;
			mAnimFrame = 0;
			mTransient = false;
		}

		MouseCursorInfo(Sprite* sprite, int32_t offsX, int32_t offsY, int32_t animDelay, bool transient)
		{
			Set(sprite, offsX, offsY, animDelay, transient);
		}

		void Set(Sprite* sprite, int32_t offsX, int32_t offsY, int32_t animDelay, bool transient)
		{
			mSprite = sprite;
			mOffsetX = offsX;
			mOffsetY = offsY;
			mAnimDelay = animDelay;
			mAnimFrame = 0;
			mTransient = transient;
		}

		~MouseCursorInfo()
		{
			if (!mTransient && mSprite != nullptr)
				delete mSprite;
			mSprite = nullptr;
		}
	};

	std::vector<MouseCursorInfo> mInternalCursors;
	MouseCursorInfo* mCurrentCursor = nullptr;

	Point mPosition;

	// stores saved screen rect
	// manual blitting for now
	std::vector<Color> mSavedBuffer;
	Rect mSavedRect;
};
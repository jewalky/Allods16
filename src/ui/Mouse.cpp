#include "Mouse.h"
#include "../data/Sprite16A.h"
#include "../Application.h"
#include "../logging.h"

Mouse::Mouse()
{
	mInternalCursors.resize(NUM_CURSORS);
	mInternalCursors[Default].Set(new Sprite16A("graphics/cursors/default/sprites.16a"), 4, 4, -1, false);
	mInternalCursors[Wait].Set(new Sprite16A("graphics/cursors/wait/sprites.16a"), 16, 16, 40, false);
	mCurrentCursor = nullptr;
}

void Mouse::PreApply()
{

	if (mCurrentCursor == nullptr || mCurrentCursor->mSprite == nullptr)
		return;

	Screen* screen = Application::GetInstance()->GetScreen();
	Rect viewport = screen->GetViewport();

	Rect curMouseRect = Rect::FromXYWH(
		mPosition.x - mCurrentCursor->mOffsetX,
		mPosition.y - mCurrentCursor->mOffsetY,
		mCurrentCursor->mSprite->GetWidth(mCurrentCursor->mAnimFrame),
		mCurrentCursor->mSprite->GetHeight(mCurrentCursor->mAnimFrame)
	).GetIntersection(viewport);

	mSavedBuffer.resize(curMouseRect.w * curMouseRect.h);
	Color* screenBuffer = screen->GetBuffer() + curMouseRect.y * viewport.w + curMouseRect.x;
	Color* buffer = mSavedBuffer.data();
	for (int y = curMouseRect.GetTop(); y < curMouseRect.GetBottom(); y++)
	{
		for (int x = curMouseRect.GetLeft(); x < curMouseRect.GetRight(); x++)
			*buffer++ = *screenBuffer++;
		screenBuffer += viewport.w - curMouseRect.w;
	}

	mSavedRect = curMouseRect;

	// render mouse
	DrawingContext ctx(screen);
	mCurrentCursor->mSprite->Draw(ctx,
		mPosition.x - mCurrentCursor->mOffsetX,
		mPosition.y - mCurrentCursor->mOffsetY,
		mCurrentCursor->mAnimFrame,
		mCurrentCursor->mSprite->GetPalette());

}

void Mouse::PostApply()
{

	if (mCurrentCursor == nullptr || mCurrentCursor->mSprite == nullptr)
		return;

	Screen* screen = Application::GetInstance()->GetScreen();
	Rect viewport = screen->GetViewport();

	Color* screenBuffer = screen->GetBuffer() + mSavedRect.y * viewport.w + mSavedRect.x;
	Color* buffer = mSavedBuffer.data();
	for (int y = mSavedRect.GetTop(); y < mSavedRect.GetBottom(); y++)
	{
		for (int x = mSavedRect.GetLeft(); x < mSavedRect.GetRight(); x++)
			*screenBuffer++ = *buffer++;
		screenBuffer += viewport.w - mSavedRect.w;
	}

}

void Mouse::SetPosition(const Point& p)
{
	mPosition = p;
}

void Mouse::SetCursor(Sprite* sprite)
{

	if (mCurrentCursor != nullptr)
	{
		if (mCurrentCursor->mTransient)
			delete mCurrentCursor;
		mCurrentCursor = nullptr;
	}

	if (sprite != nullptr)
		mCurrentCursor = new MouseCursorInfo(sprite, sprite->GetWidth(0) / 2, sprite->GetHeight(0) / 2, -1, true);

}

void Mouse::SetCursor(InternalCursor cursor)
{

	if (mCurrentCursor != nullptr)
	{
		if (mCurrentCursor->mTransient)
			delete mCurrentCursor;
		mCurrentCursor = nullptr;
	}

	if (cursor < 0 || cursor >= NUM_CURSORS)
		return;

	mCurrentCursor = &mInternalCursors[cursor];

}

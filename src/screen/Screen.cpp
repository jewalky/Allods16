#include "Screen.h"
#include "../logging.h"
#include "../Application.h"

Screen::Screen(int w, int h)
{

	mWindow = nullptr;
	mSurface = nullptr;

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		Printf("Error initializing SDL: %s", SDL_GetError());
		return;
	}

	if (SDL_GetDesktopDisplayMode(0, &mDisplayMode) != 0)
	{
		Printf("Error querying display mode: %s", SDL_GetError());
		return;
	}

	mWindow = SDL_CreateWindow("Allods", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, 0);
	if (!mWindow)
	{
		Printf("Error creating window: %s", SDL_GetError());
		return;
	}

	SDL_ShowCursor(0);

	mSurface = SDL_GetWindowSurface(mWindow);
	if (!mSurface)
	{
		Printf("Error getting window surface: %s", SDL_GetError());
		SDL_DestroyWindow(mWindow);
		mWindow = nullptr;
		return;
	}

	mViewport = Rect::FromXYWH(0, 0, w, h);

	mFPS = 0;
	mFPSTimer = 0;

}

Screen::~Screen()
{
	if (mWindow != nullptr)
		SDL_DestroyWindow(mWindow);
	// surface is controlled by window
	mSurface = nullptr;
	mWindow = nullptr;
}

bool Screen::IsValid()
{
	return mWindow != nullptr;
}

Color* Screen::GetBuffer()
{
	if (mWindow == nullptr || mSurface == nullptr)
		return nullptr;
	// can we be sure that this is always consistent?
	return (Color*)mSurface->pixels;
}

Rect Screen::GetViewport()
{
	return mViewport;
}

void Screen::Apply()
{
	if (mWindow == nullptr)
		return;
	if (!mFPSTimer)
		mFPSTimer = Application::GetTicks();
	mFPSLast++;
	if (Application::GetTicks() - mFPSTimer > 1000)
	{
		mFPS = mFPSLast;
		mFPSLast = 0;
		mFPSTimer = Application::GetTicks();
		SDL_SetWindowTitle(mWindow, Format("Allods (%d FPS)", mFPS).c_str());
	}
	SDL_UpdateWindowSurface(mWindow);
}

bool Screen::PollEvent(SDL_Event& ev)
{
	return SDL_PollEvent(&ev);
}

uint64_t Screen::GetFPS()
{
	return mFPS;
}
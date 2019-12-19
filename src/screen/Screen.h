#pragma once

#include <cstdint>
#include <SDL.h>
#include "Rect.h"
#include "Color.h"

class Screen
{
public:

	Screen(int w, int h);
	virtual ~Screen();

	operator bool() { return IsValid(); }
	bool IsValid();

	Color* GetBuffer();
	Rect GetViewport();
	void Apply();

	bool PollEvent(SDL_Event& ev);

private:
	SDL_Window* mWindow;
	SDL_Surface* mSurface;
	Rect mViewport;
	SDL_DisplayMode mDisplayMode;
};
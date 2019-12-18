#pragma once

#include <SDL.h>
#include "Point.h"

// This is a backwards compatible wrapper around SDL_Rect which supports multiple formats
struct Rect : public SDL_Rect
{

	// left,right,top,bottom
	int GetLeft() const;
	int GetRight() const;
	int GetTop() const;
	int GetBottom() const;

	// w,h,x,y
	int GetWidth() const;
	int GetHeight() const;
	int GetX() const;
	int GetY() const;

	// setters
	int SetLeft(int left);
	int SetRight(int right);
	int SetTop(int top);
	int SetBottom(int bottom);

	//
	int SetX(int x);
	int SetY(int y);
	int SetWidth(int w);
	int SetHeight(int h);

	//
	operator SDL_Rect();

	//
	Rect();
	Rect(const SDL_Rect& sdlrec);
	
	static Rect FromXYWH(int x, int y, int w, int h);
	static Rect FromLTRB(int left, int top, int right, int bottom);

	//
	bool Contains(const Point& p) const;
	bool Intersects(const Rect& r) const;
	Rect GetIntersection(const Rect& r) const;
	Rect GetTranslated(const Point& p) const;

};
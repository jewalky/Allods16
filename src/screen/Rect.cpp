#include "Rect.h"
#include <algorithm>

//
int Rect::GetLeft() const { return x; }
int Rect::GetX() const { return x; }

int Rect::GetTop() const { return y; }
int Rect::GetY() const { return y; }

int Rect::GetRight() const { return x + w; }
int Rect::GetWidth() const { return w; }

int Rect::GetBottom() const { return y + h; }
int Rect::GetHeight() const { return h; }

//
int Rect::SetLeft(int left) { return x = left; }
int Rect::SetX(int x) { return this->x = x; }

int Rect::SetTop(int top) { return y = top; }
int Rect::SetY(int y) { return this->y = y; }

int Rect::SetRight(int right) { w = right - x; return right; }
int Rect::SetWidth(int w) { return this->w = w; }

int Rect::SetBottom(int bottom) { h = bottom - y; return bottom; }
int Rect::SetHeight(int h) { return this->h = h; }


//
Rect::operator SDL_Rect()
{
	SDL_Rect rec;
	rec.x = x;
	rec.y = y;
	rec.w = w;
	rec.h = h;
	return rec;
}

//
Rect::Rect()
{
	x = y = w = h = 0;
}

Rect::Rect(const SDL_Rect& sdlrec)
{
	x = sdlrec.x;
	y = sdlrec.y;
	w = sdlrec.w;
	h = sdlrec.h;
}

Rect Rect::FromXYWH(int x, int y, int w, int h)
{
	Rect r;
	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;
	return r;
}

Rect Rect::FromLTRB(int left, int top, int right, int bottom)
{
	Rect r;
	r.x = left;
	r.y = top;
	r.w = right - left;
	r.h = bottom - top;
	return r;
}

//
bool Rect::Contains(const Point& p) const
{
	return (p.x >= x && p.y >= y && p.x < GetRight() && p.y < GetBottom());
}

bool Rect::Intersects(const Rect& r) const
{
	return
		((GetLeft() >= r.GetLeft() && GetLeft() <= r.GetRight()) ||
		(GetRight() >= r.GetLeft() && GetRight() <= r.GetRight()))
		&&
		((GetTop() >= r.GetTop() && GetTop() <= r.GetBottom()) ||
		(GetBottom() >= r.GetTop() && GetBottom() <= r.GetBottom()));
}

Rect Rect::GetIntersection(const Rect& r) const
{
	return Rect::FromLTRB(
		std::max(GetLeft(), r.GetLeft()),
		std::max(GetTop(), r.GetTop()),
		std::min(GetRight(), r.GetRight()),
		std::min(GetBottom(), r.GetBottom())
	);
}
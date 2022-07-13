#include "SmackerView.h"

SmackerView::SmackerView(UIElement* parent, const std::string& path)
{
	mVideoPath = path;
	Rect p = parent->GetClientRect();
	SetClientRect(Rect::FromXYWH(p.w / 2 - 320, p.h / 2 - 240, 640, 480));
}

SmackerView::~SmackerView()
{

}

void SmackerView::Tick()
{
}

void SmackerView::Draw()
{
}

bool SmackerView::HandleEvent(const SDL_Event* ev)
{
	return false;
}

void SmackerView::LoadingThread()
{

}

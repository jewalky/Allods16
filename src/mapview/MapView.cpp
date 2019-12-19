#include "MapView.h"
#include "../Application.h"

MapView::MapView(UIElement* parent) : LoadingElement(parent)
{
	if (parent == nullptr)
		Application::GetInstance()->Abort("Attempted to create MapView without parent");
	SetClientRect(parent->GetClientRect());
}

void MapView::Tick()
{
	LoadingElement::Tick();
	if (IsLoading())
		Application::GetInstance()->GetMouse()->SetCursor(Mouse::Wait);
	else Application::GetInstance()->GetMouse()->SetCursor(Mouse::Default);
}

bool MapView::HandleEvent(const SDL_Event* ev)
{
	return false;
}

void MapView::LoadingThread()
{
	// to-do put MapLogic loading here
}

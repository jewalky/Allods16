#pragma once

#include "../ui/LoadingElement.h"

class MapView : public LoadingElement
{
public:
	explicit MapView(UIElement* parent = nullptr);

	virtual void Tick();
	virtual bool HandleEvent(const SDL_Event* ev);

private:
	virtual void LoadingThread();
};
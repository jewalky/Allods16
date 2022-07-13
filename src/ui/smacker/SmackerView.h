#pragma once

#include "../LoadingElement.h"

class SmackerView : public LoadingElement
{
public:
	explicit SmackerView(UIElement* parent, const std::string& path);
	virtual ~SmackerView();

	virtual void Tick();
	virtual void Draw();
	virtual bool HandleEvent(const SDL_Event* ev);

private:

	virtual void LoadingThread();

	std::string mVideoPath;

};
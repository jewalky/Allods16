#pragma once

#include "../ui/LoadingElement.h"

class TemplateLoader : public LoadingElement
{
public:

	explicit TemplateLoader(UIElement* parent);

	virtual void Tick();

private:

	virtual void LoadingThread();

};
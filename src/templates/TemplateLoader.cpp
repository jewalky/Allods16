#include "TemplateLoader.h"
#include "../Application.h"

#include "ObstacleClass.h"

TemplateLoader::TemplateLoader(UIElement* parent) : LoadingElement(parent)
{
	if (parent == nullptr)
		Application::GetInstance()->Abort("Attempted to create TemplateLoader without parent");
}

void TemplateLoader::Tick()
{

	LoadingElement::Tick();

	if (IsLoading())
		Application::GetInstance()->GetMouse()->SetCursor(Mouse::Wait);
	else Application::GetInstance()->GetMouse()->SetCursor(Mouse::Default);
	
}

void TemplateLoader::LoadingThread()
{
	ObstacleClassManager::Load();
}

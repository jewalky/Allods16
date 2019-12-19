#include "RootUIElement.h"
#include "../Application.h"

RootUIElement::RootUIElement() : UIElement(nullptr)
{
	SetClientRect(Application::GetInstance()->GetScreen()->GetViewport());
}
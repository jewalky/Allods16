#include "UIElement.h"

#include "../logging.h"

UIElement::UIElement(UIElement* parent)
{
	mParent = nullptr;
	SetParent(parent);
}

UIElement::~UIElement()
{
	SetParent(nullptr);
	for (auto& child : mChildren)
		delete child;
}

void UIElement::PropagateClip(bool haveParentResize)
{
	UpdateClipRect();
	if (haveParentResize)
		HandleParentResize();
	for (auto& child : mChildren)
		child->PropagateClip(true);
}

void UIElement::PropagateTick()
{
	for (std::vector<UIElement*>::iterator it = mChildren.begin();
		it != mChildren.end(); ++it)
	{
		UIElement* child = *it;
		if (child->mIsClosing)
		{
			it--;
			delete child;
		}
	}

	Tick();
	for (auto& child : mChildren)
		child->PropagateTick();
}

void UIElement::PropagateDraw()
{
	Draw();
	for (auto& child : mChildren)
		child->PropagateDraw();
}

bool UIElement::PropagateEvent(const SDL_Event* ev)
{

	switch (ev->type)
	{
	case SDL_KEYDOWN:
	case SDL_KEYUP:
	case SDL_TEXTINPUT:
		{
			if (!IsFocusable() || IsFocused())
			{
				if (mChildren.size())
				{
					bool anyHandled = false;
					for (auto& child : mChildren)
						anyHandled |= child->PropagateEvent(ev);
					return anyHandled;
				}
				else
				{
					return BubbleEvent(ev);
				}
			}
			break;
		}
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEMOTION:
		{
			if (mClipRect.Contains(Point(ev->motion.x, ev->motion.y)))
			{
				if (mChildren.size())
				{
					bool anyHandled = false;
					for (auto& child : mChildren)
						anyHandled |= child->PropagateEvent(ev);
					return anyHandled;
				}
				else
				{
					return BubbleEvent(ev);
				}
			}
			break;
		}
	default:
		break;
	}

	return false;

}

bool UIElement::BubbleEvent(const SDL_Event* ev)
{

	if (HandleEvent(ev))
		return true;
	
	if (mParent != nullptr)
		return mParent->BubbleEvent(ev);

	return false;

}

void UIElement::Tick()
{
}

void UIElement::Draw()
{
}

bool UIElement::HandleEvent(const SDL_Event* ev)
{
	return false;
}

void UIElement::HandleResize()
{
}

void UIElement::HandleParentResize()
{
}

UIElement* UIElement::GetParent()
{
	return mParent;
}

void UIElement::SetParent(UIElement* newParent)
{

	if (mParent != nullptr)
	{
		mParent->RemoveChild(this);
	}

	mParent = newParent;

	if (mParent != nullptr)
	{
		mParent->mChildren.push_back(this);
	}

}

const std::vector<UIElement*>& UIElement::GetChildren()
{
	return mChildren;
}

void UIElement::RemoveChild(UIElement* child)
{
	for (std::vector<UIElement*>::iterator it = mChildren.begin();
		it != mChildren.end(); ++it)
	{
		if ((*it) == child)
		{
			// sadly it-- is not always possible.
			// so for now just not expect it to be added twice...
			mChildren.erase(it);
			return;
		}
	}
}

const Rect& UIElement::GetClipRect()
{
	return mClipRect;
}

const Rect& UIElement::GetClientRect()
{
	return mClientRect;
}

void UIElement::UpdateClipRect()
{
	// clip against all parent rects
	Rect clientRect = GetClientRect();
	UIElement* p = mParent;
	while (p)
	{
		clientRect.x += p->mClientRect.x;
		clientRect.y += p->mClientRect.y;
		clientRect = clientRect.GetIntersection(p->mClientRect);
		p = p->mParent;
	}
	mClipRect = clientRect;
}

void UIElement::SetFlags(UIElementFlags flags)
{
	mFlags = flags;
}

UIElementFlags UIElement::GetFlags()
{
	return mFlags;
}

bool UIElement::SetVisible(bool visible)
{
	if (visible) mFlags = mFlags | UIElementFlags::Visible;
	else mFlags = mFlags & ~UIElementFlags::Visible;
	return visible;
}

bool UIElement::SetTransparent(bool transparent)
{
	if (transparent) mFlags = mFlags | UIElementFlags::Transparent;
	else mFlags = mFlags & ~UIElementFlags::Transparent;
	return transparent;
}

bool UIElement::SetFocused(bool focused)
{
	if (!IsFocusable())
		focused = false;

	if (focused) mFlags = mFlags | UIElementFlags::Focused;
	else mFlags = mFlags & ~UIElementFlags::Focused;

	if (focused && mParent != nullptr)
	{
		for (auto& child : mParent->mChildren)
		{
			if (child != this)
				child->SetFocused(false);
		}
		mParent->SetFocused(true);
	}
	return focused;
}

bool UIElement::SetFocusable(bool focusable)
{
	if (focusable) mFlags = mFlags | UIElementFlags::Focusable;
	else mFlags = mFlags & ~UIElementFlags::Focusable;
	return focusable;
}

void UIElement::Close()
{
	mIsClosing = true;
}

void UIElement::SetClientRect(const Rect& newClientRect)
{
	mClientRect = newClientRect;
	PropagateClip(false);
	HandleResize();
}

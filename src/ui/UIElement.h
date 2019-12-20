#pragma once

#include <SDL.h>
#include <vector>
#include "../screen/Rect.h"

enum class UIElementFlags : uint16_t
{
	NoFlags = 0x0000,
	Visible = 0x0001,
	Transparent = 0x0002,
	Focused = 0x0004,
	Focusable = 0x0008
};

inline UIElementFlags operator|(UIElementFlags lhs, UIElementFlags rhs)
{
	return static_cast<UIElementFlags>(static_cast<uint16_t>(lhs) | static_cast<uint16_t>(rhs));
}

inline UIElementFlags operator&(UIElementFlags lhs, UIElementFlags rhs)
{
	return static_cast<UIElementFlags>(static_cast<uint16_t>(lhs)& static_cast<uint16_t>(rhs));
}

inline UIElementFlags operator~(UIElementFlags flag)
{
	return static_cast<UIElementFlags>(~static_cast<uint16_t>(flag));
}


class UIElement
{
public:
	explicit UIElement(UIElement* parent = nullptr);
	virtual ~UIElement();

	void PropagateClip(bool haveParentResize);
	void PropagateTick();
	void PropagateDraw();
	bool PropagateEvent(const SDL_Event* ev);
	bool BubbleEvent(const SDL_Event* ev);
	
	virtual void Tick();
	virtual void Draw();
	virtual bool HandleEvent(const SDL_Event* ev);
	virtual void HandleResize();
	virtual void HandleParentResize();

	UIElement* GetParent();
	void SetParent(UIElement* newParent);
	const std::vector<UIElement*>& GetChildren();
	void RemoveChild(UIElement* child);

	const Rect& GetClipRect();
	const Rect& GetClientRect();

	void SetClientRect(const Rect& newClientRect);

	void SetFlags(UIElementFlags flags);
	UIElementFlags GetFlags();
	inline bool IsVisible() { return (mFlags & UIElementFlags::Visible) != UIElementFlags::NoFlags; }
	inline bool IsTransparent() { return (mFlags & UIElementFlags::Transparent) != UIElementFlags::NoFlags; }
	inline bool IsFocused() { return (mFlags & UIElementFlags::Focused) != UIElementFlags::NoFlags; }
	inline bool IsFocusable() { return (mFlags & UIElementFlags::Focusable) != UIElementFlags::NoFlags; }
	bool SetVisible(bool visible);
	bool SetTransparent(bool transparent);
	bool SetFocused(bool focused);
	bool SetFocusable(bool focusable);

private:

	UIElement* mParent = nullptr;
	std::vector<UIElement*> mChildren;
	Rect mClientRect;
	Rect mClipRect;
	UIElementFlags mFlags = UIElementFlags::NoFlags;

	void UpdateClipRect();

};
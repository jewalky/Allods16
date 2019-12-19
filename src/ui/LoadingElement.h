#pragma once

#include "UIElement.h"
#include "../Thread.h"

class LoadingElement : public UIElement
{
public:
	explicit LoadingElement(UIElement* parent = nullptr);
	bool IsLoading();
	
	virtual void Tick();

protected:
	virtual void LoadingThread();

private:
	bool mLoading = false;

	class LoadingThreadImpl : public Thread
	{
		LoadingElement* mElement;

	public:
		LoadingThreadImpl(LoadingElement* el) : Thread()
		{
			mElement = el;
		}

		virtual int Run()
		{
			mElement->LoadingThread();
			RLock lock(mElement->mLoadingMutex);
			mElement->mLoading = false;
			return 0;
		}
	};

	Thread* mLoadingThread = nullptr;
	Mutex mLoadingMutex;
};
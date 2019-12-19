#include "LoadingElement.h"

LoadingElement::LoadingElement(UIElement* parent) : UIElement(parent)
{
	mLoading = true;
	mLoadingThread = nullptr;
}

void LoadingElement::Tick()
{
	if (mLoadingThread == nullptr)
	{
		mLoadingThread = new LoadingThreadImpl(this);
		mLoadingThread->Start();
	}
}

void LoadingElement::LoadingThread()
{
	// default does nothing
}

bool LoadingElement::IsLoading()
{
	RLock lock(mLoadingMutex);
	return mLoading;
}

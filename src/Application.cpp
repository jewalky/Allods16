#include <SDL.h>
#include "Application.h"
#include "logging.h"

#include "draw/DrawingContext.h"
#include "data/Resource.h"

#include "mapview/MapView.h"

Application* Application::mApplication = nullptr;
std::vector<std::string> Application::mArguments;

void Application::SetArguments(int argc, const char* argv[])
{
	mArguments.resize(argc);
	for (int i = 0; i < argc; i++)
		mArguments[i] = argv[i];
}

Application* Application::GetInstance()
{
	if (mApplication == nullptr)
		mApplication = new Application();
	return mApplication;
}

Application::Application()
{
	mScreen = nullptr;
}

Application::~Application()
{
	if (mScreen != nullptr)
		delete mScreen;
	mScreen = nullptr;
}

Screen* Application::GetScreen()
{
	return mScreen;
}

ResourceManager* Application::GetResources()
{
	return mResources;
}

Mouse* Application::GetMouse()
{
	return mMouse;
}

RootUIElement* Application::GetUIRoot()
{
	return mUIRoot;
}

void Application::Exit()
{
	mExiting = true;
}

void Application::Abort(std::string message)
{
	Printf("\n!! ABORTED !!\nWith message: %s\n", message);
	SDL_Quit();
	exit(3);
}

uint64_t Application::GetTicks()
{
	return uint64_t(SDL_GetTicks());
}

int Application::Run()
{
	mScreen = new Screen(1024, 768);
	if (!mScreen->IsValid())
	{
		Printf("Failed to initialize screen");
		return 1;
	}

	mResources = new ResourceManager();
	mMouse = new Mouse();
	mMouse->SetCursor(Mouse::Default);
	mUIRoot = new RootUIElement();

	// load ALM :D


	DrawingContext ctx(mScreen);
	ctx.ClearRect(ctx.GetViewport(), Color(0, 0, 0, 255));

	// start loading data.bin & reg files
	mTemplateLoader = new TemplateLoader(mUIRoot);

	while (!mExiting)
	{
		mUIRoot->PropagateTick();

		// if initial game data is loaded, delete loader and create initial screen
		if (mTemplateLoader != nullptr && !mTemplateLoader->IsLoading())
		{
			delete mTemplateLoader;
			mTemplateLoader = nullptr;
			new MapView(mUIRoot, "kids3.alm");
		}
		
		SDL_Event ev;
		while (mScreen->PollEvent(ev))
			HandleEvent(&ev);

		mUIRoot->PropagateDraw();

		mMouse->PreApply();
		mScreen->Apply();
		mMouse->PostApply();

		if (mScreen->GetFPS() > 60)
			SDL_Delay(1);
	}
	
	return 0;
}

void Application::HandleEvent(const SDL_Event* ev)
{
	switch (ev->type)
	{
	case SDL_QUIT:
		mExiting = true;
		break;
	case SDL_MOUSEMOTION:
		mMouse->SetPosition(Point(ev->motion.x, ev->motion.y));
		break;
	default:
		break;
	}

	mUIRoot->PropagateEvent(ev);
}
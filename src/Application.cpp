#include <SDL.h>
#include "Application.h"
#include "logging.h"

#include "draw/DrawingContext.h"
#include "File.h"

#include <iostream>

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

int Application::Run()
{
	mScreen = new Screen(640, 480);
	if (!mScreen->IsValid())
	{
		Printf("Failed to initialize screen");
		return 1;
	}

	while (!mExiting)
	{
		SDL_Event ev;
		while (mScreen->PollEvent(ev))
			HandleEvent(&ev);

		DrawingContext ctx(mScreen);
		ctx.ClearRect(ctx.GetViewport(), Color(0, 0, 0, 255));
		ctx.DrawLine(Point(640, 480), Point(0, 0), Color(255, 255, 255, 128));

		mScreen->Apply();
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
	default:
		break;
	}
}
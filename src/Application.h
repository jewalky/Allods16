#pragma once

#include <vector>
#include <string>
#include "screen/Screen.h"
#include "data/Resource.h"
#include "ui/Mouse.h"
#include "ui/RootUIElement.h"
#include "maplogic/MapLogic.h"
#include "templates/TemplateLoader.h"

class Application
{
public:

	static void SetArguments(int argc, const char* argv[]);
	static Application* GetInstance();
	Screen* GetScreen();

	int Run();

	void Exit();
	void Abort(std::string message);

	static uint64_t GetTicks();

	//
	ResourceManager* GetResources();
	Mouse* GetMouse();
	RootUIElement* GetUIRoot();

private:

	Application();
	~Application();
	static Application* mApplication;

	static std::vector<std::string> mArguments;

	Screen* mScreen;
	bool mExiting = false;

	//
	void HandleEvent(const SDL_Event* ev);

	//
	ResourceManager* mResources;
	Mouse* mMouse;
	RootUIElement* mUIRoot;
	// loads data.bin and .reg files
	TemplateLoader* mTemplateLoader;

};
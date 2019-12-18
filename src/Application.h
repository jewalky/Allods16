#pragma once

#include <vector>
#include <string>
#include "screen/Screen.h"

class Application
{
public:

	static void SetArguments(int argc, const char* argv[]);
	static Application* GetInstance();
	Screen* GetScreen();

	int Run();

	void Exit();
	void Abort(std::string message);

private:
	Application();
	~Application();
	static Application* mApplication;

	static std::vector<std::string> mArguments;

	Screen* mScreen;
	bool mExiting = false;

	//
	void HandleEvent(const SDL_Event* ev);
};
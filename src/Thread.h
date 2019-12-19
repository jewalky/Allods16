#pragma once

#include <SDL.h>
#include <string>

class Thread
{
public:
	Thread();
	Thread(const std::string& name);
	virtual ~Thread() {}

	void Start();
	//void Stop();
	virtual int Run() { return 0; };
	int Wait();

private:
	std::string mName;
	SDL_Thread* mThread = nullptr;
	static int RunThread(void* ptr);
};

class RLock;
class Mutex
{
public:
	Mutex();
	virtual ~Mutex();

private:
	SDL_mutex* mMutex;
	friend class RLock;
};

class RLock
{
public:
	explicit RLock(const Mutex& mutex);
	~RLock();

	void Unlock();

private:
	SDL_mutex* mMutex;
	friend class Mutex;
};
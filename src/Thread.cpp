#include "Thread.h"
#include "utils.h"

static int ThreadCounter = 0;

Thread::Thread()
{
	mName = Format("Thread-%d", ThreadCounter++);
}

Thread::Thread(const std::string& name)
{
	mName = name;
}

void Thread::Start()
{
	mThread = SDL_CreateThread(&RunThread, mName.c_str(), this);
}

int Thread::RunThread(void* ptr)
{
	Thread* thread = (Thread*)ptr;
	return thread->Run();
}

int Thread::Wait()
{
	int status;
	SDL_WaitThread(mThread, &status);
	return status;
}

/////////

Mutex::Mutex()
{
	mMutex = SDL_CreateMutex();
}

Mutex::~Mutex()
{
	if (mMutex != nullptr)
		SDL_DestroyMutex(mMutex);
	mMutex = nullptr;
}

/////////

RLock::RLock(const Mutex& mutex)
{
	mMutex = mutex.mMutex;
	SDL_LockMutex(mMutex);
}

RLock::~RLock()
{
	Unlock();
}

void RLock::Unlock()
{
	if (mMutex == nullptr)
		return;

	SDL_mutex* uMutex = mMutex;
	mMutex = nullptr;
	SDL_UnlockMutex(uMutex);
}
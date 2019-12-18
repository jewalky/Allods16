#pragma once

#include <iostream>
#include "utils.h"

inline void Printf(const char* s)
{
	std::cout << s << std::endl;
}

template<typename... Args>
void Printf(const char* s, Args... args)
{
	std::string ps = Format(s, args...);
	std::cout << ps << std::endl;
}

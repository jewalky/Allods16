#pragma once

#include <string>
#include <vector>
#include "../MemoryStream.h"

#define RESOURCE_SIGNATURE 0x31415926

class Resource
{
public:

	Resource(const std::string& filename);

	bool Open();

	operator bool() { return IsValid(); }
	bool IsValid();

	bool CheckExists(const std::string& path);
	bool ReadFile(MemoryStream& target, const std::string& path);

private:

	struct Entry
	{
		bool mIsDirectory = false;
		std::string mName;
		uint32_t mOffset = 0;
		uint32_t mSize = 0;
		std::vector<Entry> mChildren;
	};

	Entry mRoot;
	std::string mPath;
	std::string mBaseName;
	bool mIsValid;

	bool OpenTreeTraverse(Stream& f, Entry& e, uint32_t fat_offset, uint32_t first, uint32_t last);
	Entry* FindEntry(const std::string& path);

};

class ResourceManager
{
public:
	ResourceManager();

	bool CheckExists(const std::string& path);
	bool ReadFile(MemoryStream& target, const std::string& path);

private:
	void AddResource(const std::string& path);
	std::vector<Resource> mResources;
};
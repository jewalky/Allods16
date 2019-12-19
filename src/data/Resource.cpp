#include "Resource.h"
#include "../utils.h"
#include "../logging.h"
#include "../File.h"
#include "../Application.h"

Resource::Resource(const std::string& filename)
{
	mPath = filename;
	mBaseName = ToLower(Explode(Basename(filename), ".")[0]);
	mIsValid = false;
}

bool Resource::OpenTreeTraverse(Stream& f, Entry& e, uint32_t fat_offset, uint32_t first, uint32_t last)
{
	f.SetPosition(uint64_t(fat_offset) + uint64_t(first) * 0x20);
	for (uint32_t i = first; i < last; i++)
	{
		f.SetPosition(f.GetPosition() + 4);
		
		Entry childEntry;

		uint32_t e_offset = f.ReadUInt32();
		uint32_t e_size = f.ReadUInt32();
		uint32_t e_type = f.ReadUInt32();
		
		childEntry.mName = ToLower(f.ReadString(16));
		
		if (e_type == 1)
		{
			childEntry.mIsDirectory = true;
			if (!OpenTreeTraverse(f, childEntry, fat_offset, e_offset, e_offset + e_size))
				return false;
		}
		else if (e_type == 0)
		{
			childEntry.mOffset = e_offset;
			childEntry.mSize = e_size;
		}
		else
		{
			Printf("Warning: invalid entry type in \"%s\": %08X", mPath, e_type);
			return false;
		}

		e.mChildren.push_back(childEntry);
	}

	return true;
}

bool Resource::Open()
{
	
	File f(mPath, File_OpenRead);
	if (!f.Open())
	{
		Printf("Warning: couldn't open \"%s\"", mPath);
		return false;
	}

	uint32_t signature = f.ReadUInt32();
	if (signature != RESOURCE_SIGNATURE)
	{
		Printf("Warning: invalid RES signature 0x%08X", signature);
		return false;
	}

	uint32_t root_offset = f.ReadUInt32();
	uint32_t root_size = f.ReadUInt32();
	f.SetPosition(f.GetPosition() + 4);
	uint32_t fat_offset = f.ReadUInt32();

	mRoot.mIsDirectory = true;
	mRoot.mName = mBaseName;
	
	if (!OpenTreeTraverse(f, mRoot, fat_offset, root_offset, root_offset + root_size))
		return false;
	
	mIsValid = true;
	return true;

}

bool Resource::IsValid()
{
	return mIsValid;
}

Resource::Entry* Resource::FindEntry(const std::string& path)
{

	std::vector<std::string> splitPath = Explode(FixSlashes(ToLower(path)), "/");
	if (splitPath[0] != mBaseName)
		return nullptr;
	splitPath.erase(splitPath.begin());

	int pathCount = 0;
	Entry* topEntry = &mRoot;
	for (auto part : splitPath)
	{

		Entry* nextEntry = nullptr;
		for (auto& child : topEntry->mChildren)
		{
			if (part == child.mName)
			{
				pathCount++;
				nextEntry = &child;
				break;
			}
		}

		topEntry = nextEntry;
		if (!topEntry)
			break;

	}

	if (!topEntry)
		return nullptr;

	if (pathCount != splitPath.size())
		return nullptr;
	
	return topEntry;

}

bool Resource::CheckExists(const std::string& path)
{
	
	Entry* entry = FindEntry(path);
	return !!entry;

}

bool Resource::ReadFile(MemoryStream& target, const std::string& path)
{
	
	Entry* entry = FindEntry(path);
	if (!entry) return false;

	File f(mPath, File_OpenRead);
	if (!f.Open())
	{
		Printf("Warning: couldn't open \"%s\": file is gone", mPath);
		return false;
	}

	if (f.SetPosition(entry->mOffset) != entry->mOffset)
	{
		Printf("Warning: couldn't seek to 0x%08X in \"%s\"", entry->mOffset, mPath);
		return false;
	}

	std::vector<uint8_t> bytes;
	bytes.resize(entry->mSize);
	if (f.ReadBytes(bytes.data(), entry->mSize) != entry->mSize)
	{
		Printf("Warning: couldn't read %d bytes at 0x%08X in \"%s\"", entry->mSize, entry->mOffset, mPath);
		return false;
	}

	target.SetBuffer(bytes);
	
	f.Close();
	return true;

}

/////////////

ResourceManager::ResourceManager()
{
	AddResource("main.res");
	AddResource("graphics.res");
	AddResource("world.res");
	AddResource("patch.res");
	AddResource("scenario.res");
}

bool ResourceManager::CheckExists(const std::string& path)
{

	if (FileExists(path))
		return true;

	for (auto& res : mResources)
	{
		if (res.CheckExists(path))
			return true;
	}

	return false;

}

bool ResourceManager::ReadFile(MemoryStream& target, const std::string& path)
{
	
	// first try real filesystem
	File f(path, File_OpenRead);
	if (f.Open())
	{
		std::vector<uint8_t> bytes;
		uint64_t fileLen = f.GetLength();
		bytes.resize(fileLen);
		if (f.ReadBytes(bytes.data(), fileLen) != fileLen)
		{
			Printf("Warning: couldn't read %d bytes from \"%s\"", fileLen, path);
			return false;
		}
		target.SetBuffer(bytes);
		return true;
	}

	// check resources
	for (auto& res : mResources)
	{
		if (res.ReadFile(target, path))
			return true;
	}

	return false; 

}

void ResourceManager::AddResource(const std::string& path)
{
	mResources.push_back(Resource(path));
	if (!mResources.back().Open())
		Application::GetInstance()->Abort(Format("Critical: couldn't load \"%s\"", path));
}
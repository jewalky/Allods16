#include "Registry.h"
#include "../utils.h"
#include "../Application.h"

RegistryValue::RegistryValue()
{
	mType = RegistryValueType::Null;
	mIsPresent = false;
	mValueS = "";
	mValueI = 0;
	mValueF = 0;
}

RegistryValue::RegistryValue(const std::string& valueS)
{
	mType = RegistryValueType::String;
	mIsPresent = true;
	mValueS = valueS;
	if (CheckFloat(valueS))
	{
		mValueF = StrToFloat(valueS);
		mValueI = int32_t(mValueF);
	}
	else if (CheckInt(valueS))
	{
		mValueI = StrToInt(valueS);
		mValueF = double_t(mValueI);
	}
	else if (CheckHex(valueS))
	{
		mValueI = HexToInt(valueS);
		mValueF = double_t(mValueI);
	}
	mValueA.push_back(mValueI);
}

RegistryValue::RegistryValue(double_t valueF)
{
	mType = RegistryValueType::Float;
	mIsPresent = true;
	mValueF = valueF;
	mValueI = int32_t(valueF);
	mValueS = Format("%f", valueF);
	mValueA.push_back(mValueI);
}

RegistryValue::RegistryValue(int32_t valueI)
{
	mType = RegistryValueType::Integer;
	mIsPresent = true;
	mValueI = valueI;
	mValueF = double_t(valueI);
	mValueS = Format("%d", valueI);
	mValueA.push_back(mValueI);
}

RegistryValue::RegistryValue(const std::vector<int32_t>& valueA)
{
	mType = RegistryValueType::Array;
	mIsPresent = true;
	if (valueA.size())
	{
		mValueI = valueA[0];
		mValueF = double_t(mValueI);
		mValueS = Format("%d", mValueI);
	}
	else
	{
		mValueI = 0;
		mValueF = 0;
		mValueS = "";
	}
	mValueA = valueA;
}

RegistryValue::RegistryValue(const RegistryValue& other)
{
	mType = other.mType;
	mIsPresent = other.mIsPresent;
	mValueI = other.mValueI;
	mValueS = other.mValueS;
	mValueA = other.mValueA;
	mValueF = other.mValueF;
}

bool RegistryValue::IsPresent() const
{
	return mIsPresent;
}

RegistryValueType RegistryValue::GetType() const
{
	return mType;
}

const std::string& RegistryValue::AsString() const
{
	return mValueS;
}

double_t RegistryValue::AsFloat() const
{
	return mValueF;
}

int32_t RegistryValue::AsInteger() const
{
	return mValueI;
}

const std::vector<int32_t>& RegistryValue::AsArray() const
{
	return mValueA;
}

bool Registry::TreeTraverse(Stream& f, Entry& e, uint32_t first, uint32_t last, uint32_t data_origin)
{
	
	for (uint32_t i = first; i < last; i++)
	{

		f.SetPosition(0x18 + 0x20 * i);
		
		e.mChildren.push_back(Entry());
		Entry& childEntry = e.mChildren.back();

		f.SkipBytes(4);
		uint32_t e_offset = f.ReadUInt32();
		uint32_t e_count = f.ReadUInt32();
		uint32_t e_type = f.ReadUInt32();
		childEntry.mName = ToLower(f.ReadString(16));

		if (e_type == 0) // string value
		{
			f.SetPosition(data_origin + e_offset);
			childEntry.mValue = RegistryValue(f.ReadString(e_count));
		}
		else if (e_type == 2) // dword value
		{
			childEntry.mValue = RegistryValue(int32_t(e_offset));
		}
		else if (e_type == 4) // float value
		{
			uint32_t repl[] = { e_offset, e_count };
			double_t v = *(double_t*)repl;
			childEntry.mValue = RegistryValue(v);
		}
		else if (e_type == 6) // int array
		{
			if ((e_count % 4) != 0)
			{
				Printf("Warning: invalid REG float array byte size: %u % 4 != 0", e_count);
				return false;
			}

			uint32_t e_acount = e_count / 4;
			std::vector<int32_t> values;
			values.resize(e_acount);
			f.SetPosition(data_origin + e_offset);
			for (uint32_t i = 0; i < e_acount; i++)
				values[i] = f.ReadInt32();
			childEntry.mValue = RegistryValue(values);
		}
		else if (e_type == 1) // directory
		{
			childEntry.mIsDirectory = true;
			if (!TreeTraverse(f, childEntry, e_offset, e_offset + e_count, data_origin))
				return false;
		}

	}

	return true;

}

Registry::Entry* Registry::FindEntry(const std::string& path)
{

	std::vector<std::string> splitPath = Explode(FixSlashes(ToLower(path)), "/");

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

const RegistryValue& Registry::GetValue(const std::string& path)
{
	
	Entry* entry = FindEntry(path);
	if (entry == nullptr || entry->mIsDirectory)
		return mNotPresent;

	return entry->mValue;

}


Registry::Registry(const std::string& path)
{

	MemoryStream ms;
	if (!Application::GetInstance()->GetResources()->ReadFile(ms, path))
		Application::GetInstance()->Abort(Format("Couldn't load \"%s\": couldn't open file", path));

	mRoot.mIsDirectory = true;
	mRoot.mName = "";

	uint32_t reg_signature = ms.ReadUInt32();
	uint32_t root_offset = ms.ReadUInt32();
	uint32_t root_size = ms.ReadUInt32();
	ms.SkipBytes(4);
	uint32_t eat_size = ms.ReadUInt32();
	ms.SkipBytes(4);

	if (reg_signature != REGISTRY_SIGNATURE)
		Application::GetInstance()->Abort(Format("Couldn't load \"%s\": invalid signature", path));

	if (!TreeTraverse(ms, mRoot, root_offset, root_offset + root_size, 0x1C + 0x20 * eat_size))
		Application::GetInstance()->Abort(Format("Couldn't load \"%s\": invalid EAT structure", path));

}
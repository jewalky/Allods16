#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "../Stream.h"

#define REGISTRY_SIGNATURE 0x31415926

enum class RegistryValueType : uint8_t
{
	String,
	Float,
	Integer,
	Array,
	Null // special type, returned when not found
};

class Registry;
class RegistryValue
{
private:

	bool mIsPresent = false;

	RegistryValueType mType;
	std::string mValueS;
	double_t mValueF = 0;
	int32_t mValueI = 0;
	std::vector<int32_t> mValueA;

public:

	RegistryValue();
	RegistryValue(const std::string& valueS);
	RegistryValue(double_t valueF);
	RegistryValue(int32_t valueI);
	RegistryValue(const std::vector<int32_t>& valueA);
	RegistryValue(const RegistryValue& other);

	operator bool() const { return IsPresent(); }
	bool IsPresent() const;

	RegistryValueType GetType() const;

	const std::string& AsString() const;
	double_t AsFloat() const;
	int32_t AsInteger() const;
	const std::vector<int32_t>& AsArray() const;

};

class Registry 
{
public:
	
	Registry();
	Registry(const std::string& path);
	const RegistryValue& GetValue(const std::string& path);

private:
	
	struct Entry
	{
		bool mIsDirectory = false;
		std::string mName;
		RegistryValue mValue;
		std::vector<Entry> mChildren;
	};

	Entry mRoot;
	RegistryValue mNotPresent;

	bool TreeTraverse(Stream& f, Entry& e, uint32_t first, uint32_t last, uint32_t data_origin);
	Entry* FindEntry(const std::string& path);

};
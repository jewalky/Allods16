#pragma once

#include <cstdint>
#include "../Stream.h"

struct TplClass
{

	std::string mName;
	float_t mJunkFloat1;
	float_t mJunkFloat2;
	float_t mPrice;
	float_t mWeight;
	float_t mDamage;
	float_t mToHit;
	float_t mDefense;
	float_t mAbsorbtion;
	float_t mMagicVolume;

	bool LoadFromStream(Stream& stream);

};

struct TplMaterial : TplClass {};

struct TplModifier
{

	std::string mName;
	uint16_t mFieldCount;
	int32_t mManaCost;
	int32_t mAffectMin;
	int32_t mAffectMax;
	int32_t mUsableBy;
	int32_t mSlotsWarrior[12];
	int32_t mSlotsMage[12];

	bool LoadFromStream(Stream& stream);

};

struct TplArmor
{

	std::string mName;
	uint16_t mFieldCount;
	int32_t mShape;
	int32_t mMaterial;
	int32_t mPrice;
	int32_t mWeight;
	int32_t mSlot;
	int32_t mAttackType;
	int32_t mPhysicalMin;
	int32_t mPhysicalMax;
	int32_t mToHit;
	int32_t mDefense;
	int32_t mAbsorbtion;
	int32_t mRange;
	int32_t mCharge;
	int32_t mRelax;
	int32_t mTwoHanded;
	int32_t mSuitableFor;
	int32_t mOtherParam;
	uint16_t mClassesAllowed[8];

	bool LoadFromStream(Stream& stream);
	
	bool IsAllowed(int cls, int material)
	{
		if (cls < 0)
		{
			for (int i = 0; i < 8; i++)
				if (IsAllowed(i, material))
					return true;
			return false;
		}

		return !!(mClassesAllowed[cls] & (1 << material));
	}

};

struct TplMagicItem
{

	std::string mName;
	uint16_t mFieldCount;
	int32_t mPrice;
	int32_t mWeight;
	std::string mEffects;

	bool LoadFromStream(Stream& stream);

};

struct TplMonster
{

};

struct TplHuman
{

};

struct TplStructure
{

};

struct TplSpell
{

};

struct Templates
{



};
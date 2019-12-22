#include "ObstacleClass.h"
#include "../logging.h"
#include "../data/Registry.h"
#include "../mapview/MapView.h"

std::vector<ObstacleClass> ObstacleClassManager::mObjects;

ObstacleClass* ObstacleClassManager::GetByID(uint32_t id)
{
	
	ObstacleClass* obstacles = mObjects.data();
	for (size_t i = 0; i < mObjects.size(); i++)
	{
		if (obstacles->mID == id)
			return obstacles;
		obstacles++;
	}

	return nullptr;

}

struct ObstacleClassTmp
{
	ObstacleClassTmp() {}
	RegistryValue DescText;
	RegistryValue ID;
	RegistryValue File;
	RegistryValue Index;
	RegistryValue CenterX;
	RegistryValue CenterY;
	RegistryValue Width;
	RegistryValue Height;
	RegistryValue Phases;
	RegistryValue AnimationTime;
	RegistryValue AnimationFrame;
	RegistryValue DeadObject;
	RegistryValue Parent;
};

void ObstacleClassManager::Load()
{

	Registry reg("graphics/objects/objects.reg");

	uint32_t ObjectCount = reg.GetValue("Global/ObjectCount").AsInteger();
	uint32_t FileCount = reg.GetValue("Global/FileCount").AsInteger();

	std::vector<ObstacleClassTmp> obstaclesTmp;
	obstaclesTmp.resize(ObjectCount);

	for (uint32_t i = 0; i < ObjectCount; i++)
	{

		ObstacleClassTmp& ob = obstaclesTmp[i];

		ob.DescText = reg.GetValue(Format("Object%d/DescText", i));
		ob.ID = reg.GetValue(Format("Object%d/ID", i));
		ob.File = reg.GetValue(Format("Object%d/File", i));
		ob.Phases = reg.GetValue(Format("Object%d/Phases", i));
		ob.Index = reg.GetValue(Format("Object%d/Index", i));
		ob.AnimationTime = reg.GetValue(Format("Object%d/AnimationTime", i));
		ob.AnimationFrame = reg.GetValue(Format("Object%d/AnimationFrame", i));
		ob.CenterX = reg.GetValue(Format("Object%d/CenterX", i));
		ob.CenterY = reg.GetValue(Format("Object%d/CenterY", i));
		ob.Width = reg.GetValue(Format("Object%d/Width", i));
		ob.Height = reg.GetValue(Format("Object%d/Height", i));
		ob.DeadObject = reg.GetValue(Format("Object%d/DeadObject", i));
		ob.Parent = reg.GetValue(Format("Object%d/Parent", i));

		if (!ob.ID)
			Printf("Warning: Obstacle #%d does not have ID", i);

	}

	// resolve parents
	for (uint32_t i = 0; i < ObjectCount; i++)
	{

		ObstacleClassTmp& ob = obstaclesTmp[i];
		int32_t cur_id = ob.ID.AsInteger();
		int32_t id = -1;
		if (ob.Parent)
			id = ob.Parent.AsInteger();

		while (id != -1)
		{

			ObstacleClassTmp* clsp = nullptr;
			for (uint32_t j = 0; j < ObjectCount; j++)
			{
				if (obstaclesTmp[j].ID && obstaclesTmp[j].ID.AsInteger() == id)
				{
					clsp = &obstaclesTmp[j];
					break;
				}
			}

			if (clsp == nullptr)
			{
				Printf("Warning: Parent %d specified for Obstacle ID %d, but not found", id, cur_id);
				break;
			}

			// put fields from the other obstacle where ours are not present
			if (!ob.DescText && clsp->DescText)
				ob.DescText = clsp->DescText;
			if (!ob.File && clsp->File)
				ob.File = clsp->File;
			if (!ob.Phases && clsp->Phases)
				ob.Phases = clsp->Phases;
			if (!ob.Index && clsp->Index)
				ob.Index = clsp->Index;
			if (!ob.AnimationTime && clsp->AnimationTime)
				ob.AnimationTime = clsp->AnimationTime;
			if (!ob.AnimationFrame && clsp->AnimationFrame)
				ob.AnimationFrame = clsp->AnimationFrame;
			if (!ob.CenterX && clsp->CenterX)
				ob.CenterX = clsp->CenterX;
			if (!ob.CenterY && clsp->CenterY)
				ob.CenterY = clsp->CenterY;
			if (!ob.Width && clsp->Width)
				ob.Width = clsp->Width;
			if (!ob.Height && clsp->Height)
				ob.Height = clsp->Height;
			if (!ob.DeadObject && clsp->DeadObject)
				ob.DeadObject = clsp->DeadObject;

			cur_id = clsp->ID.AsInteger();
			id = -1;
			if (clsp->Parent)
				id = clsp->Parent.AsInteger();

		}

	}

	// convert reg objects to user objects
	mObjects.resize(ObjectCount);
	for (uint32_t i = 0; i < ObjectCount; i++)
	{

		ObstacleClass& cls = mObjects[i];
		ObstacleClassTmp& ob = obstaclesTmp[i];

		cls.mID = ob.ID ? ob.ID.AsInteger() : -1;
		cls.mDescText = ob.DescText ? ob.DescText.AsString() : "";
		cls.mDeadObject = ob.DeadObject ? ob.DeadObject.AsInteger() : -1;

		if (ob.CenterX && ob.CenterY && ob.Width && ob.Height)
		{
			float fW = ob.Width.AsInteger();
			float fH = ob.Height.AsInteger();
			float fX = ob.CenterX.AsInteger();
			float fY = ob.CenterY.AsInteger();
			cls.mCenterX = fX / fW;
			cls.mCenterY = fY / fH;
		}
		else
		{
			cls.mCenterX = cls.mCenterY = 0.5;
		}

		int phaseIndex = ob.Index ? ob.Index.AsInteger() : 0;
		int phases = ob.Phases ? ob.Phases.AsInteger() : 0;
		phaseIndex *= phases;

		if (ob.AnimationTime && ob.AnimationFrame)
		{
			std::vector<int32_t> animTime = ob.AnimationTime.AsArray();
			std::vector<int32_t> animFrame = ob.AnimationFrame.AsArray();
			if (animTime.size() == animFrame.size())
			{
				cls.mFrames.resize(animTime.size());
				for (size_t j = 0; j < animTime.size(); j++)
				{
					cls.mFrames[j].mTime = animTime[j];
					cls.mFrames[j].mFrame = animFrame[j]+phaseIndex;
				}
			}
			else
			{
				Printf("Warning: invalid animation in Obstacle ID %d: AnimationFrame has length %d, AnimationTime has length %d", cls.mID, animFrame.size(), animTime.size());
			}
		}
		else
		{
			cls.mFrames.resize(1);
			cls.mFrames[0].mTime = -1;
			cls.mFrames[0].mFrame = phaseIndex;
		}

		if (ob.File && ob.File.AsInteger() >= 0)
		{
			uint32_t fileNum = ob.File.AsInteger();
			if (fileNum < FileCount)
			{
				RegistryValue filePath = reg.GetValue(Format("Files/File%d", fileNum));
				if (filePath && filePath.AsString().length())
				{
					cls.mFile.mPath = filePath.AsString();
				}
				else
				{
					Printf("Warning: invalid File #%u (missing) for Obstacle ID %d", fileNum, cls.mID);
				}
			}
			else
			{
				Printf("Warning: invalid File #%u (total %d) for Obstacle ID %d", fileNum, FileCount, cls.mID);
			}
		}
		else
		{
			Printf("Warning: missing File for Obstacle ID %d", cls.mID);
		}

	}

}

void ObstacleFile::CheckLoad(MapView* view)
{

	if (mSprite == nullptr)
		mSprite = new Sprite256("graphics/objects/" + mPath + ".256");
	if (mSpriteB == nullptr)
		mSpriteB = new Sprite256("graphics/objects/" + mPath + "b.256");

	if (view != nullptr)
	{
		const CompoundPalette* pal = mPalettes[view];
		if (pal == nullptr)
			mPalettes[view] = view->AllocateCompoundPalette(mSprite->GetPalette());
	}

}

const CompoundPalette* ObstacleFile::GetPalette(MapView* view)
{
	return mPalettes[view];
}
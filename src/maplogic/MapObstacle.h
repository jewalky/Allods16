#pragma once

#include "MapObject.h"
#include "../templates/ObstacleClass.h"

class MapObstacle : public MapObject
{
public:

	MapObstacle(MapLogic* logic, uint32_t id);
	virtual ~MapObstacle();

	bool IsValid();

	virtual uint16_t GetNodeLinkFlags();

	virtual bool Tick();
	virtual void Draw(MapView* view);

private:

	ObstacleClass* mClass;
	uint32_t mTime;
	uint32_t mFrame;

};
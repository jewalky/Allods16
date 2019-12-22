#pragma once

#include <string>
#include "Sprite.h"

class Sprite256 : public Sprite
{
public:

	Sprite256(const std::string& path);
	virtual void Draw(DrawingContext& ctx, int32_t x, int32_t y, uint32_t index, const Color* palette);

};
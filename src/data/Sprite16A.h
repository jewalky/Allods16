#pragma once

#include <string>
#include "Sprite.h"

class Sprite16A : public Sprite
{
public:
	
	Sprite16A(const std::string& path);
	virtual void Draw(DrawingContext& ctx, int32_t x, int32_t y, uint32_t index, const std::vector<Color>& palette);

};
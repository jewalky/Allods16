#pragma once

#include <vector>
#include "../screen/Color.h"

// this class provides generating 33 versions of a palette for light/shadow with the specified tint
// 0-15: dark
// 16: normal
// 17-33: light
class CompoundPalette
{
public:

	void SetBasePalette(const Color* basePalette);
	// brightness, contrast are from 0 to 255 (or larger) where 255 is normal
	void UpdatePalettes(Color tint, uint16_t brightness, uint16_t contrast);
	const Color* GetPalette(uint32_t index) const;

private:

	std::vector<Color> mBasePalette;
	std::vector<Color> mGeneratedPalettes;

	Color mLastTint;
	uint16_t mLastBrightness;
	uint16_t mLastContrast;

};
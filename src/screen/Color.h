#pragma once

#include <cstdint>

union Color
{
	struct {
		uint8_t b;
		uint8_t g;
		uint8_t r;
		uint8_t a;
	} components;
	uint32_t value;

	Color() {
		value = 0;
	}

	Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		components.r = r;
		components.g = g;
		components.b = b;
		components.a = a;
	}

	Color(uint32_t c)
	{
		value = c;
	}

	Color(const Color& c, uint8_t a)
	{
		value = c.value;
		components.a = a;
	}

	Color(uint32_t c, uint8_t a)
	{
		value = c;
		components.a = a;
	}
};
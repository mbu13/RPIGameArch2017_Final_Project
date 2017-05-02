/*
** RPI Game Architecture Engine
**
** Portions adapted from:
** Viper Engine - Copyright (C) 2016 Velan Studios - All Rights Reserved
**
** This file is distributed under the MIT License. See LICENSE.txt.
*/

#include "ga_label.h"
#include "ga_font.h"
#include <stdlib.h>

#include "framework/ga_frame_params.h"

ga_label::ga_label(const char* text, float x, float y, ga_frame_params* params)
{
	// TODO: Homework 4
	extern ga_font* g_font;
	ga_vec3f color = { 255, 255, 255 };

	ga_vec2f min;
	ga_vec2f max;

	g_font->print(params, text, x, y, color, &min, &max);
	_width = abs(min.x - max.x);
	_height = abs(min.y - max.y);
}

ga_label::~ga_label()
{
	// TODO: Homework 4
}

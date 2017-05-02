#pragma once

#include "ga_widget.h"
#include "ga_label.h"

#include "math/ga_vec2f.h"

/*
** GUI pressable button widget.
*/
class ga_textbox : public ga_widget
{
public:
	ga_textbox(const char* text, float x, float y, struct ga_frame_params* params);
	~ga_textbox();

	bool get_clicked(const struct ga_frame_params* params) const;
	void update_text(const char* text, float x, float y, ga_frame_params* params);
private:
	float _x;
	float _y;
	float _txt_width;
	float _txt_height;
	ga_label* txt_label;

	void ga_highlight(ga_frame_params* params);
};
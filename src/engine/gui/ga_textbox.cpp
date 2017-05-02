
#include "ga_textbox.h"

#include "framework/ga_frame_params.h"

#define X_OFFSET 6
#define Y_OFFSET 6

ga_textbox::ga_textbox(const char* text, float x, float y, ga_frame_params* params)
{
	// Draw temp label to get properties
	ga_label *temp = new ga_label(text, x - 1000, y - 1000, params);
	float width = 150;
	float height = 15;
	delete temp;

	_x = x;
	_y = y;
	_txt_width = width;
	_txt_height = height;

	// Detect mouse hover to highlight blue
	bool mouse_in_region =
		params->_mouse_x >= _x &&
		params->_mouse_y >= _y - _txt_height &&
		params->_mouse_x <= _x + _txt_width &&
		params->_mouse_y <= _y;

	ga_vec3f color = { 255, 255, 255 };
	if (mouse_in_region)
	{
		color = { 0, 0, 255 };

		if (params->_mouse_press_mask != 0)
		{
			ga_highlight(params);
		}
	}

	ga_dynamic_drawcall drawcall;

	drawcall._positions.push_back({ x, y, 0.0f });
	drawcall._positions.push_back({ x + width + X_OFFSET, y, 0.0f });
	drawcall._positions.push_back({ x + width + X_OFFSET, y - height - Y_OFFSET, 0.0f });
	drawcall._positions.push_back({ x, y - height - Y_OFFSET, 0.0f });

	drawcall._indices.push_back(0);
	drawcall._indices.push_back(1);
	drawcall._indices.push_back(1);
	drawcall._indices.push_back(2);
	drawcall._indices.push_back(2);
	drawcall._indices.push_back(3);
	drawcall._indices.push_back(3);
	drawcall._indices.push_back(0);

	drawcall._color = color;
	drawcall._draw_mode = GL_LINES;
	drawcall._transform.make_identity();
	drawcall._material = nullptr;

	while (params->_gui_drawcall_lock.test_and_set(std::memory_order_acquire)) {}
	params->_gui_drawcalls.push_back(drawcall);
	params->_gui_drawcall_lock.clear(std::memory_order_release);

	// Draw text label within button
	txt_label = new ga_label(text, x + X_OFFSET / 2, y - Y_OFFSET / 2, params);
}

void ga_textbox::update_text(const char* text, float x, float y, ga_frame_params* params)
{
	delete txt_label;

	txt_label = new ga_label(text, x + X_OFFSET / 2, y - Y_OFFSET / 2, params);
}

/**
* Helper function that highlights the button when being pressed
*/
void ga_textbox::ga_highlight(ga_frame_params* params) {
	ga_vec3f color = { 0, 0, 255 };

	ga_dynamic_drawcall drawcall;

	drawcall._positions.push_back({ _x, _y, 0.0f });
	drawcall._positions.push_back({ _x + _txt_width + X_OFFSET / 2, _y, 0.0f });
	drawcall._positions.push_back({ _x + _txt_width + X_OFFSET / 2, _y - _txt_height - Y_OFFSET / 2, 0.0f });
	drawcall._positions.push_back({ _x, _y - _txt_height - Y_OFFSET / 2, 0.0f });

	drawcall._indices.push_back(0);
	drawcall._indices.push_back(1);
	drawcall._indices.push_back(2);
	drawcall._indices.push_back(2);
	drawcall._indices.push_back(3);
	drawcall._indices.push_back(0);

	drawcall._color = color;
	drawcall._draw_mode = GL_TRIANGLES;
	drawcall._transform.make_identity();
	drawcall._material = nullptr;

	while (params->_gui_drawcall_lock.test_and_set(std::memory_order_acquire)) {}
	params->_gui_drawcalls.push_back(drawcall);
	params->_gui_drawcall_lock.clear(std::memory_order_release);
}

ga_textbox::~ga_textbox()
{
	delete txt_label;
}

bool ga_textbox::get_clicked(const ga_frame_params* params) const
{
	bool click_in_region =
		params->_mouse_click_mask != 0 &&
		params->_mouse_x >= _x &&
		params->_mouse_y >= _y - _txt_height &&
		params->_mouse_x <= _x + _txt_width &&
		params->_mouse_y <= _y;

	return click_in_region;
}

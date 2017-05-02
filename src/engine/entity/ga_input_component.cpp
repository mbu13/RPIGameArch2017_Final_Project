/*
** RPI Game Architecture Engine 2017
**
** Author: Matthew Bu
*/

#include "ga_input_component.h"
#include "entity/ga_entity.h"

#include "framework/ga_frame_params.h"

#include <iostream>

ga_input_component::ga_input_component(ga_entity* ent) : ga_component(ent)
{
	_has_moved = false;
	_move_left = false;
	_move_right = false;
	_move_forward = false;
	_move_backward = false;
	_lag_factor = 1;
}

ga_input_component::ga_input_component(ga_entity* ent, bool main) : ga_component(ent)
{
	_main = main;
	_has_moved = false;
	_move_left = false;
	_move_right = false;
	_move_forward = false;
	_move_backward = false;
	_lag_factor = 1;
}

void ga_input_component::update(ga_frame_params* params)
{
	if (_main)
	{
		if (frame_params_get_input_left(params))
		{
			_has_moved = true;
			_new_transform.x = -0.1;
			_new_transform.y = 0.0;
			_new_transform.z = 0.0;
			entity_translate(-0.1, 0.0, 0.0);
		}
		else if (frame_params_get_input_right(params))
		{
			_has_moved = true;
			_new_transform.x = 0.1;
			_new_transform.y = 0.0;
			_new_transform.z = 0.0;
			entity_translate(0.1, 0.0, 0.0);
		}
		else if (frame_params_get_input_forward(params))
		{
			_has_moved = true;
			_new_transform.x = 0.0;
			_new_transform.y = 0.1;
			_new_transform.z = 0.0;
			entity_translate(0.0, 0.1, 0.0);
		}
		else if (frame_params_get_input_backward(params))
		{
			_has_moved = true;
			_new_transform.x = 0.0;
			_new_transform.y = -0.1;
			_new_transform.z = 0.0;
			entity_translate(0.0, -0.1, 0.0);
		}
		else
		{
			_new_transform.x = 0.0;
			_new_transform.y = 0.0;
			_new_transform.z = 0.0;
			_has_moved = false;
		}
	}
	else
	{
		if (_move_left)
		{
			entity_translate(-0.1 * _lag_factor, 0.0, 0.0);
		}
		else if (_move_right)
		{
			entity_translate(0.1 * _lag_factor, 0.0, 0.0);
		}
		else if (_move_forward)
		{
			entity_translate(0.0, 0.1 * _lag_factor, 0.0);
		}
		else if (_move_backward)
		{
			entity_translate(0.0, -0.1 * _lag_factor, 0.0);
		}
		else 
		{
			entity_translate(0.0, 0.0, 0.0);
		}
	}
}

void ga_input_component::entity_translate(float x, float y, float z)
{
	ga_entity* ent = this->get_entity();
	ent->translate({ x, y, z });
}

bool ga_input_component::has_moved()
{
	return _has_moved;
}

bool ga_input_component::frame_params_get_input_left(ga_frame_params* params)
{
	if (params->_button_mask & k_button_j)
	{
		return true;
	}

	return false;
}

bool ga_input_component::frame_params_get_input_right(ga_frame_params* params)
{
	if (params->_button_mask & k_button_l)
	{
		return true;
	}

	return false;
}

bool ga_input_component::frame_params_get_input_forward(ga_frame_params* params)
{
	if (params->_button_mask & k_button_i)
	{
		return true;
	}

	return false;
}

bool ga_input_component::frame_params_get_input_backward(ga_frame_params* params)
{
	if (params->_button_mask & k_button_k)
	{
		return true;
	}

	return false;
}
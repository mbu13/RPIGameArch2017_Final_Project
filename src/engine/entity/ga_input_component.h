#pragma once

/*
** RPI Game Architecture Engine 2017
**
** Author: Matthew Bu
*/

#include "ga_component.h"
#include "math/ga_vec3f.h"

/*
** A component that handles keyboard input and cube
** movement states.
*/
class ga_input_component : public ga_component
{
public:
	bool _move_left;
	bool _move_right;
	bool _move_forward;
	bool _move_backward;

	int _lag_factor;

	ga_vec3f _new_transform;

	ga_input_component(class ga_entity* ent);
	ga_input_component(class ga_entity* ent, bool main);
	void entity_translate(float x, float y, float z);
	virtual void update(struct ga_frame_params* params) override;

	bool has_moved();
private:
	bool _has_moved;
	bool _main;

	bool frame_params_get_input_left(ga_frame_params* params);
	bool frame_params_get_input_right(ga_frame_params* params);
	bool frame_params_get_input_forward(ga_frame_params* params);
	bool frame_params_get_input_backward(ga_frame_params* params);
};
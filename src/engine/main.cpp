/*
** RPI Game Architecture Engine
**
** Portions adapted from:
** Viper Engine - Copyright (C) 2016 Velan Studios - All Rights Reserved
**
** This file is distributed under the MIT License. See LICENSE.txt.
*/

#include "framework/ga_camera.h"
#include "framework/ga_compiler_defines.h"
#include "framework/ga_frame_params.h"
#include "framework/ga_input.h"
#include "framework/ga_sim.h"
#include "framework/ga_output.h"
#include "jobs/ga_job.h"
#include "gui/ga_font.h"

#include "gui/ga_button.h"
#include "gui/ga_checkbox.h"
#include "gui/ga_label.h"

#include "entity/ga_entity.h"
#include "entity/ga_input_component.h"

#include "graphics/ga_cube_component.h"
#include "graphics/ga_program.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#if defined(GA_MINGW)
#include <unistd.h>
#endif

#include "network/ga_network_module.h"
#include "gui/ga_textbox.h"
#include <iostream>
#include <vector>

#define K_PING_SIZE 64

ga_font* g_font = nullptr;
bool create_cube_1 = false;
bool create_cube_2 = false;

bool created_cube_1 = false;
bool created_cube_2 = false;

bool server = true;

static void gui_test(ga_frame_params* params);
static void set_root_path(const char* exepath);
static void show_gui(ga_frame_params* params, ga_network_module* udp_module);

int main(int argc, const char** argv)
{
	set_root_path(argv[0]);

	ga_job::startup(0xffff, 256, 256);

	// Create objects for three phases of the frame: input, sim and output.
	ga_input* input = new ga_input();
	ga_sim* sim = new ga_sim();
	ga_output* output = new ga_output(input->get_window());

	// Create camera.
	ga_camera* camera = new ga_camera({ 0.0f, 7.0f, 20.0f });
	ga_quatf rotation;
	rotation.make_axis_angle(ga_vec3f::y_vector(), ga_degrees_to_radians(180.0f));
	camera->rotate(rotation);
	rotation.make_axis_angle(ga_vec3f::x_vector(), ga_degrees_to_radians(15.0f));
	camera->rotate(rotation);

	// Create cube entities
	ga_entity cube1;
	cube1.translate({ 0.0f, 2.0f, 1.0f });
	ga_input_component *cube_move_1;
	ga_cube_component cube_model_1(&cube1, "data/textures/cube.jpg");

	ga_entity cube2;
	cube2.translate({ 3.0f, 2.0f, 1.0f });
	ga_input_component *cube_move_2;
	ga_cube_component cube_model_2(&cube2, "data/textures/creeper.jpg");

	// Create the default font:
	g_font = new ga_font("VeraMono.ttf", 32.0f, 512, 512);

	// Instantiate network module
	ga_network_module *udp_module = new ga_network_module();

	// Main loop:
	while (true)
	{
		// We pass frame state through the 3 phases using a params object.
		ga_frame_params params;

		// Gather user input and current time.
		if (!input->update(&params))
		{
			break;
		}

		// Update the camera.
		camera->update(&params);

		// Run gameplay.
		sim->update(&params);

		// Perform the late update.
		sim->late_update(&params);

		// Display GUI
		show_gui(&params, udp_module);

		// Draw to screen.
		output->update(&params);

		// Client updates
		if (!server && created_cube_1 && created_cube_2)
		{
			if (cube_move_2->has_moved())
			{
				udp_module->send_transform_msg_to_server(cube_move_2->_new_transform);
			}
			else
			{
				udp_module->send_transform_msg_to_server({ 0, 0, 0 });
			}
		}

		if (create_cube_1)
		{
			if (server)
			{
				cube_move_1 = new ga_input_component(&cube1, true);
				cube_move_2 = new ga_input_component(&cube2, false);
				udp_module->add_input_component_other(cube_move_2);
				udp_module->add_input_component_this(cube_move_1);
			}

			create_cube_1 = false;
			created_cube_1 = true;
			sim->add_entity(&cube1);
		}

		if (create_cube_2)
		{
			if (!server)
			{
				cube_move_1 = new ga_input_component(&cube1, false);
				cube_move_2 = new ga_input_component(&cube2, true);
				udp_module->add_input_component_other(cube_move_1);
				udp_module->add_input_component_this(cube_move_2);
			}

			create_cube_2 = false;
			created_cube_2 = true;
			sim->add_entity(&cube2);
		}
	}

	delete output;
	delete sim;
	delete input;
	delete camera;
	delete udp_module;
	delete cube_move_1;
	delete cube_move_2;

	ga_job::shutdown();

	return 0;
}

static void show_gui(ga_frame_params* params, ga_network_module* udp_module)
{
	static bool start_server = false;
	static bool start_client = false;
	static bool show_server = false;
	static bool show_client = false;
	static bool show_lag_btns = false;

	// Show lag simulation elements
	if (ga_button("Sim Lag", 20.0f, 100.0f, params).get_clicked(params))
	{
		show_lag_btns = !show_lag_btns;
		udp_module->_sim_lag = show_lag_btns;
	}

	if (show_lag_btns)
	{
		if (ga_button("<", 160.0f, 95.0f, params).get_clicked(params))
		{
			udp_module->_min_lag = (udp_module->_min_lag >= 10) ? udp_module->_min_lag - 10 : 0;
		}

		if (ga_button(">", 190.0f, 95.0f, params).get_clicked(params))
		{
			udp_module->_min_lag += 10;
		}
		char lag[K_PING_SIZE];
		int ret = snprintf(lag, sizeof(lag), "K:%d", udp_module->_min_lag);
		lag[ret] = '\0';
		ga_label(lag, 220.0f, 95.0f, params);
	}

	// Show server elements
	if (ga_button("Server", 20.0f, 60.0f, params).get_clicked(params))
	{
		start_server = !start_server;
		show_server = true;
		create_cube_1 = true;
		server = true;
	}

	if (start_server)
	{
		start_server = !start_server;
		udp_module->start_server();
	}

	if (show_server)
	{
		ga_label("SERVER", 500.0f, 60.0f, params);
		char ping[K_PING_SIZE];
		int ret = snprintf(ping, sizeof(ping), "PING %.2f MS", udp_module->get_ping());
		ping[ret] = '\0';
		ga_label(ping, 620.0f, 60.0f, params);
	}

	// Show client elements
	if (ga_button("Client", 140.0f, 60.0f, params).get_clicked(params))
	{
		start_client = !start_client;
		show_client = true;
		create_cube_2 = true;
		server = false;
	}

	if (server && udp_module->_add_player && !created_cube_2)
	{
		create_cube_2 = true;
		udp_module->_add_player = false;
	}

	if (!server && !created_cube_1)
	{
		create_cube_1 = true;
		udp_module->_add_player = false;
	}

	if (start_client)
	{
		start_client = !start_client;
		udp_module->start_client();
	}

	if (show_client)
	{
		ga_label("CLIENT", 500.0f, 60.0f, params);
		char ping[64];
		int n = snprintf(ping, sizeof(ping), "PING %.2f MS", udp_module->get_ping());
		ping[n] = '\0';
		ga_label(ping, 620.0f, 60.0f, params);
	}

	// Show message info
	char data[BUFFER];
	int ret = snprintf(data, sizeof(data), "RECEIVING: %s", udp_module->get_message());
	data[ret] = '\0';
	ga_label(data, 20.0f, 540.0f, params);

	// Show address info
	char addr[BUFFER];
	int ret2 = snprintf(addr, sizeof(addr), "ADDRESS: %s", udp_module->_addr);
	addr[ret2] = '\0';
	ga_label(addr, 20.0f, 580.0f, params);

	// Show port info
	char port[BUFFER];
	int ret3 = snprintf(port, sizeof(port), "PORT: %d", udp_module->_port);
	port[ret3] = '\0';
	ga_label(port, 20.0f, 620.0f, params);
}

char g_root_path[256];
static void set_root_path(const char* exepath)
{
#if defined(GA_MSVC)
	strcpy_s(g_root_path, sizeof(g_root_path), exepath);

	// Strip the executable file name off the end of the path:
	char* slash = strrchr(g_root_path, '\\');
	if (!slash)
	{
		slash = strrchr(g_root_path, '/');
	}
	if (slash)
	{
		slash[1] = '\0';
	}
#elif defined(GA_MINGW)
	char* cwd;
	char buf[PATH_MAX + 1];
	cwd = getcwd(buf, PATH_MAX + 1);
	strcpy_s(g_root_path, sizeof(g_root_path), cwd);

	g_root_path[strlen(cwd)] = '/';
	g_root_path[strlen(cwd) + 1] = '\0';
#endif
}
